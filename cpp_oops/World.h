/*
 * World.h  -  the whole game is inside this one class
 *
 *   World HAS-A Grid (C 2D array)            -> the map
 *         HAS-A Player*                      -> the animal you chose
 *         HAS-A vector<Animal*>              -> hunters and deer (STL vector)
 *         HAS-A Queue  (C circular queue)    -> grass that will grow back
 *         HAS-A EventHeap (C priority queue) -> planned events
 *         HAS-A LogList (C linked list)      -> last messages
 *
 * CPP topics: class, data hiding (private), constructor / destructor,
 *             dynamic objects (new / delete), pointers to objects, STL vector,
 *             structure + array of structures (LevelSetup), exceptions
 */
#ifndef WORLD_H
#define WORLD_H

#include <string>
#include <vector>
#include "Animal.h"
#include "../c_ds/grid.h"
#include "../c_ds/queue.h"
#include "../c_ds/event_heap.h"
#include "../c_ds/log_list.h"
using namespace std;

// ---- game rules (change these to make the game easier / harder) ----
#define GOAL_DAYS      50   // survive this many days to win
#define DRINK_WATER    30   // one drink gives this much water
#define DROUGHT_DRINK  10   // one drink during drought
#define REGROW_DAYS    15   // eaten / burnt plants grow back after this many days
#define CUSTOM_LEVEL    4   // level number of the custom level
#define MAX_EACH        3   // custom level: max 3 of each animal

// how many of each animal are in a level
struct LevelSetup {
    int fox;
    int eagle;
    int tiger;
    int leopard;
    int snake;
    int croc;
    int deer;
};

class World {
private:
    Grid            grid;
    Player*         player;
    vector<Animal*> animals;        // every animal except the player
    Queue           regrowQueue;
    EventHeap       events;
    LogList         log;

    string playerName;
    int    level;                   // 1 easy, 2 normal, 3 hard, 4 custom
    int    hunterCount;             // number of hunters at the start (custom score)
    int    day;
    int    nextId;
    int    nameCount[8];            // for names "Fox 1", "Fox 2" ... (index = AnimalType)
    int    droughtLeft;
    bool   started;
    bool   over;
    bool   won;
    bool   saved;
    string endReason;

    void   clearGame();
    void   endTurn();
    void   processRegrow();
    void   processEvents();
    void   finishGame(bool win, string reason);
    void   spawn(Animal* a, int minDist);              // put animal on a random good cell
    Pos    randomCellFor(Animal* a, int minDist);
    string nextName(string base, int type);

public:
    World();
    ~World();

    // ---- called by the server ----
    void   newGame(string name, char animal, int level, LevelSetup custom);   // throws GameError
    void   playerMove(char dir);                                           // U D L R X, throws GameError
    string toJson();
    void   printText();

    // ---- used by animals and events ----
    bool    inside(Pos p) const;
    char    tileAt(Pos p) const;
    void    setTile(Pos p, char t);
    Player* getPlayer() { return player; }
    bool    playerHidden() const;
    bool    groundAnimalAt(Pos p, Animal* except) const;
    Animal* animalAt(Pos p) const;
    void    makeWalkMap(int map[ROWS][COLS], Animal* a) const;
    void    addMessage(string msg);
    void    scheduleRegrow(Pos p, char tile);
    void    playerCaught(Animal* hunter);
    void    addFox();
    bool    removeOneHunter(string& whoRemoved);
    void    startDrought(int days);
    void    endDrought();
    bool    isDrought() const { return droughtLeft > 0; }

    // ---- simple getters ----
    int    getDay() const        { return day; }
    bool   isStarted() const     { return started; }
    bool   isOver() const        { return over; }
    bool   isSaved() const       { return saved; }
    void   markSaved()           { saved = true; }
    string getPlayerName() const { return playerName; }
    int    getScore() const;
};

#endif
