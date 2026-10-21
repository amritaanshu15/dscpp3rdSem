#include "Ecosystem.h"
#include <algorithm>
#include <cmath>

using namespace std;

Ecosystem::Ecosystem() : day(0), baselineAnimals(0) {}

Ecosystem::~Ecosystem() { clearAll(); }

void Ecosystem::clearAll()
{
    for (size_t i = 0; i < organisms.size(); i++) delete organisms[i];
    organisms.clear();
    web.clear();
}

void Ecosystem::loadDefaultSpecies()
{
    clearAll();
    env.reset();
    season.reset();
    weather.reset();
    slowLosses.clear();
    day = 0;

    addOrganism(new Plant    ( 1, "Grass",        299, 600));
    addOrganism(new Plant    ( 2, "Berry Bush",   267, 400));
    addOrganism(new Plant    ( 9, "Wildflower",   191, 300));

    addOrganism(new Herbivore( 3, "Deer",          20, 0.20));
    addOrganism(new Herbivore( 4, "Rabbit",        56, 0.09));
    addOrganism(new Herbivore(10, "Mouse",         45, 0.12));
    addOrganism(new Bird     ( 7, "Sparrow",       54, 0.08));

    addOrganism(new Carnivore( 5, "Tiger",          5, 0.08));
    addOrganism(new Carnivore( 6, "Fox",            8, 0.12));
    addOrganism(new Carnivore(11, "Snake",         16, 0.03));
    addOrganism(new Bird     (12, "Owl",           22, 0.03));

    addOrganism(new Bird     ( 8, "Eagle",         21, 0.08));

    web.addRelationship( 1, "Grass",      3, "Deer");
    web.addRelationship( 1, "Grass",      4, "Rabbit");
    web.addRelationship( 1, "Grass",     10, "Mouse");
    web.addRelationship( 2, "Berry Bush", 4, "Rabbit");
    web.addRelationship( 2, "Berry Bush", 7, "Sparrow");
    web.addRelationship( 9, "Wildflower",10, "Mouse");
    web.addRelationship( 9, "Wildflower", 7, "Sparrow");
    web.addRelationship( 3, "Deer",       5, "Tiger");
    web.addRelationship( 4, "Rabbit",     6, "Fox");
    web.addRelationship( 4, "Rabbit",     8, "Eagle");
    web.addRelationship( 7, "Sparrow",    8, "Eagle");
    web.addRelationship(10, "Mouse",     11, "Snake");
    web.addRelationship(10, "Mouse",     12, "Owl");
    web.addRelationship(11, "Snake",      8, "Eagle");

    refreshFood();
    baselineAnimals = totalPopulation() - totalPopulationOfType(PLANT);
}

int Ecosystem::indexOfId(int id) const
{
    for (size_t i = 0; i < organisms.size(); i++)
        if (organisms[i]->getId() == id) return (int)i;
    return -1;
}

bool Ecosystem::addOrganism(Organism* o)
{
    if (o == 0 || indexOfId(o->getId()) != -1) { delete o; return false; }
    organisms.push_back(o);
    return true;
}

bool Ecosystem::removeOrganism(int id)
{
    int i = indexOfId(id);
    if (i < 0) return false;
    delete organisms[i];
    organisms.erase(organisms.begin() + i);
    web.removeOrganism(id);
    refreshFood();
    return true;
}

Organism* Ecosystem::findOrganism(int id) const
{
    int i = indexOfId(id);
    return i < 0 ? 0 : organisms[i];
}

int Ecosystem::smallestFreeId() const
{
    for (int id = 1; id <= 20; id++)
        if (indexOfId(id) == -1) return id;
    return 0;
}

double Ecosystem::totalPopulationOfType(int type) const
{
    double sum = 0;
    for (size_t i = 0; i < organisms.size(); i++)
        if (organisms[i]->typeCode() == type) sum += organisms[i]->getPopulation();
    return sum;
}

double Ecosystem::totalPopulation() const
{
    double sum = 0;
    for (size_t i = 0; i < organisms.size(); i++) sum += organisms[i]->getPopulation();
    return sum;
}

bool Ecosystem::isPredator(int id) const
{
    vector<int> foods = web.foodsOf(id);
    for (size_t i = 0; i < foods.size(); i++) {
        Organism* f = findOrganism(foods[i]);
        if (f != 0 && f->typeCode() != PLANT) return true;
    }
    return false;
}

void Ecosystem::refreshFood()
{
    env.getFood().set(totalPopulationOfType(PLANT));
}

string Ecosystem::applyEvent(Event& e)
{
    EventEffect fx = e.apply(env, season, weather);

    for (int type = 0; type < 4; type++) {
        double total = fx.popMultiplier[type];
        if (total == 1.0) continue;

        if (fx.spreadDays <= 1) {
            scaleType(type, total);
        } else {

            SlowLoss loss;
            loss.type = type;
            loss.dailyFactor = pow(total, 1.0 / fx.spreadDays);
            loss.daysLeft = fx.spreadDays - 1;
            scaleType(type, loss.dailyFactor);
            if (loss.daysLeft > 0) slowLosses.push_back(loss);
        }
    }
    refreshFood();
    return fx.summary;
}

void Ecosystem::scaleType(int type, double factor)
{
    for (size_t i = 0; i < organisms.size(); i++)
        if (organisms[i]->typeCode() == type)
            organisms[i]->setPopulation(organisms[i]->getPopulation() * factor);
}

void Ecosystem::applySlowLosses()
{
    for (size_t i = 0; i < slowLosses.size(); ) {
        scaleType(slowLosses[i].type, slowLosses[i].dailyFactor);
        if (--slowLosses[i].daysLeft <= 0) slowLosses.erase(slowLosses.begin() + i);
        else i++;
    }
}

void Ecosystem::simulateDay()
{
    season.tickDay();
    weather.tickDay();
    env.tickDay();

    applySlowLosses();
    updateResources();
    applyResourceEffects();
    updatePopulations();
    refreshFood();
    day++;
}

void Ecosystem::updateResources()
{
    refreshFood();
    double plants  = totalPopulationOfType(PLANT);
    double animals = totalPopulation() - plants;

    double consumption = plants * 0.01 + animals * 0.03;
    env.getWater().add(season.waterRegen() + weather.waterChange() - consumption);

    env.getShelter().add(2);
}

void Ecosystem::applyResourceEffects()
{
    double foodR = env.getFood().ratio();
    double waterR = env.getWater().ratio();
    double shelterR = env.getShelter().ratio();

    for (size_t i = 0; i < organisms.size(); i++) {
        Organism* o = organisms[i];
        double p = o->getPopulation();
        int t = o->typeCode();

        if (t == HERBIVORE && foodR < 0.3)      p *= 0.95;
        if (t != PLANT && waterR < 0.2)         p *= 0.97;
        if (t == PLANT && waterR < 0.2)         p *= 0.98;
        if ((t == HERBIVORE || t == BIRD) && shelterR < 0.3) p *= 0.98;

        o->setPopulation(p);
    }
}

vector<string> Ecosystem::activeResourceEffects() const
{
    vector<string> list;
    if (env.getFood().ratio() < 0.3)    list.push_back("Rule 1 active: food is scarce, herbivores lose 5% per day");
    if (env.getWater().ratio() < 0.2)   list.push_back("Rule 2 active: water is critical, animals lose 3% and plants 2% per day");
    if (env.getShelter().ratio() < 0.3) list.push_back("Rule 3 active: shelter is damaged, herbivores and birds lose 2% per day");
    return list;
}

void Ecosystem::updatePopulations()
{
    const size_t n = organisms.size();
    vector<double> pop(n), need(n, 0.0), got(n, 0.0), lost(n, 0.0), totalWanted(n, 0.0), avail(n, 0.0);
    struct Want { size_t prey; double amount; };
    vector< vector<Want> > wants(n);

    for (size_t i = 0; i < n; i++) pop[i] = organisms[i]->getPopulation();

    for (size_t c = 0; c < n; c++) {
        Organism* o = organisms[c];
        if (o->typeCode() == PLANT) continue;
        need[c] = pop[c] * o->appetite();

        vector<int> foods = web.foodsOf(o->getId());
        vector<size_t> idx;
        for (size_t k = 0; k < foods.size(); k++) {
            int j = indexOfId(foods[k]);
            if (j >= 0) { idx.push_back((size_t)j); avail[c] += pop[j]; }
        }
        if (avail[c] <= 0 || need[c] <= 0) continue;
        for (size_t k = 0; k < idx.size(); k++) {
            double share = need[c] * pop[idx[k]] / avail[c];
            Want w = { idx[k], share };
            wants[c].push_back(w);
            totalWanted[idx[k]] += share;
        }
    }

    for (size_t c = 0; c < n; c++)
        for (size_t k = 0; k < wants[c].size(); k++) {
            size_t prey = wants[c][k].prey;
            double limit = (organisms[prey]->typeCode() == PLANT ? 0.5 : 0.03) * pop[prey];
            double scale = (totalWanted[prey] > limit) ? limit / totalWanted[prey] : 1.0;
            double eaten = wants[c][k].amount * scale;
            got[c]  += eaten;
            lost[prey] += eaten;
        }

    double waterR = env.getWater().ratio();
    double shelterR = env.getShelter().ratio();

    for (size_t i = 0; i < n; i++) {
        Organism* o = organisms[i];
        double p = pop[i], np;

        if (o->typeCode() == PLANT) {

            double K = o->capacity();
            double growth = p * o->birthRate() * (1.0 - p / K)
                          * season.growthFactor() * (0.3 + 0.7 * waterR) * env.getGrowthModifier();
            np = p + growth - p * o->deathRate() - lost[i];
            if (np > K) np = K;
        } else {
            double satisfaction = (need[i] > 0) ? min(1.0, got[i] / need[i]) : 1.0;
            double foodCover = (need[i] > 0) ? min(1.0, avail[i] / (need[i] * 120.0)) : 1.0;
            double shelterFactor = 0.7 + 0.3 * shelterR;

            double births = p * o->birthRate() * satisfaction * foodCover * shelterFactor;
            double deaths = p * o->deathRate();
            if (satisfaction < 0.7) deaths += p * (0.7 - satisfaction) * 0.25;
            np = p + births - deaths - lost[i];
        }
        if (np < 0.5) np = 0;
        o->setPopulation(np);
    }
}

double Ecosystem::calculateHealth() const
{
    if (organisms.empty()) return 0;

    int alive = 0;
    for (size_t i = 0; i < organisms.size(); i++)
        if (organisms[i]->getPopulation() >= 1) alive++;
    double biodiversity = 30.0 * alive / organisms.size();

    double resources = 30.0 * (min(1.0, env.getFood().ratio()) + env.getWater().ratio()
                               + env.getShelter().ratio()) / 3.0;

    double herb = totalPopulationOfType(HERBIVORE);
    double carn = totalPopulationOfType(CARNIVORE);
    double balance = 0;
    if (herb >= 1 && carn >= 1) {
        double r = herb / carn;
        if (r < 5)        balance = 20.0 * r / 5.0;
        else if (r <= 30) balance = 20.0;
        else              balance = 20.0 * max(0.0, 1.0 - (r - 30.0) / 60.0);
    } else if (herb >= 1) {
        balance = 3;
    }

    double animals = totalPopulation() - totalPopulationOfType(PLANT);
    double abundance = (baselineAnimals > 0) ? 20.0 * min(1.0, animals / baselineAnimals) : 20.0;

    return biodiversity + resources + balance + abundance;
}

string Ecosystem::healthLabel() const
{
    double h = calculateHealth();
    if (h >= 80) return "Excellent";
    if (h >= 60) return "Good";
    if (h >= 40) return "Fair";
    return "Poor";
}
