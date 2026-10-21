#ifndef RESOURCE_H
#define RESOURCE_H

#include <string>

class Resource {
protected:
    std::string name;
    double amount;
    double capacity;
public:
    Resource(const std::string& name, double amount, double capacity);
    virtual ~Resource() {}

    std::string getName() const     { return name; }
    double      getAmount() const   { return amount; }
    double      getCapacity() const { return capacity; }
    double      ratio() const;

    void set(double value);
    void add(double value);
    void reduce(double value);

    virtual std::string status() const;
    virtual std::string describe() const;
};

class Food : public Resource {
public:
    Food(double amount = 800, double capacity = 1000);
    std::string status() const override;
    std::string describe() const override;
};

class Water : public Resource {
public:
    Water(double amount = 700, double capacity = 1000);
    std::string status() const override;
    std::string describe() const override;
};

class Shelter : public Resource {
public:
    Shelter(double amount = 400, double capacity = 500);
    std::string status() const override;
    std::string describe() const override;
};

class Environment {
private:
    Food    food;
    Water   water;
    Shelter shelter;
    double  growthModifier;
    int     modifierDaysLeft;
public:
    Environment();

    Food&    getFood()    { return food; }
    Water&   getWater()   { return water; }
    Shelter& getShelter() { return shelter; }
    const Food&    getFood() const    { return food; }
    const Water&   getWater() const   { return water; }
    const Shelter& getShelter() const { return shelter; }

    void   setGrowthModifier(double multiplier, int days);
    double getGrowthModifier() const { return growthModifier; }
    int    getModifierDaysLeft() const { return modifierDaysLeft; }
    void   tickDay();
    void   reset();
};

#endif
