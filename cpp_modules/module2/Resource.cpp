#include "Resource.h"

using namespace std;

Resource::Resource(const string& name, double amount, double capacity)
    : name(name), amount(amount), capacity(capacity) {}

double Resource::ratio() const { return capacity > 0 ? amount / capacity : 0; }

void Resource::set(double value)
{
    amount = value;
    if (amount < 0) amount = 0;
    if (amount > capacity) amount = capacity;
}

void Resource::add(double value)    { set(amount + value); }
void Resource::reduce(double value) { set(amount - value); }

string Resource::status() const
{
    double r = ratio();
    if (r < 0.2) return "Critical";
    if (r < 0.5) return "Low";
    if (r < 0.8) return "Moderate";
    return "Abundant";
}

string Resource::describe() const { return name + " resource"; }

Food::Food(double amount, double capacity) : Resource("Food", amount, capacity) {}
string Food::status() const
{
    double r = ratio();
    if (r < 0.2) return "Scarce";
    if (r < 0.5) return "Limited";
    if (r < 0.8) return "Enough";
    return "Plentiful";
}
string Food::describe() const { return "Food = total plant biomass available to herbivores"; }

Water::Water(double amount, double capacity) : Resource("Water", amount, capacity) {}
string Water::status() const
{
    double r = ratio();
    if (r < 0.2) return "Dry";
    if (r < 0.5) return "Low";
    if (r < 0.8) return "Good";
    return "Full";
}
string Water::describe() const { return "Water = rivers, ponds and soil moisture"; }

Shelter::Shelter(double amount, double capacity) : Resource("Shelter", amount, capacity) {}
string Shelter::status() const
{
    double r = ratio();
    if (r < 0.2) return "Destroyed";
    if (r < 0.5) return "Damaged";
    if (r < 0.8) return "Fair";
    return "Healthy";
}
string Shelter::describe() const { return "Shelter = trees, bushes and burrows where animals hide"; }

Environment::Environment() : growthModifier(1.0), modifierDaysLeft(0) {}

void Environment::setGrowthModifier(double multiplier, int days)
{
    growthModifier = multiplier;
    modifierDaysLeft = days;
}

void Environment::tickDay()
{
    if (modifierDaysLeft > 0) {
        modifierDaysLeft--;
        if (modifierDaysLeft == 0) growthModifier = 1.0;
    }
}

void Environment::reset()
{
    food = Food();
    water = Water();
    shelter = Shelter();
    growthModifier = 1.0;
    modifierDaysLeft = 0;
}
