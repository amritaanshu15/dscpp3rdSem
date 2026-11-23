/*
 * ScoreBoard.cpp
 * File format (data/scores.txt), one game per line:
 *     name score days
 *     Unnati 1340 50
 */
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include "ScoreBoard.h"
#include "Utils.h"

ScoreBoard::ScoreBoard(string fileName)
{
    root = NULL;
    this->fileName = fileName;

    try {
        load();
    }
    catch (GameError& e) {
        // first time the game runs there is no file yet. That is fine.
        cout << e.what() << " - starting with an empty leaderboard." << endl;
    }
}

ScoreBoard::~ScoreBoard()
{
    bst_free(root);
}

void ScoreBoard::load()
{
    ifstream fin(fileName.c_str());
    if (!fin)
        throw GameError("Could not open " + fileName);

    string name;
    int score, days;
    int count = 0;
    while (fin >> name >> score >> days) {
        root = bst_insert(root, name.c_str(), score, days);
        count++;
    }
    fin.close();
    cout << "Loaded " << count << " old scores from " << fileName << endl;
}

void ScoreBoard::appendToFile(string name, int score, int days)
{
    ofstream fout(fileName.c_str(), ios::app);      // ios::app = add at the end
    if (!fout)
        throw GameError("Could not write to " + fileName);
    fout << name << " " << score << " " << days << "\n";
    fout.close();
}

void ScoreBoard::addScore(string name, int score, int days)
{
    root = bst_insert(root, name.c_str(), score, days);
    try {
        appendToFile(name, score, days);
    }
    catch (GameError& e) {
        cout << "Warning: " << e.what() << endl;
        throw;                                       // re-throw to the caller
    }
}

void ScoreBoard::saveIfGameOver(World& world)
{
    if (!world.isOver() || world.isSaved())
        return;
    world.markSaved();                               // never save the same game twice
    try {
        addScore(world.getPlayerName(), world.getScore(), world.getDay());
    }
    catch (GameError& e) {
        // score is still in memory (BST), only the file failed. Game goes on.
        cout << "Score kept only in memory: " << e.what() << endl;
    }
}

string ScoreBoard::topJson(int howMany)
{
    ScoreRec top[20];
    int count = 0;
    if (howMany > 20) howMany = 20;
    bst_top(root, top, howMany, &count);             // reverse inorder traversal

    ostringstream js;
    js << "{\"ok\":true,\"total\":" << bst_count(root)
       << ",\"treeHeight\":" << bst_height(root) << ",\"scores\":[";
    for (int i = 0; i < count; i++) {
        if (i > 0) js << ",";
        js << "{\"rank\":" << (i + 1) << ",\"name\":\"" << top[i].name << "\""
           << ",\"score\":" << top[i].score << ",\"days\":" << top[i].days << "}";
    }
    js << "]}";
    return js.str();
}

void ScoreBoard::printTop(int howMany)
{
    ScoreRec top[20];
    int count = 0;
    if (howMany > 20) howMany = 20;
    bst_top(root, top, howMany, &count);

    // setw / left are I/O manipulators: they make a neat table
    cout << "\n---- TOP SCORES ----\n";
    cout << left << setw(5) << "#" << setw(16) << "Name" << setw(8) << "Score" << "Days\n";
    for (int i = 0; i < count; i++)
        cout << left << setw(5) << (i + 1) << setw(16) << top[i].name
             << setw(8) << top[i].score << top[i].days << "\n";
    if (count == 0) cout << "(no scores yet)\n";
}
