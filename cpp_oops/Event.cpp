/*
 * Event.cpp  -  what every event does to the forest
 */
#include <cstdlib>
#include "Event.h"
#include "World.h"
#include "../c_ds/event_heap.h"

Event* makeEvent(int type)
{
    switch (type) {
        case EV_DROUGHT:     return new Drought();
        case EV_FOREST_FIRE: return new ForestFire();
        case EV_HEAVY_RAIN:  return new HeavyRain();
        case EV_NEW_FOX:     return new NewFox();
        case EV_HUNTER:      return new HunterVisit();
    }
    return NULL;
}

string eventName(int type)
{
    Event* e = makeEvent(type);
    if (e == NULL) return "Unknown";
    string n = e->getName();
    delete e;
    return n;
}

// ---------------------------------------------------------------------
// Drought: ponds give less water and rabbit gets thirsty faster for 8 days
// ---------------------------------------------------------------------
string Drought::apply(World& world)
{
    world.startDrought(8);
    return "Ponds are drying up! For 8 days drinking gives less water.";
}

// ---------------------------------------------------------------------
// Forest fire: burns grass and bushes in a 5x5 square
// ---------------------------------------------------------------------
string ForestFire::apply(World& world)
{
    int cr = rand() % ROWS;
    int cc = rand() % COLS;
    int burnt = 0;

    for (int r = cr - 2; r <= cr + 2; r++) {
        for (int c = cc - 2; c <= cc + 2; c++) {
            Pos p(r, c);
            if (!world.inside(p)) continue;
            char t = world.tileAt(p);
            if (t == T_GRASS || t == T_BUSH) {
                world.setTile(p, T_BURNT);
                world.scheduleRegrow(p, t);       // it will come back later
                burnt++;
            }
        }
    }

    Player* pl = world.getPlayer();
    if (chebyshev(pl->getPos(), Pos(cr, cc)) <= 2) {
        pl->loseWater(10);
        return "Fire near you! " + to_string(burnt) + " plants burnt and the smoke made you thirsty (-10 water).";
    }
    return "Fire in the forest! " + to_string(burnt) + " plants burnt.";
}

// ---------------------------------------------------------------------
// Heavy rain: ends drought, new grass grows, rabbit drinks rain
// ---------------------------------------------------------------------
string HeavyRain::apply(World& world)
{
    world.endDrought();
    int grown = 0;
    for (int tries = 0; tries < 200 && grown < 8; tries++) {
        Pos p(rand() % ROWS, rand() % COLS);
        char t = world.tileAt(p);
        if (t == T_GROUND || t == T_BURNT) {
            world.setTile(p, T_GRASS);
            grown++;
        }
    }
    world.getPlayer()->drink(10);         // rain = free drink for every animal
    return "Heavy rain! " + to_string(grown) + " new grass patches and you drank some rain (+10 water).";
}

// ---------------------------------------------------------------------
// A new fox comes from outside
// ---------------------------------------------------------------------
string NewFox::apply(World& world)
{
    world.addFox();
    return "A new fox has come into the forest. Be careful!";
}

// ---------------------------------------------------------------------
// Hunter scares one predator away (same as "Predator Removal" in EcoLife)
// ---------------------------------------------------------------------
string HunterVisit::apply(World& world)
{
    string who;
    if (world.removeOneHunter(who))
        return "A hunter scared away " + who + ".";
    return "A hunter came but found no predator.";
}
