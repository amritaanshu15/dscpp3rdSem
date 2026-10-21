#ifndef ECOSYSTEM_H
#define ECOSYSTEM_H

#include <string>
#include <vector>
#include "../module1_organisms/Organism.h"
#include "../module2_resources/Resource.h"
#include "../module3_events/Event.h"
#include "../module4_foodweb/FoodWeb.h"

class Ecosystem {
private:
    std::vector<Organism*> organisms;
    Environment env;
    Season      season;
    Weather     weather;
    FoodWeb     web;
    int         day;
    double      baselineAnimals;

    struct SlowLoss {
        int    type;
        double dailyFactor;
        int    daysLeft;
    };
    std::vector<SlowLoss> slowLosses;
    void scaleType(int type, double factor);
    void applySlowLosses();

    void updateResources();
    void applyResourceEffects();
    void updatePopulations();

    int indexOfId(int id) const;

public:
    Ecosystem();
    ~Ecosystem();
    Ecosystem(const Ecosystem&) = delete;
    Ecosystem& operator=(const Ecosystem&) = delete;

    void loadDefaultSpecies();
    void clearAll();

    bool       addOrganism(Organism* o);
    bool       removeOrganism(int id);
    Organism*  findOrganism(int id) const;
    int        smallestFreeId() const;
    const std::vector<Organism*>& getOrganisms() const { return organisms; }
    double     totalPopulationOfType(int type) const;
    double     totalPopulation() const;
    bool       isPredator(int id) const;

    Environment& getEnvironment() { return env; }
    Season&      getSeason()      { return season; }
    Weather&     getWeather()     { return weather; }
    FoodWeb&     getFoodWeb()     { return web; }
    const Environment& getEnvironment() const { return env; }
    const Season&      getSeason() const      { return season; }
    const Weather&     getWeather() const     { return weather; }
    const FoodWeb&     getFoodWeb() const     { return web; }
    int          getDay() const   { return day; }

    std::string applyEvent(Event& e);
    void        simulateDay();
    void        refreshFood();

    double      calculateHealth() const;
    std::string healthLabel() const;
    std::vector<std::string> activeResourceEffects() const;
};

#endif
