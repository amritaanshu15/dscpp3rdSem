#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include "module5/EcoLifeEngine.h"
#include "server.h"

using namespace std;

static vector<string> split(const string& line, char sep)
{
    vector<string> parts;
    string current;
    for (size_t i = 0; i < line.size(); i++) {
        if (line[i] == sep) { parts.push_back(current); current.clear(); }
        else current += line[i];
    }
    parts.push_back(current);
    return parts;
}

static int runCommandLine(EcoLifeEngine& engine)
{
    cout << "{\"ok\":true,\"ready\":true}" << endl;

    string line;
    while (getline(cin, line)) {
        if (!line.empty() && line[line.size() - 1] == '\r') line.erase(line.size() - 1);
        if (line.empty()) continue;
        if (line == "quit") break;
        cout << engine.handle(split(line, '|')) << endl;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    EcoLifeEngine engine;

    if (argc >= 2 && string(argv[1]) == "cli") return runCommandLine(engine);

    int port = 3000;
    if (argc >= 2) {
        port = atoi(argv[1]);
        if (port < 1 || port > 65535) {
            cerr << "Usage: ecolife_engine [port] [frontend folder]   or   ecolife_engine cli\n";
            return 1;
        }
    }
    string frontendDir = (argc >= 3) ? argv[2] : "frontend";
    return runServer(engine, port, frontendDir);
}
