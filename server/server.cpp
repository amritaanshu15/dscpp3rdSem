/*
 * server.cpp  -  tiny web server for Rabbit Run
 *
 * NOTE: socket programming is NOT in our syllabus. This file is a cut-down
 * copy of server.cpp from our EcoLife project (same Winsock / POSIX code),
 * only the API part is new. Only GET requests are used to keep it simple.
 *
 *   GET /                         -> web/index.html
 *   GET /css/style.css, /js/...   -> files from web/
 *   GET /api/state                -> game state (JSON)
 *   GET /api/new?name=Ram&animal=R&level=2   -> start new game
 *        custom level adds:  &level=4&fox=1&eagle=0&tiger=1&leopard=0&snake=2&croc=0&deer=2
 *   GET /api/move?dir=U           -> move player (U D L R X)
 *   GET /api/scores               -> top 10 scores
 */
#include "server.h"
#include "../cpp_oop/Utils.h"

#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
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
#ifndef MSG_NOSIGNAL
  #define MSG_NOSIGNAL 0
#endif

using namespace std;

struct Reply {
    int    status;
    string type;
    string body;
};

// ---------------------------------------------------------------
// small text helpers
// ---------------------------------------------------------------
static int hexValue(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

// "Ram%20Kumar" -> "Ram Kumar"
static string urlDecode(string s)
{
    string out;
    for (int i = 0; i < (int)s.size(); i++) {
        if (s[i] == '+') out += ' ';
        else if (s[i] == '%' && i + 2 < (int)s.size() && hexValue(s[i + 1]) >= 0 && hexValue(s[i + 2]) >= 0) {
            out += (char)(hexValue(s[i + 1]) * 16 + hexValue(s[i + 2]));
            i += 2;
        }
        else out += s[i];
    }
    return out;
}

// "a=1&b=two" -> map { a:"1", b:"two" }    (STL map)
static map<string, string> parseQuery(string query)
{
    map<string, string> result;
    stringstream ss(query);
    string pair;
    while (getline(ss, pair, '&')) {
        int eq = (int)pair.find('=');
        if (eq < 0) result[urlDecode(pair)] = "";
        else result[urlDecode(pair.substr(0, eq))] = urlDecode(pair.substr(eq + 1));
    }
    return result;
}

static string mimeType(string path)
{
    if (path.find(".html") != string::npos) return "text/html; charset=utf-8";
    if (path.find(".css")  != string::npos) return "text/css; charset=utf-8";
    if (path.find(".js")   != string::npos) return "text/javascript; charset=utf-8";
    return "text/plain";
}

static Reply makeReply(int status, string type, string body)
{
    Reply r;
    r.status = status;
    r.type = type;
    r.body = body;
    return r;
}

static Reply jsonError(string msg)
{
    return makeReply(400, "application/json", "{\"ok\":false,\"error\":\"" + msg + "\"}");
}

// ---------------------------------------------------------------
// files of the web page
// ---------------------------------------------------------------
static Reply serveFile(string webDir, string path)
{
    if (path == "/") path = "/index.html";
    if (path.find("..") != string::npos)                     // do not leave the web folder
        return makeReply(403, "text/plain", "Forbidden");

    ifstream file((webDir + path).c_str(), ios::in | ios::binary);
    if (!file)
        return makeReply(404, "text/plain", "Not found");

    ostringstream content;
    content << file.rdbuf();
    return makeReply(200, mimeType(path), content.str());
}

// ---------------------------------------------------------------
// the game API
// ---------------------------------------------------------------
static Reply handleApi(World& world, ScoreBoard& board, string path, map<string, string>& q)
{
    try {
        if (path == "/api/state")
            return makeReply(200, "application/json", world.toJson());

        if (path == "/api/new") {
            int level = atoi(q["level"].c_str());
            string animal = q["animal"];
            if (animal.size() != 1) throw GameError("Choose an animal");

            // only used when level == 4 (custom)
            LevelSetup custom;
            custom.fox     = atoi(q["fox"].c_str());
            custom.eagle   = atoi(q["eagle"].c_str());
            custom.tiger   = atoi(q["tiger"].c_str());
            custom.leopard = atoi(q["leopard"].c_str());
            custom.snake   = atoi(q["snake"].c_str());
            custom.croc    = atoi(q["croc"].c_str());
            custom.deer    = atoi(q["deer"].c_str());

            world.newGame(q["name"], animal[0], level, custom);   // may throw GameError
            return makeReply(200, "application/json", world.toJson());
        }

        if (path == "/api/move") {
            string dir = q["dir"];
            if (dir.size() != 1) throw GameError("dir must be one letter");
            world.playerMove(dir[0]);                         // may throw GameError
            board.saveIfGameOver(world);
            return makeReply(200, "application/json", world.toJson());
        }

        if (path == "/api/scores")
            return makeReply(200, "application/json", board.topJson(10));

        return makeReply(404, "application/json", "{\"ok\":false,\"error\":\"Unknown API address\"}");
    }
    catch (GameError& e) {
        return jsonError(e.what());
    }
}

// ---------------------------------------------------------------
// one connection = one request = one reply
// ---------------------------------------------------------------
static void sendReply(sock_t s, Reply r)
{
    const char* text = (r.status == 200) ? "OK" : (r.status == 404 ? "Not Found" : "Bad Request");
    ostringstream head;
    head << "HTTP/1.1 " << r.status << " " << text << "\r\n"
         << "Content-Type: " << r.type << "\r\n"
         << "Content-Length: " << r.body.size() << "\r\n"
         << "Cache-Control: no-store\r\n"
         << "Connection: close\r\n\r\n";
    string all = head.str() + r.body;

    int sent = 0;
    while (sent < (int)all.size()) {
        int n = (int)send(s, all.c_str() + sent, (int)all.size() - sent, MSG_NOSIGNAL);
        if (n <= 0) return;           // browser closed the tab
        sent += n;
    }
}

static void handleConnection(sock_t client, World& world, ScoreBoard& board, string webDir)
{
    // read until end of headers. We only use GET so there is no body.
    string raw;
    char buffer[2048];
    while (raw.find("\r\n\r\n") == string::npos && raw.size() < 16000) {
        int n = (int)recv(client, buffer, sizeof(buffer), 0);
        if (n <= 0) return;
        raw.append(buffer, n);
    }

    // first line:  GET /api/move?dir=U HTTP/1.1
    string method, target;
    stringstream first(raw.substr(0, raw.find("\r\n")));
    first >> method >> target;

    if (method != "GET") {
        sendReply(client, makeReply(400, "text/plain", "Only GET is supported"));
        return;
    }

    string path = target, query = "";
    int qm = (int)target.find('?');
    if (qm >= 0) {
        path = target.substr(0, qm);
        query = target.substr(qm + 1);
    }

    if (path.compare(0, 5, "/api/") == 0) {
        map<string, string> q = parseQuery(query);
        sendReply(client, handleApi(world, board, path, q));
    } else {
        sendReply(client, serveFile(webDir, urlDecode(path)));
    }
}

// ---------------------------------------------------------------
// start the server (this part is the same as in EcoLife)
// ---------------------------------------------------------------
int runServer(World& world, ScoreBoard& board, int port, string webDir)
{
    ifstream check((webDir + "/index.html").c_str());
    if (!check) {
        cout << "Cannot find " << webDir << "/index.html  -> start the program from the project folder\n";
        return 1;
    }

#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) { cout << "Windows sockets could not start\n"; return 1; }
#else
    signal(SIGPIPE, SIG_IGN);
#endif

    sock_t server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == BAD_SOCKET) { cout << "Could not create socket\n"; return 1; }

    int yes = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes));

    sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons((unsigned short)port);
    address.sin_addr.s_addr = inet_addr("127.0.0.1");     // only this computer

    // "::bind" = the socket bind() function. Without "::" the Mac compiler picks
    // std::bind (because of "using namespace std") and gives an error.
    if (::bind(server, (sockaddr*)&address, sizeof(address)) != 0 || listen(server, 8) != 0) {
        cout << "Port " << port << " is busy. Try:  rabbit_run 3001\n";
        CLOSE_SOCKET(server);
        return 1;
    }

    cout << "Rabbit Run is running at  http://localhost:" << port << "   (stop with Ctrl + C)" << endl;

    // browsers sometimes open a connection and send nothing. select() tells us
    // which connections really have data, so one quiet connection can not block the game.
    struct Waiting { sock_t sock; time_t since; };
    vector<Waiting> waiting;

    while (true) {
        fd_set readable;
        FD_ZERO(&readable);
        FD_SET(server, &readable);
        sock_t highest = server;
        for (int i = 0; i < (int)waiting.size(); i++) {
            FD_SET(waiting[i].sock, &readable);
            if (waiting[i].sock > highest) highest = waiting[i].sock;
        }

        timeval wait;
        wait.tv_sec = 1;
        wait.tv_usec = 0;
        if (select((int)highest + 1, &readable, 0, 0, &wait) < 0) continue;

        if (FD_ISSET(server, &readable)) {
            sock_t client = accept(server, 0, 0);
            if (client != BAD_SOCKET) {
                if (waiting.size() >= 40) {
                    CLOSE_SOCKET(waiting[0].sock);
                    waiting.erase(waiting.begin());
                }
                Waiting w;
                w.sock = client;
                w.since = time(0);
                waiting.push_back(w);
            }
        }

        for (int i = (int)waiting.size() - 1; i >= 0; i--) {
            if (FD_ISSET(waiting[i].sock, &readable)) {
                // wait max 5 seconds for the rest of the request
#ifdef _WIN32
                DWORD ms = 5000;
                setsockopt(waiting[i].sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&ms, sizeof(ms));
#else
                timeval tv;
                tv.tv_sec = 5;
                tv.tv_usec = 0;
                setsockopt(waiting[i].sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
#endif
                handleConnection(waiting[i].sock, world, board, webDir);
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
