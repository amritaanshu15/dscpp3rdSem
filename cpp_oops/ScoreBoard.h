/*
 * ScoreBoard.h  -  leaderboard (high scores)
 *   - scores are kept in memory in a BST (C code, score_bst.c)
 *   - scores are saved in a text file so they are not lost (fstream)
 *
 * CPP topics: file handling (ifstream / ofstream), exception handling
 *             (throw, catch, re-throw), constructor / destructor
 */
#ifndef SCOREBOARD_H
#define SCOREBOARD_H

#include <string>
#include "World.h"
#include "../c_ds/score_bst.h"
using namespace std;

class ScoreBoard {
private:
    ScoreNode* root;
    string     fileName;

    void load();                                     // throws GameError if no file
    void appendToFile(string name, int score, int days);

public:
    ScoreBoard(string fileName);
    ~ScoreBoard();

    void   addScore(string name, int score, int days);
    void   saveIfGameOver(World& world);             // call after every move
    string topJson(int howMany);
    void   printTop(int howMany);                    // for cli mode
};

#endif
