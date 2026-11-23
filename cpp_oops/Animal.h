/*
 * Animal.h  -  all animals of the game
 *
 *                              Animal  (abstract)
 *               /                 |                  \
 *          Player (abstract)    Hunter (abstract)     Deer
 *        /    |     |    \     /  |   |   |   |  \
 *   Rabbit Squirrel Turtle Hedgehog  Fox Eagle Tiger Leopard Snake Crocodile
 *
 *   Player = the animal YOU control (you choose one at the start)
 *   Hunter = predators controlled by the computer
 *   Deer   = harmless animal that eats grass
 *
 * CPP topics: abstract class, pure virtual function, inheritance
 *             (single, hierarchical, MULTILEVEL: Animal -> Player -> Rabbit),
 *             function overriding, virtual destructor, static data member,
 *             protected members, passing parameters to base class constructor
 */
#ifndef ANIMAL_H
#define ANIMAL_H

#include <string>
#include "Pos.h"
#include "../c_ds/common.h"
using namespace std;

class World;      // forward declaration (World.h includes Animal.h)

enum AnimalType { PLAYER = 0, FOX, EAGLE, DEER, TIGER, LEOPARD, SNAKE, CROCODILE };

// =====================================================================
// Animal : base class of everything that moves
// =====================================================================
class Animal {
protected:
    int    id;
    string name;
    Pos    pos;

    static int totalAnimals;              // shared by all objects

    bool tryStep(World& world, Pos dir);  // move one step if the tile is allowed
    void wander(World& world);            // random walk (up/down/left/right)

public:
    Animal(int id, string name, Pos start);
    virtual ~Animal();

    virtual void   takeTurn(World& world) = 0;   // PURE virtual -> abstract class
    virtual int    type() const = 0;
    virtual char   letter() const = 0;           // one letter for the map / JSON
    virtual string describe() const = 0;         // ability text shown on the web page
    virtual bool   isPredator() const;
    virtual bool   canWalkOn(char tile) const;
    virtual bool   flies() const;

    int    getId() const   { return id; }
    string getName() const { return name; }
    Pos    getPos() const  { return pos; }
    void   setPos(Pos p)   { pos = p; }

    static int getTotalAnimals() { return totalAnimals; }
};

// =====================================================================
// Player : the animal you control. Has food, water and lives.
// Every child only changes a few numbers / rules (its ability).
// =====================================================================
class Player : public Animal {
protected:
    int food;          // 0..100
    int water;         // 0..100
    int lives;         // 1 for most, 2 for hedgehog
    int grassEaten;
    int drinks;
public:
    Player(string name, Pos start, int lives);

    void takeTurn(World& world) override;          // hungry + thirsty every day
    int  type() const override { return PLAYER; }

    // the "ability" numbers. Children override the ones they want to change.
    virtual int  foodPerDay() const  { return 3; }
    virtual int  waterPerDay() const { return 3; }
    virtual int  grassFood() const   { return 15; }
    virtual bool hiddenOn(char tile) const { (void)tile; return false; }  // safe spot?

    void eat();
    void drink(int amount);
    void loseWater(int amount);
    bool loseLife();                // returns true if still alive after losing one

    int getFood() const       { return food; }
    int getWater() const      { return water; }
    int getLives() const      { return lives; }
    int getGrassEaten() const { return grassEaten; }
    int getDrinks() const     { return drinks; }
};

class Rabbit : public Player {
public:
    Rabbit(Pos start) : Player("Rabbit", start, 1) {}
    char   letter() const override { return 'R'; }
    string describe() const override;
    bool   canWalkOn(char tile) const override;       // + bushes
    bool   hiddenOn(char tile) const override;        // hidden in bushes
};

class Squirrel : public Player {
public:
    Squirrel(Pos start) : Player("Squirrel", start, 1) {}
    char   letter() const override { return 'Q'; }
    string describe() const override;
    bool   canWalkOn(char tile) const override;       // + bushes + rocks (climbs)
    bool   hiddenOn(char tile) const override;        // hidden in bushes
    int    foodPerDay() const override { return 2; }  // small body, eats less
    int    grassFood() const override  { return 10; } // ... but gets less from grass
};

class Turtle : public Player {
public:
    Turtle(Pos start) : Player("Turtle", start, 1) {}
    char   letter() const override { return 'U'; }
    string describe() const override;
    bool   canWalkOn(char tile) const override;       // + water (swims)
    bool   hiddenOn(char tile) const override;        // hidden under water
    int    waterPerDay() const override { return 4; } // dries out on land
};

class Hedgehog : public Player {
public:
    Hedgehog(Pos start) : Player("Hedgehog", start, 2) {}   // 2 lives (spikes)
    char   letter() const override { return 'H'; }
    string describe() const override;
};

// creates the player the user chose: 'R' 'Q' 'U' 'H'   (throws GameError if wrong)
Player* makePlayer(char kind, Pos start);

// =====================================================================
// Hunter : base class of all predators.
// chase() = move one step on the shortest path (BFS) to the player.
// =====================================================================
class Hunter : public Animal {
protected:
    int  sight;        // how far it can see / smell the player
    bool chasing;
    bool chase(World& world);        // true if it moved towards the player
public:
    Hunter(int id, string name, Pos start, int sight);
    void takeTurn(World& world) override;          // chase, else wander
    bool isPredator() const override { return true; }
    virtual bool seesHidden() const { return false; }   // snake overrides this
};

class Fox : public Hunter {
public:
    Fox(int id, string name, Pos start) : Hunter(id, name, start, 6) {}
    int    type() const override   { return FOX; }
    char   letter() const override { return 'F'; }
    string describe() const override;
};

class Eagle : public Hunter {
    int turnCounter;
public:
    Eagle(int id, string name, Pos start) : Hunter(id, name, start, 4) { turnCounter = 0; }
    void   takeTurn(World& world) override;        // own movement (diagonal flying)
    int    type() const override   { return EAGLE; }
    char   letter() const override { return 'E'; }
    string describe() const override;
    bool   canWalkOn(char tile) const override { (void)tile; return true; }
    bool   flies() const override { return true; }
};

class Tiger : public Hunter {
    int runDays;       // how many days it has been running
    int restDays;      // days left to rest
public:
    Tiger(int id, string name, Pos start) : Hunter(id, name, start, 5) { runDays = 0; restDays = 0; }
    void   takeTurn(World& world) override;        // 2 steps per day, then gets tired
    int    type() const override   { return TIGER; }
    char   letter() const override { return 'T'; }
    string describe() const override;
};

class Leopard : public Hunter {
public:
    Leopard(int id, string name, Pos start) : Hunter(id, name, start, 6) {}
    int    type() const override   { return LEOPARD; }
    char   letter() const override { return 'L'; }
    string describe() const override;
    bool   canWalkOn(char tile) const override;    // + rocks (climbs)
};

class Snake : public Hunter {
    int turnCounter;
public:
    Snake(int id, string name, Pos start) : Hunter(id, name, start, 5) { turnCounter = 0; }
    void   takeTurn(World& world) override;        // slow: moves every 2nd day
    int    type() const override   { return SNAKE; }
    char   letter() const override { return 'S'; }
    string describe() const override;
    bool   canWalkOn(char tile) const override;    // + bushes + water
    bool   seesHidden() const override { return true; }   // smells you anywhere
};

class Crocodile : public Hunter {
public:
    Crocodile(int id, string name, Pos start) : Hunter(id, name, start, 4) {}
    void   takeTurn(World& world) override;        // stays in water, bites next to it
    int    type() const override   { return CROCODILE; }
    char   letter() const override { return 'C'; }
    string describe() const override;
    bool   canWalkOn(char tile) const override { return tile == T_WATER; }
};

// =====================================================================
// Deer : harmless, eats grass
// =====================================================================
class Deer : public Animal {
public:
    Deer(int id, string name, Pos start) : Animal(id, name, start) {}
    void   takeTurn(World& world) override;
    int    type() const override   { return DEER; }
    char   letter() const override { return 'D'; }
    string describe() const override;
};

#endif
