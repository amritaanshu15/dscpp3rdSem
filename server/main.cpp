/*
 * main.cpp  -  program starts here
 *
 *   rabbit_run            -> web game on http://localhost:3000
 *   rabbit_run 8080       -> web game on port 8080
 *   rabbit_run cli        -> play inside the terminal (no browser), good for testing
 */
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include "../cpp_oop/World.h"
#include "../cpp_oop/ScoreBoard.h"
#include "../cpp_oop/Utils.h"
#include "server.h"
using namespace std;

static int askNumber(string question)
{
    int x = 0;
    cout << question;
    cin >> x;
    return x;
}

static int playInTerminal(World& world, ScoreBoard& board)
{
    string name;
    char animal;
    int level;
    LevelSetup custom = {0, 0, 0, 0, 0, 0, 0};

    cout << "===== RABBIT RUN (terminal version) =====\n";
    cout << "Your name: ";
    cin >> name;
    cout << "Animal (R rabbit, Q squirrel, U turtle, H hedgehog): ";
    cin >> animal;
    level = askNumber("Level (1 easy, 2 normal, 3 hard, 4 custom): ");

    if (level == CUSTOM_LEVEL) {
        cout << "How many of each animal (0 to 3)?\n";
        custom.fox     = askNumber("  Fox: ");
        custom.eagle   = askNumber("  Eagle: ");
        custom.tiger   = askNumber("  Tiger: ");
        custom.leopard = askNumber("  Leopard: ");
        custom.snake   = askNumber("  Snake: ");
        custom.croc    = askNumber("  Crocodile: ");
        custom.deer    = askNumber("  Deer: ");
    }

    try {
        world.newGame(name, animal, level, custom);
    }
    catch (GameError& e) {
        cout << e.what() << " -> starting rabbit, level 1\n";
        world.newGame(name, 'R', 1, custom);
    }

    cout << "Map: player R/Q/U/H  F fox  E eagle  T tiger  L leopard  S snake  C croc  D deer\n";
    cout << "     g grass  w water  b bush  r rock  x burnt\n";
    cout << "Keys: w a s d = move, x = stay, q = quit (then press Enter)\n";

    char key;
    while (true) {
        world.printText();
        if (world.isOver()) break;

        cout << "> ";
        if (!(cin >> key) || key == 'q') break;

        char dir = ' ';
        if (key == 'w') dir = 'U';
        else if (key == 's') dir = 'D';
        else if (key == 'a') dir = 'L';
        else if (key == 'd') dir = 'R';
        else if (key == 'x') dir = 'X';

        try {
            world.playerMove(dir);          // wrong key -> GameError
        }
        catch (GameError& e) {
            cout << "!! " << e.what() << "\n";
        }
        board.saveIfGameOver(world);
    }
    board.printTop(5);
    return 0;
}

int main(int argc, char* argv[])
{
    srand((unsigned)time(0));       // new random map every game

    World world;
    ScoreBoard board("data/scores.txt");

    if (argc >= 2 && string(argv[1]) == "cli")
        return playInTerminal(world, board);

    int port = 3000;
    if (argc >= 2) {
        port = atoi(argv[1]);
        if (port < 1 || port > 65535) {
            cout << "Usage: rabbit_run [port]   or   rabbit_run cli\n";
            return 1;
        }
    }
    return runServer(world, board, port, "web");
}
