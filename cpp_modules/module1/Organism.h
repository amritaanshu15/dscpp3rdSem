#ifndef ORGANISM_H
#define ORGANISM_H

#include <string>

enum OrganismType { PLANT = 0, HERBIVORE = 1, CARNIVORE = 2, BIRD = 3 };

class Organism {
protected:
    int         id;
    std::string name;
    double      population;

public:
    Organism(int id, const std::string& name, double population);
    virtual ~Organism() {}

    int         getId() const         { return id; }
    std::string getName() const       { return name; }
    double      getPopulation() const { return population; }
    void        setPopulation(double p) { population = (p < 0) ? 0 : p; }

    virtual int         typeCode() const;
    virtual std::string typeName() const;
    virtual std::string diet() const;
    virtual std::string describe() const;
    virtual double      birthRate() const;
    virtual double      deathRate() const;
    virtual double      appetite() const;
    virtual double      capacity() const;
};

class Plant : public Organism {
    double carryingCapacity;
public:
    Plant(int id, const std::string& name, double population, double capacity = 400);
    int         typeCode() const override;
    std::string typeName() const override;
    std::string diet() const override;
    std::string describe() const override;
    double      birthRate() const override;
    double      deathRate() const override;
    double      capacity() const override;
};

class Animal : public Organism {
protected:
    double appetiteValue;
public:
    Animal(int id, const std::string& name, double population, double appetite);
    std::string typeName() const override;
    std::string describe() const override;
    double      appetite() const override;
};

class Herbivore : public Animal {
public:
    Herbivore(int id, const std::string& name, double population, double appetite = 0.3);
    int         typeCode() const override;
    std::string typeName() const override;
    std::string diet() const override;
    std::string describe() const override;
    double      birthRate() const override;
    double      deathRate() const override;
};

class Carnivore : public Animal {
public:
    Carnivore(int id, const std::string& name, double population, double appetite = 0.1);
    int         typeCode() const override;
    std::string typeName() const override;
    std::string diet() const override;
    std::string describe() const override;
    double      birthRate() const override;
    double      deathRate() const override;
};

class Bird : public Animal {
public:
    Bird(int id, const std::string& name, double population, double appetite = 0.1);
    int         typeCode() const override;
    std::string typeName() const override;
    std::string diet() const override;
    std::string describe() const override;
    double      birthRate() const override;
    double      deathRate() const override;
};

Organism* createOrganism(int type, int id, const std::string& name, double population);

#endif
