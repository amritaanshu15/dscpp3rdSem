#include "server.h"
#include "common/json.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <vector>


#ifdef _WIN32
  #include <winsock2.h>
  typedef SOCKET sock_t;
  #define CLOSE_SOCKET closesocket
  #define BAD_SOCKET   INVALID_SOCKET
#else
  #include <arpa/inet.h>
  #include <netinet/in.h>
  #include <signal.h>
  #include <sys/select.h>
  #include <sys/socket.h>
  #include <unistd.h>

  typedef int sock_t;
  #define CLOSE_SOCKET close
  #define BAD_SOCKET   (-1)
#endif

using namespace std;

#ifndef MSG_NOSIGNAL
  #define MSG_NOSIGNAL 0
#endif

static const char* PASSWORD   = "123@geu";
static const size_t MAX_BODY  = 10000;
static const size_t MAX_HEAD  = 16000;

typedef map<string, string> Params;

struct Reply {
    int         status;
    string type;
    string body;
};

static string trim(const string& s)
{
    size_t a = 0, b = s.size();
    while (a < b && (s[a] == ' ' || s[a] == '\t' || s[a] == '\r' || s[a] == '\n')) a++;
    while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t' || s[b - 1] == '\r' || s[b - 1] == '\n')) b--;
    return s.substr(a, b - a);
}

static string toLower(string s)
{
    for (size_t i = 0; i < s.size(); i++)
        if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a');
    return s;
}

static int hexValue(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static string urlDecode(const string& s)
{
    string out;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '+') out += ' ';
        else if (s[i] == '%' && i + 2 < s.size() && hexValue(s[i + 1]) >= 0 && hexValue(s[i + 2]) >= 0) {
            out += (char)(hexValue(s[i + 1]) * 16 + hexValue(s[i + 2]));
            i += 2;
        }
        else out += s[i];
    }
    return out;
}

static Params parseQuery(const string& q)
{
    Params p;
    size_t start = 0;
    while (start <= q.size()) {
        size_t end = q.find('&', start);
        if (end == string::npos) end = q.size();
        string pair = q.substr(start, end - start);
        if (!pair.empty()) {
            size_t eq = pair.find('=');
            if (eq == string::npos) p[urlDecode(pair)] = "";
            else p[urlDecode(pair.substr(0, eq))] = urlDecode(pair.substr(eq + 1));
        }
        start = end + 1;
    }
    return p;
}

static string bodyValue(const string& body, const string& key)
{
    string pattern = "\"" + key + "\"";
    size_t p = body.find(pattern);
    if (p == string::npos) return "";
    p = body.find(':', p + pattern.size());
    if (p == string::npos) return "";
    p++;
    while (p < body.size() && (body[p] == ' ' || body[p] == '\t' || body[p] == '\n' || body[p] == '\r')) p++;

    string out;
    if (p < body.size() && body[p] == '"') {
        p++;
        while (p < body.size() && body[p] != '"') {
            if (body[p] == '\\' && p + 1 < body.size()) p++;
            out += body[p++];
        }
    } else {
        while (p < body.size() && body[p] != ',' && body[p] != '}') out += body[p++];
        out = trim(out);
    }
    return out;
}

static string clean(string s)
{
    for (size_t i = 0; i < s.size(); i++)
        if (s[i] == '|' || s[i] == '\n' || s[i] == '\r') s[i] = ' ';
    return s;
}

static string valueOf(const Params& q, const string& key)
{
    Params::const_iterator it = q.find(key);
    return it == q.end() ? string() : it->second;
}

static Reply jsonReply(int status, const string& body)
{
    Reply r; r.status = status; r.type = "application/json; charset=utf-8"; r.body = body;
    return r;
}

static Reply errorReply(int status, const string& message)
{
    return jsonReply(status, json::error(message));
}

static const char* statusText(int code)
{
    switch (code) {
        case 200: return "OK";
        case 400: return "Bad Request";
        case 401: return "Unauthorized";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 413: return "Payload Too Large";
        default:  return "Internal Server Error";
    }
}

static string mimeType(const string& path)
{
    size_t dot = path.rfind('.');
    string ext = (dot == string::npos) ? "" : toLower(path.substr(dot));
    if (ext == ".html") return "text/html; charset=utf-8";
    if (ext == ".css")  return "text/css; charset=utf-8";
    if (ext == ".js")   return "text/javascript; charset=utf-8";
    if (ext == ".json") return "application/json";
    if (ext == ".svg")  return "image/svg+xml";
    if (ext == ".png")  return "image/png";
    if (ext == ".ico")  return "image/x-icon";
    return "application/octet-stream";
}

static Reply serveFile(const string& frontendDir, string path)
{
    if (path == "/" || path.empty()) path = "/index.html";

    if (path.find("..") != string::npos || path.find('\\') != string::npos ||
        path.find('\0') != string::npos) {
        Reply r; r.status = 403; r.type = "text/plain"; r.body = "Forbidden";
        return r;
    }
    ifstream file((frontendDir + path).c_str(), ios::in | ios::binary);
    if (!file) {
        Reply r; r.status = 404; r.type = "text/plain"; r.body = "Not found";
        return r;
    }
    ostringstream content;
    content << file.rdbuf();
    Reply r; r.status = 200; r.type = mimeType(path); r.body = content.str();
    return r;
}

static set<string> validTokens;

static Reply sendToEngine(EcoLifeEngine& engine, vector<string> args, bool isLogin = false)
{
    for (size_t i = 0; i < args.size(); i++) args[i] = clean(args[i]);
    string result = engine.handle(args);
    bool failed = result.compare(0, 11, "{\"ok\":false") == 0;
    return jsonReply(failed ? (isLogin ? 401 : 400) : 200, result);
}

static vector<string> cmd(const char* a, const char* b = 0, const char* c = 0)
{
    vector<string> v;
    v.push_back(a);
    if (b) v.push_back(b);
    if (c) v.push_back(c);
    return v;
}

static bool startsWithPath(const string& path, const string& prefix, string& rest)
{
    if (path.size() <= prefix.size() || path.compare(0, prefix.size(), prefix) != 0) return false;
    rest = urlDecode(path.substr(prefix.size()));
    return rest.find('/') == string::npos;
}

static Reply handleApi(EcoLifeEngine& engine, const string& method, const string& path,
                       const Params& q, const string& body, const string& token)
{
    string id;

    if (method == "POST" && path == "/api/login") {
        string user = trim(bodyValue(body, "username"));
        if (user.empty() || user.size() > 40) return errorReply(401, "Please enter a username");
        if (bodyValue(body, "password") != PASSWORD) return errorReply(401, "Wrong password");
        ostringstream t;
        t << hex << (unsigned long)time(0) << rand() << rand();
        validTokens.insert(t.str());
        return jsonReply(200, "{\"ok\":true,\"token\":" + json::str(t.str()) + ",\"username\":" + json::str(user) + "}");
    }

    if (path.compare(0, 5, "/api/") != 0) return errorReply(404, "Unknown API address");
    if (validTokens.count(token) == 0) return errorReply(401, "Please log in first");

    if (method == "POST" && path == "/api/logout") { validTokens.erase(token); return jsonReply(200, "{\"ok\":true}"); }

    if (method == "GET"  && path == "/api/dashboard")           return sendToEngine(engine, cmd("state"));
    if (method == "POST" && path == "/api/simulation/start")    return sendToEngine(engine, cmd("reset"));
    if (method == "POST" && path == "/api/simulation/step")     return sendToEngine(engine, cmd("step", bodyValue(body, "days").c_str()));
    if (method == "POST" && path == "/api/simulation/scenario") return sendToEngine(engine, cmd("scenario", bodyValue(body, "name").c_str()));
    if (method == "POST" && path == "/api/simulation/random-events")
        return sendToEngine(engine, cmd("random_events", bodyValue(body, "enabled") == "true" ? "on" : "off"));
    if (method == "GET"  && path == "/api/simulation/analysis") return sendToEngine(engine, cmd("analysis"));
    if (method == "GET"  && path == "/api/simulation/search")   return sendToEngine(engine, cmd("analysis_search", valueOf(q, "name").c_str()));
    if (method == "GET"  && path == "/api/simulation/history")  return sendToEngine(engine, cmd("history"));
    if (method == "GET"  && path == "/api/simulation/compare") {
        string days = valueOf(q, "days");
        return sendToEngine(engine, cmd("compare", days.empty() ? "30" : days.c_str()));
    }

    if (method == "GET" && path == "/api/population/organisms") return sendToEngine(engine, cmd("organisms"));
    if (method == "GET" && path == "/api/population/stats")     return sendToEngine(engine, cmd("pop_stats"));
    if (method == "GET" && path == "/api/population/search") {
        if (!valueOf(q, "id").empty())   return sendToEngine(engine, cmd("search_id", valueOf(q, "id").c_str()));
        if (!valueOf(q, "name").empty()) return sendToEngine(engine, cmd("search_name", valueOf(q, "name").c_str()));
        return errorReply(400, "Give an id or a name to search for");
    }
    if (method == "GET" && path == "/api/population/list")      return sendToEngine(engine, cmd("list"));
    if (method == "GET" && startsWithPath(path, "/api/population/list/", id))
        return sendToEngine(engine, cmd("list_search", id.c_str()));
    if (method == "POST" && path == "/api/population/organisms") {
        vector<string> v = cmd("add_organism");
        v.push_back(bodyValue(body, "name"));
        v.push_back(bodyValue(body, "type"));
        v.push_back(bodyValue(body, "population"));
        return sendToEngine(engine, v);
    }
    if (method == "PUT" && startsWithPath(path, "/api/population/organisms/", id))
        return sendToEngine(engine, cmd("update_population", id.c_str(), bodyValue(body, "population").c_str()));
    if (method == "DELETE" && startsWithPath(path, "/api/population/organisms/", id))
        return sendToEngine(engine, cmd("delete_organism", id.c_str()));

    if (method == "GET" && path == "/api/resources") return sendToEngine(engine, cmd("resources"));

    if (method == "GET"  && path == "/api/events")             return sendToEngine(engine, cmd("events"));
    if (method == "POST" && path == "/api/events") {
        string queue = bodyValue(body, "queue");
        return sendToEngine(engine, cmd("add_event", bodyValue(body, "type").c_str(), queue.empty() ? "priority" : queue.c_str()));
    }
    if (method == "POST" && path == "/api/events/process") {
        string queue = bodyValue(body, "queue");
        return sendToEngine(engine, cmd("process_event", queue.empty() ? "priority" : queue.c_str()));
    }
    if (method == "POST" && path == "/api/events/process-all") return sendToEngine(engine, cmd("process_all"));

    if (method == "GET"    && path == "/api/foodweb") return sendToEngine(engine, cmd("foodweb"));
    if (method == "POST"   && path == "/api/foodweb/relationships")
        return sendToEngine(engine, cmd("fw_add", bodyValue(body, "food").c_str(), bodyValue(body, "eater").c_str()));
    if (method == "DELETE" && path == "/api/foodweb/relationships")
        return sendToEngine(engine, cmd("fw_remove", valueOf(q, "food").c_str(), valueOf(q, "eater").c_str()));
    if (method == "GET" && startsWithPath(path, "/api/foodweb/dfs/", id)) return sendToEngine(engine, cmd("fw_dfs", id.c_str()));
    if (method == "GET" && startsWithPath(path, "/api/foodweb/bfs/", id)) return sendToEngine(engine, cmd("fw_bfs", id.c_str()));
    if (method == "GET" && path == "/api/foodweb/components") return sendToEngine(engine, cmd("fw_components"));

    return errorReply(404, "Unknown API address");
}

static void sendAll(sock_t s, const string& data)
{
    size_t sent = 0;
    while (sent < data.size()) {
        int n = (int)send(s, data.data() + sent, (int)(data.size() - sent), MSG_NOSIGNAL);
        if (n <= 0) return;
        sent += (size_t)n;
    }
}

static void sendReply(sock_t s, const Reply& r)
{
    ostringstream head;
    head << "HTTP/1.1 " << r.status << " " << statusText(r.status) << "\r\n"
         << "Content-Type: " << r.type << "\r\n"
         << "Content-Length: " << r.body.size() << "\r\n"
         << "Cache-Control: no-store\r\n"
         << "Connection: close\r\n\r\n";
    sendAll(s, head.str() + r.body);
}

struct Request {
    string method;
    string path;
    string query;
    string token;
    string body;
};

static bool readRequest(sock_t client, Request& req, Reply& error)
{
    error.status = 0;
    char buffer[2048];

    string raw;
    size_t headEnd = string::npos;
    while (headEnd == string::npos && raw.size() < MAX_HEAD) {
        int n = (int)recv(client, buffer, sizeof(buffer), 0);
        if (n <= 0) return false;
        raw.append(buffer, (size_t)n);
        headEnd = raw.find("\r\n\r\n");
    }
    if (headEnd == string::npos) { error = errorReply(400, "Request too large"); return false; }

    string head = raw.substr(0, headEnd);
    size_t lineEnd = head.find("\r\n");
    string requestLine = head.substr(0, lineEnd);
    size_t sp1 = requestLine.find(' ');
    size_t sp2 = requestLine.rfind(' ');
    if (sp1 == string::npos || sp2 == sp1) { error = errorReply(400, "Bad request"); return false; }
    req.method = requestLine.substr(0, sp1);
    string target = requestLine.substr(sp1 + 1, sp2 - sp1 - 1);

    size_t contentLength = 0;
    size_t pos = (lineEnd == string::npos) ? head.size() : lineEnd + 2;
    while (pos < head.size()) {
        size_t e = head.find("\r\n", pos);
        if (e == string::npos) e = head.size();
        string line = head.substr(pos, e - pos);
        size_t colon = line.find(':');
        if (colon != string::npos) {
            string name  = toLower(line.substr(0, colon));
            string value = trim(line.substr(colon + 1));
            if (name == "content-length") contentLength = (size_t)atol(value.c_str());
            if (name == "x-auth-token")   req.token = value;
        }
        pos = e + 2;
    }
    if (contentLength > MAX_BODY) { error = errorReply(413, "Request body too large"); return false; }

    req.body = raw.substr(headEnd + 4);
    while (req.body.size() < contentLength) {
        int n = (int)recv(client, buffer, sizeof(buffer), 0);
        if (n <= 0) break;
        req.body.append(buffer, (size_t)n);
    }
    if (req.body.size() > contentLength) req.body.resize(contentLength);

    req.path = target;
    size_t qm = target.find('?');
    if (qm != string::npos) { req.path = target.substr(0, qm); req.query = target.substr(qm + 1); }
    req.path = urlDecode(req.path);
    return true;
}

static void handleConnection(sock_t client, EcoLifeEngine& engine, const string& frontendDir)
{
    Request req;
    Reply error;
    if (!readRequest(client, req, error)) {
        if (error.status != 0) sendReply(client, error);
        return;
    }

    if (req.path.compare(0, 5, "/api/") == 0) {
        sendReply(client, handleApi(engine, req.method, req.path, parseQuery(req.query), req.body, req.token));
    } else if (req.method == "GET") {
        sendReply(client, serveFile(frontendDir, req.path));
    } else {
        Reply r; r.status = 405; r.type = "text/plain"; r.body = "Method not allowed";
        sendReply(client, r);
    }
}

static sock_t openListeningSocket(int port)
{
    sock_t server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == BAD_SOCKET) { cerr << "Could not create a socket.\n"; return BAD_SOCKET; }

    int yes = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes));

    sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons((unsigned short)port);
    address.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (::bind(server, (sockaddr*)&address, sizeof(address)) != 0 || ::listen(server, 8) != 0) {
        cerr << "Could not use port " << port << " (is EcoLife already running?). Try another port:  ecolife_engine 3001\n";
        CLOSE_SOCKET(server);
        return BAD_SOCKET;
    }
    return server;
}

struct Waiting { sock_t sock; time_t since; };

static void setReceiveTimeout(sock_t s)
{
#ifdef _WIN32
    DWORD ms = 5000;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char*)&ms, sizeof(ms));
#else
    timeval tv; tv.tv_sec = 5; tv.tv_usec = 0;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
#endif
}

int runServer(EcoLifeEngine& engine, int port, const string& frontendDir)
{
    string dir = frontendDir;
    while (dir.size() > 1 && (dir[dir.size() - 1] == '/' || dir[dir.size() - 1] == '\\')) dir.erase(dir.size() - 1);

    ifstream check((dir + "/index.html").c_str());
    if (!check) {
        cerr << "Cannot find " << dir << "/index.html\n"
                  << "Start the program from the EcoLife folder, or give the frontend folder:\n"
                  << "    ecolife_engine 3000 path/to/frontend\n";
        return 1;
    }

#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) { cerr << "Windows sockets could not start.\n"; return 1; }
#else
    signal(SIGPIPE, SIG_IGN);
#endif

    sock_t server = openListeningSocket(port);
    if (server == BAD_SOCKET) return 1;

    srand((unsigned)time(0));
    cout << "EcoLife is running:  http://localhost:" << port << "\n"
              << "Login: any username, password " << PASSWORD << "     (stop with Ctrl + C)" << endl;

    vector<Waiting> waiting;

    while (true) {

        fd_set readable;
        FD_ZERO(&readable);
        FD_SET(server, &readable);
        sock_t highest = server;
        for (size_t i = 0; i < waiting.size(); i++) {
            FD_SET(waiting[i].sock, &readable);
            if (waiting[i].sock > highest) highest = waiting[i].sock;
        }

        timeval wait; wait.tv_sec = 1; wait.tv_usec = 0;
        if (select((int)highest + 1, &readable, 0, 0, &wait) < 0) continue;

        if (FD_ISSET(server, &readable)) {
            sock_t client = accept(server, 0, 0);
            if (client != BAD_SOCKET) {
                if (waiting.size() >= 40) {
                    CLOSE_SOCKET(waiting[0].sock);
                    waiting.erase(waiting.begin());
                }
                Waiting w; w.sock = client; w.since = time(0);
                waiting.push_back(w);
            }
        }

        for (size_t i = waiting.size(); i-- > 0; ) {
            if (FD_ISSET(waiting[i].sock, &readable)) {
                setReceiveTimeout(waiting[i].sock);
                handleConnection(waiting[i].sock, engine, dir);
                CLOSE_SOCKET(waiting[i].sock);
                waiting.erase(waiting.begin() + i);
            } else if (time(0) - waiting[i].since > 10) {
                CLOSE_SOCKET(waiting[i].sock);
                waiting.erase(waiting.begin() + i);
            }
        }
    }
    return 0;
}
