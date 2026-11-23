/*
 * Animal.cpp  -  what every animal does in one turn
 */
#include <cstdlib>
#include "Animal.h"
#include "World.h"
#include "Utils.h"
#include "../c_ds/pathfind.h"

// static data member must be defined once outside the class
int Animal::totalAnimals = 0;

// =====================================================================
// Animal (base class)
// =====================================================================
Animal::Animal(int id, string name, Pos start)
{
    this->id = id;
    this->name = name;
    pos = start;
    totalAnimals++;
}

Animal::~Animal()
{
    // nothing to free, but it is virtual so child destructors also run
}

bool Animal::isPredator() const
{
    return false;
}

bool Animal::flies() const
{
    return false;
}

// default for walking animals: mud, grass and burnt ground
bool Animal::canWalkOn(char tile) const
{
    if (tile == T_GROUND || tile == T_GRASS || tile == T_BURNT)
        return true;
    return false;
}

bool Animal::tryStep(World& world, Pos dir)
{
    Pos target = pos + dir;                  // operator+ of Pos

    if (!world.inside(target)) return false;
    if (!canWalkOn(world.tileAt(target))) return false;
    if (target == world.getPlayer()->getPos()) return false;   // wandering never steps on the player
    if (!flies() && world.groundAnimalAt(target, this)) return false;

    pos = target;
    return true;
}

void Animal::wander(World& world)
{
    Pos dirs[4] = { Pos(-1, 0), Pos(1, 0), Pos(0, -1), Pos(0, 1) };
    int start = rand() % 4;

    if (rand() % 4 == 0) return;             // 1 in 4 chance to stand still

    for (int i = 0; i < 4; i++) {
        if (tryStep(world, dirs[(start + i) % 4]))
            return;
    }
}

// =====================================================================
// Player (base of Rabbit, Squirrel, Turtle, Hedgehog)
// =====================================================================
Player::Player(string name, Pos start, int lives) : Animal(0, name, start)
{
    food = 100;
    water = 100;
    this->lives = lives;
    grassEaten = 0;
    drinks = 0;
}

void Player::takeTurn(World& world)
{
    // foodPerDay() and waterPerDay() are virtual, so a Squirrel uses its own numbers
    food = food - foodPerDay();
    if (world.isDrought())
        water = water - (waterPerDay() + 2);     // hot days -> thirsty faster
    else
        water = water - waterPerDay();

    food = clampValue(food, 0, 100);             // template function
    water = clampValue(water, 0, 100);
}

void Player::eat()
{
    food = clampValue(food + grassFood(), 0, 100);
    grassEaten++;
}

void Player::drink(int amount)
{
    water = clampValue(water + amount, 0, 100);
    drinks++;
}

void Player::loseWater(int amount)
{
    water = clampValue(water - amount, 0, 100);
}

bool Player::loseLife()
{
    lives--;
    return lives > 0;
}

Player* makePlayer(char kind, Pos start)
{
    switch (kind) {
        case 'R': return new Rabbit(start);
        case 'Q': return new Squirrel(start);
        case 'U': return new Turtle(start);
        case 'H': return new Hedgehog(start);
    }
    throw GameError("Unknown animal. Choose R (rabbit), Q (squirrel), U (turtle) or H (hedgehog)");
}

// ---------------- Rabbit ----------------
string Rabbit::describe() const
{
    return "Balanced animal. Hides in bushes (most hunters can not see you there).";
}

bool Rabbit::canWalkOn(char tile) const
{
    if (tile == T_BUSH) return true;
    return Animal::canWalkOn(tile);              // call the base class version
}

bool Rabbit::hiddenOn(char tile) const
{
    return tile == T_BUSH;
}

// ---------------- Squirrel ----------------
string Squirrel::describe() const
{
    return "Climbs on rocks (fox, tiger and snake can not follow) and hides in bushes. Eats less but grass gives only +10.";
}

bool Squirrel::canWalkOn(char tile) const
{
    if (tile == T_BUSH || tile == T_ROCK) return true;
    return Animal::canWalkOn(tile);
}

bool Squirrel::hiddenOn(char tile) const
{
    return tile == T_BUSH;
}

// ---------------- Turtle ----------------
string Turtle::describe() const
{
    return "Swims in ponds and drinks while swimming. Hidden under water, but can not enter bushes. Gets thirsty faster on land.";
}

bool Turtle::canWalkOn(char tile) const
{
    if (tile == T_WATER) return true;
    return Animal::canWalkOn(tile);
}

bool Turtle::hiddenOn(char tile) const
{
    return tile == T_WATER;
}

// ---------------- Hedgehog ----------------
string Hedgehog::describe() const
{
    return "Can not hide anywhere, but has spikes: survives the first attack (2 lives) and the hunter runs away.";
}

// =====================================================================
// Hunter (base of all predators)
// =====================================================================
Hunter::Hunter(int id, string name, Pos start, int sight) : Animal(id, name, start)
{
    this->sight = sight;
    chasing = false;
}

// one step on the shortest path to the player. Returns false if it can not chase.
bool Hunter::chase(World& world)
{
    Player* pl = world.getPlayer();
    Pos ppos = pl->getPos();

    if (!seesHidden() && world.playerHidden()) return false;     // can not see the player
    if (manhattan(pos, ppos) > sight) return false;               // too far
    if (!canWalkOn(world.tileAt(ppos))) return false;             // player is on a tile I can not go to

    int map[ROWS][COLS];
    world.makeWalkMap(map, this);

    Cell start  = { pos.r, pos.c };
    Cell target = { ppos.r, ppos.c };
    Cell next;
    int steps = bfs_next_step(map, start, target, &next);       // C function (BFS)
    if (steps == -1) return false;                                // no path

    if (!chasing) {
        world.addMessage(name + " found you and is chasing you!");
        chasing = true;
    }
    pos = Pos(next.r, next.c);
    if (pos == ppos)
        world.playerCaught(this);
    return true;
}

// default turn for a hunter (Fox and Leopard use this one as it is)
void Hunter::takeTurn(World& world)
{
    if (chase(world))
        return;

    if (chasing) {
        world.addMessage(name + " lost you.");
        chasing = false;
    }
    wander(world);
}

// ---------------- Fox ----------------
string Fox::describe() const
{
    return "Smells you from 6 steps and runs the shortest path (BFS). Can not enter bushes, water or rocks.";
}

// ---------------- Eagle ----------------
static int sign(int x)
{
    if (x > 0) return 1;
    if (x < 0) return -1;
    return 0;
}

void Eagle::takeTurn(World& world)
{
    turnCounter++;
    if (turnCounter % 3 == 0)
        return;                       // every 3rd day the eagle rests on a tree

    Pos ppos = world.getPlayer()->getPos();

    if (!world.playerHidden() && chebyshev(pos, ppos) <= sight) {
        if (!chasing) {
            world.addMessage(name + " spotted you from the sky!");
            chasing = true;
        }
        // fly one step towards the player (diagonal allowed)
        pos = pos + Pos(sign(ppos.r - pos.r), sign(ppos.c - pos.c));
        if (pos == ppos)
            world.playerCaught(this);
        return;
    }

    chasing = false;
    int dr[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
    int dc[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
    int k = rand() % 8;
    tryStep(world, Pos(dr[k], dc[k]));
}

string Eagle::describe() const
{
    return "Flies over everything and moves diagonally. Sees you from 4 cells. Rests every 3rd day. Can not see hidden animals.";
}

// ---------------- Tiger ----------------
void Tiger::takeTurn(World& world)
{
    if (restDays > 0) {
        restDays--;                   // too tired to move
        return;
    }

    if (chase(world)) {
        if (!world.isOver())
            chase(world);             // tiger is fast: second step in the same day
        runDays++;
        if (runDays >= 3) {
            world.addMessage(name + " is tired and rests for 2 days.");
            restDays = 2;
            runDays = 0;
        }
        return;
    }

    runDays = 0;
    if (chasing) {
        world.addMessage(name + " lost you.");
        chasing = false;
    }
    wander(world);
}

string Tiger::describe() const
{
    return "Very fast: runs 2 steps a day when chasing, but after 3 days of running it rests for 2 days. Sees 5 steps.";
}

// ---------------- Leopard ----------------
bool Leopard::canWalkOn(char tile) const
{
    if (tile == T_ROCK) return true;
    return Animal::canWalkOn(tile);
}

string Leopard::describe() const
{
    return "Climbs rocks (a squirrel on a rock is NOT safe from it). Sees 6 steps. Can not enter bushes or water.";
}

// ---------------- Snake ----------------
bool Snake::canWalkOn(char tile) const
{
    if (tile == T_BUSH || tile == T_WATER) return true;
    return Animal::canWalkOn(tile);
}

void Snake::takeTurn(World& world)
{
    turnCounter++;
    if (turnCounter % 2 == 0)
        return;                       // slow: moves only every 2nd day
    Hunter::takeTurn(world);          // then the normal hunter turn (base class function)
}

string Snake::describe() const
{
    return "Slow (moves every 2nd day) but smells you even when hidden, and can go into bushes and water. Can not climb rocks.";
}

// ---------------- Crocodile ----------------
void Crocodile::takeTurn(World& world)
{
    Pos ppos = world.getPlayer()->getPos();

    // bite: player right next to it (or on it), unless the player is in a bush
    if (manhattan(pos, ppos) <= 1 && world.tileAt(ppos) != T_BUSH) {
        world.addMessage(name + " snapped from the water!");
        world.playerCaught(this);
        return;
    }

    // swim to the water cell that is closest to the player
    if (manhattan(pos, ppos) <= sight) {
        Pos dirs[4] = { Pos(-1, 0), Pos(1, 0), Pos(0, -1), Pos(0, 1) };
        Pos best = pos;
        for (int i = 0; i < 4; i++) {
            Pos p = pos + dirs[i];
            if (world.inside(p) && canWalkOn(world.tileAt(p)) && !world.groundAnimalAt(p, this)
                && manhattan(p, ppos) < manhattan(best, ppos))
                best = p;
        }
        pos = best;
        return;
    }
    wander(world);                    // wander() only uses water tiles (canWalkOn)
}

string Crocodile::describe() const
{
    return "Lives only in ponds. Bites you if you stand next to it (careful when drinking!). Bushes are safe.";
}

// =====================================================================
// Deer
// =====================================================================
void Deer::takeTurn(World& world)
{
    wander(world);

    // deer eats the grass it is standing on (half of the time)
    if (world.tileAt(pos) == T_GRASS && rand() % 2 == 0) {
        world.setTile(pos, T_GROUND);
        world.scheduleRegrow(pos, T_GRASS);
    }
}

string Deer::describe() const
{
    return "Harmless, but eats grass (less food for you) and blocks your way.";
}
