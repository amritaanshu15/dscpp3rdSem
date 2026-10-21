#ifndef FOODWEB_H
#define FOODWEB_H

#include <string>
#include <vector>

class FoodWebMember {
protected:
    int         id;
    std::string name;
public:
    FoodWebMember(int id, const std::string& name) : id(id), name(name) {}
    virtual ~FoodWebMember() {}
    int         getId() const   { return id; }
    std::string getName() const { return name; }
    virtual std::string role() const = 0;
    virtual std::string describe() const = 0;
};

class Predator : public FoodWebMember {
public:
    Predator(int id, const std::string& name) : FoodWebMember(id, name) {}
    std::string role() const override;
    std::string describe() const override;
};

class Prey : public FoodWebMember {
public:
    Prey(int id, const std::string& name) : FoodWebMember(id, name) {}
    std::string role() const override;
    std::string describe() const override;
};

class FeedingRelationship {
    int         foodId;
    int         eaterId;
    std::string foodName;
    std::string eaterName;
public:
    FeedingRelationship(int foodId, const std::string& foodName, int eaterId, const std::string& eaterName);
    int         getFoodId() const   { return foodId; }
    int         getEaterId() const  { return eaterId; }
    std::string getFoodName() const { return foodName; }
    std::string getEaterName() const { return eaterName; }
    std::string describe() const;
};

class FoodWeb {
    std::vector<FeedingRelationship> relations;
public:
    bool addRelationship(int foodId, const std::string& foodName, int eaterId, const std::string& eaterName);
    bool removeRelationship(int foodId, int eaterId);
    bool hasRelationship(int foodId, int eaterId) const;
    void removeOrganism(int id);
    void clear();

    std::vector<int> foodsOf(int eaterId) const;
    std::vector<int> eatersOf(int foodId) const;
    const std::vector<FeedingRelationship>& getRelationships() const { return relations; }

    FoodWebMember* makeMember(int id, const std::string& name, bool isPredator) const;
};

#endif
