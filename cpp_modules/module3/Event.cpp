#include "Event.h"

using namespace std;

Season::Season() : index(0), daysInSeason(0) {}

void Season::tickDay()
{
    daysInSeason++;
    if (daysInSeason >= LENGTH) forceNext();
}

void Season::forceNext()
{
    index = (index + 1) % 4;
    daysInSeason = 0;
}

void Season::reset() { index = 0; daysInSeason = 0; }

string Season::name() const
{
    static const char* names[4] = { "Spring", "Summer", "Autumn", "Winter" };
    return names[index];
}

double Season::growthFactor() const
{
    static const double f[4] = { 1.2, 1.0, 0.8, 0.5 };
    return f[index];
}

double Season::waterRegen() const
{
    static const double w[4] = { 22, 14, 18, 16 };
    return w[index];
}

Weather::Weather() : condition(CLEAR), daysLeft(0) {}

void Weather::set(int c, int days) { condition = c; daysLeft = days; }

void Weather::tickDay()
{
    if (daysLeft > 0) {
        daysLeft--;
        if (daysLeft == 0) condition = CLEAR;
    }
}

void Weather::reset() { condition = CLEAR; daysLeft = 0; }

string Weather::name() const
{
    if (condition == RAINY) return "Rainy";
    if (condition == DRY)   return "Dry";
    return "Clear";
}

double Weather::waterChange() const
{
    if (condition == RAINY) return 25;
    if (condition == DRY)   return -15;
    return 0;
}

EventEffect Drought::apply(Environment& env, Season&, Weather& weather)
{
    EventEffect fx;
    env.getWater().reduce(env.getWater().getCapacity() * 0.60);
    env.setGrowthModifier(0.3, 15);
    weather.set(Weather::DRY, 15);
    fx.popMultiplier[PLANT] = 0.70;
    fx.spreadDays = 10;
    fx.summary = "Water -60%, plant growth reduced for 15 days, plants -30% over 10 days";
    return fx;
}
string Drought::description() const { return "A long dry period. Water and plant growth drop."; }

EventEffect ForestFire::apply(Environment& env, Season&, Weather&)
{
    EventEffect fx;
    env.getShelter().reduce(env.getShelter().getCapacity() * 0.50);
    env.setGrowthModifier(0.7, 5);
    fx.popMultiplier[PLANT]     = 0.35;
    fx.popMultiplier[HERBIVORE] = 0.70;
    fx.popMultiplier[CARNIVORE] = 0.85;
    fx.popMultiplier[BIRD]      = 0.75;
    fx.spreadDays = 4;
    fx.summary = "Plants -65%, herbivores -30%, carnivores -15%, birds -25% (over 4 days), shelter -50%";
    return fx;
}
string ForestFire::description() const { return "Fire burns vegetation and shelter; animals die or flee."; }

EventEffect PredatorRemoval::apply(Environment&, Season&, Weather&)
{
    EventEffect fx;
    fx.popMultiplier[CARNIVORE] = 0.0;
    fx.summary = "All carnivores removed: prey will no longer be hunted";
    return fx;
}
string PredatorRemoval::description() const { return "All carnivores are removed from the ecosystem."; }

EventEffect HeavyRain::apply(Environment& env, Season&, Weather& weather)
{
    EventEffect fx;
    env.getWater().add(env.getWater().getCapacity() * 0.40);
    env.setGrowthModifier(1.3, 5);
    weather.set(Weather::RAINY, 4);
    fx.summary = "Water +40%, plant growth boosted for 5 days";
    return fx;
}
string HeavyRain::description() const { return "Strong rain fills rivers and helps plants grow."; }

EventEffect SeasonalChange::apply(Environment&, Season& season, Weather&)
{
    EventEffect fx;
    season.forceNext();
    fx.summary = "Season changed to " + season.name();
    return fx;
}
string SeasonalChange::description() const { return "The season changes immediately."; }

Event* createEvent(int type)
{
    switch (type) {
        case DROUGHT:          return new Drought();
        case FOREST_FIRE:      return new ForestFire();
        case PREDATOR_REMOVAL: return new PredatorRemoval();
        case HEAVY_RAIN:       return new HeavyRain();
        case SEASONAL_CHANGE:  return new SeasonalChange();
    }
    return 0;
}
