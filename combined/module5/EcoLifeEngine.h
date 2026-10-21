#ifndef ECOLIFE_ENGINE_H
#define ECOLIFE_ENGINE_H

#include <string>
#include <vector>
#include <random>
#include "../../c_modules/module2_linkedlist/pop_list.h"
#include "../../c_modules/module3_events/event_queue.h"
#include "../../c_modules/module5_analysis/analysis.h"
#include "../../cpp_modules/module5_ecosystem/Ecosystem.h"
#include "../module1/PopulationCombined.h"
#include "../module2/ResourceCombined.h"
#include "../module3/EventCombined.h"
#include "../module4/FoodWebCombined.h"

struct DayRecord {
    int day;
    double health;
    double plants, herbivores, carnivores, birds;
};

class EcoLifeEngine {
private:

    Ecosystem eco;

    PopList      list;
    EventQueue   fifo;
    EventPQ      pq;
    EventHistory history;

    PopulationCombined m1;
    ResourceCombined   m2;
    EventCombined      m3;
    FoodWebCombined    m4;

    std::mt19937           rng;
    std::string            scenario;
    bool                   randomEvents;
    std::vector<DayRecord> trend;

    void        recordDay();
    int         advanceOneDay();
    std::string stateJson() const;
    std::string analysisJson() const;
    std::string analysisSearchJson(const std::string& name) const;
    std::string historyJson() const;
    std::string compareJson(int days);

    bool handleSimulation(const std::vector<std::string>& args, std::string& result);
    bool handlePopulation(const std::vector<std::string>& args, std::string& result);
    bool handleResources(const std::vector<std::string>& args, std::string& result);
    bool handleEvents(const std::vector<std::string>& args, std::string& result);
    bool handleFoodWeb(const std::vector<std::string>& args, std::string& result);

public:
    EcoLifeEngine();
    ~EcoLifeEngine();
    EcoLifeEngine(const EcoLifeEngine&) = delete;
    EcoLifeEngine& operator=(const EcoLifeEngine&) = delete;

    void reset();
    bool applyScenario(const std::string& name);
    int  advance(int days);
    void setRandomEvents(bool on) { randomEvents = on; }

    std::string handle(const std::vector<std::string>& args);
};

#endif
