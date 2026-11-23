/*
 * Event.h  -  nature events (same idea as the events module of EcoLife)
 *
 *                    Event  (abstract)
 *      /        |          |          |         \
 *  Drought  ForestFire  HeavyRain   NewFox   HunterVisit
 *
 * The C heap only stores (day, type number). When the day comes, the World
 * makes the right object with makeEvent(type) and calls apply().
 * apply() is virtual, so the correct child version runs = RUN TIME POLYMORPHISM
 * (late binding through a base class pointer).
 */
#ifndef EVENT_H
#define EVENT_H

#include <string>
using namespace std;

class World;

class Event {
protected:
    string name;
public:
    Event(string name) { this->name = name; }
    virtual ~Event() {}
    string getName() const { return name; }
    virtual string apply(World& world) = 0;     // returns a message for the log
};

class Drought : public Event {
public:
    Drought() : Event("Drought") {}
    string apply(World& world) override;
};

class ForestFire : public Event {
public:
    ForestFire() : Event("Forest Fire") {}
    string apply(World& world) override;
};

class HeavyRain : public Event {
public:
    HeavyRain() : Event("Heavy Rain") {}
    string apply(World& world) override;
};

class NewFox : public Event {
public:
    NewFox() : Event("New Fox") {}
    string apply(World& world) override;
};

class HunterVisit : public Event {
public:
    HunterVisit() : Event("Hunter Visit") {}
    string apply(World& world) override;
};

Event* makeEvent(int type);      // creates the right object (caller must delete it)
string eventName(int type);

#endif
