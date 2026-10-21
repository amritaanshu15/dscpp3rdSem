#ifndef EVENT_H
#define EVENT_H

#include <string>
#include "../module1_organisms/Organism.h"
#include "../module2_resources/Resource.h"

enum EventType { DROUGHT = 1, FOREST_FIRE = 2, PREDATOR_REMOVAL = 3, HEAVY_RAIN = 4, SEASONAL_CHANGE = 5 };

class Season {
    int index;
    int daysInSeason;
public:
    static const int LENGTH = 30;
    Season();
    void        tickDay();
    void        forceNext();
    void        reset();
    std::string name() const;
    double      growthFactor() const;
    double      waterRegen() const;
};

class Weather {
public:
    enum Condition { CLEAR = 0, RAINY = 1, DRY = 2 };
private:
    int condition;
    int daysLeft;
public:
    Weather();
    void        set(int condition, int days);
    void        tickDay();
    void        reset();
    std::string name() const;
    double      waterChange() const;
};

struct EventEffect {
    double      popMultiplier[4];
    int         spreadDays;
    std::string summary;
    EventEffect() : spreadDays(1) {
        for (int i = 0; i < 4; i++) popMultiplier[i] = 1.0;
    }
};

class Event {
protected:
    std::string name;
    int         priority;
public:
    Event(const std::string& name, int priority) : name(name), priority(priority) {}
    virtual ~Event() {}

    std::string getName() const    { return name; }
    int         getPriority() const { return priority; }

    virtual EventEffect apply(Environment& env, Season& season, Weather& weather) = 0;
    virtual std::string description() const = 0;
};

class Drought : public Event {
public:
    Drought() : Event("Drought", 4) {}
    EventEffect apply(Environment& env, Season& season, Weather& weather) override;
    std::string description() const override;
};

class ForestFire : public Event {
public:
    ForestFire() : Event("Forest Fire", 5) {}
    EventEffect apply(Environment& env, Season& season, Weather& weather) override;
    std::string description() const override;
};

class PredatorRemoval : public Event {
public:
    PredatorRemoval() : Event("Predator Removal", 3) {}
    EventEffect apply(Environment& env, Season& season, Weather& weather) override;
    std::string description() const override;
};

class HeavyRain : public Event {
public:
    HeavyRain() : Event("Heavy Rain", 2) {}
    EventEffect apply(Environment& env, Season& season, Weather& weather) override;
    std::string description() const override;
};

class SeasonalChange : public Event {
public:
    SeasonalChange() : Event("Seasonal Change", 1) {}
    EventEffect apply(Environment& env, Season& season, Weather& weather) override;
    std::string description() const override;
};

Event* createEvent(int type);

#endif
