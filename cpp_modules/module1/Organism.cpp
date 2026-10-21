#include "Organism.h"

using namespace std;

Organism::Organism(int id, const string& name, double population)
    : id(id), name(name), population(population) {}

int         Organism::typeCode() const { return -1; }
string Organism::typeName() const { return "Organism"; }
string Organism::diet() const     { return "Unknown"; }
string Organism::describe() const { return name + " is a living organism."; }
double      Organism::birthRate() const { return 0.03; }
double      Organism::deathRate() const { return 0.02; }
double      Organism::appetite() const  { return 0.0; }
double      Organism::capacity() const  { return 0.0; }

Plant::Plant(int id, const string& name, double population, double capacity)
    : Organism(id, name, population), carryingCapacity(capacity) {}

int         Plant::typeCode() const { return PLANT; }
string Plant::typeName() const { return "Plant"; }
string Plant::diet() const     { return "Sunlight and water (photosynthesis)"; }
string Plant::describe() const { return name + " is a producer: it makes its own food and feeds the whole food web."; }
double      Plant::birthRate() const { return 0.25; }
double      Plant::deathRate() const { return 0.01; }
double      Plant::capacity() const  { return carryingCapacity; }

Animal::Animal(int id, const string& name, double population, double appetite)
    : Organism(id, name, population), appetiteValue(appetite) {}

string Animal::typeName() const { return "Animal"; }
string Animal::describe() const { return name + " is a consumer: it must eat other organisms to survive."; }
double      Animal::appetite() const { return appetiteValue; }

Herbivore::Herbivore(int id, const string& name, double population, double appetite)
    : Animal(id, name, population, appetite) {}

int         Herbivore::typeCode() const { return HERBIVORE; }
string Herbivore::typeName() const { return "Herbivore"; }
string Herbivore::diet() const     { return "Plants"; }
string Herbivore::describe() const { return name + " is a herbivore: it eats plants and is food for predators."; }
double      Herbivore::birthRate() const { return 0.05; }
double      Herbivore::deathRate() const { return 0.02; }

Carnivore::Carnivore(int id, const string& name, double population, double appetite)
    : Animal(id, name, population, appetite) {}

int         Carnivore::typeCode() const { return CARNIVORE; }
string Carnivore::typeName() const { return "Carnivore"; }
string Carnivore::diet() const     { return "Other animals"; }
string Carnivore::describe() const { return name + " is a carnivore: a top predator that keeps prey numbers in balance."; }
double      Carnivore::birthRate() const { return 0.03; }
double      Carnivore::deathRate() const { return 0.015; }

Bird::Bird(int id, const string& name, double population, double appetite)
    : Animal(id, name, population, appetite) {}

int         Bird::typeCode() const { return BIRD; }
string Bird::typeName() const { return "Bird"; }
string Bird::diet() const     { return "Seeds, berries or small animals"; }
string Bird::describe() const { return name + " is a bird: it can fly, and eats seeds, berries or small animals."; }
double      Bird::birthRate() const { return 0.045; }
double      Bird::deathRate() const { return 0.03; }

Organism* createOrganism(int type, int id, const string& name, double population)
{
    switch (type) {
        case PLANT:     return new Plant(id, name, population);
        case HERBIVORE: return new Herbivore(id, name, population);
        case CARNIVORE: return new Carnivore(id, name, population);
        case BIRD:      return new Bird(id, name, population);
    }
    return 0;
}
