#include "FoodWeb.h"

using namespace std;

string Predator::role() const     { return "Predator"; }
string Predator::describe() const { return name + " is a predator: it hunts other animals."; }

string Prey::role() const     { return "Prey"; }
string Prey::describe() const { return name + " is prey / a producer: it is food for others."; }

FeedingRelationship::FeedingRelationship(int foodId, const string& foodName,
                                         int eaterId, const string& eaterName)
    : foodId(foodId), eaterId(eaterId), foodName(foodName), eaterName(eaterName) {}

string FeedingRelationship::describe() const
{
    return foodName + " is eaten by " + eaterName;
}

bool FoodWeb::addRelationship(int foodId, const string& foodName, int eaterId, const string& eaterName)
{
    if (foodId == eaterId) return false;
    if (hasRelationship(foodId, eaterId)) return false;
    relations.push_back(FeedingRelationship(foodId, foodName, eaterId, eaterName));
    return true;
}

bool FoodWeb::removeRelationship(int foodId, int eaterId)
{
    for (size_t i = 0; i < relations.size(); i++) {
        if (relations[i].getFoodId() == foodId && relations[i].getEaterId() == eaterId) {
            relations.erase(relations.begin() + i);
            return true;
        }
    }
    return false;
}

bool FoodWeb::hasRelationship(int foodId, int eaterId) const
{
    for (size_t i = 0; i < relations.size(); i++)
        if (relations[i].getFoodId() == foodId && relations[i].getEaterId() == eaterId) return true;
    return false;
}

void FoodWeb::removeOrganism(int id)
{
    for (size_t i = relations.size(); i > 0; i--) {
        const FeedingRelationship& r = relations[i - 1];
        if (r.getFoodId() == id || r.getEaterId() == id) relations.erase(relations.begin() + (i - 1));
    }
}

void FoodWeb::clear() { relations.clear(); }

vector<int> FoodWeb::foodsOf(int eaterId) const
{
    vector<int> result;
    for (size_t i = 0; i < relations.size(); i++)
        if (relations[i].getEaterId() == eaterId) result.push_back(relations[i].getFoodId());
    return result;
}

vector<int> FoodWeb::eatersOf(int foodId) const
{
    vector<int> result;
    for (size_t i = 0; i < relations.size(); i++)
        if (relations[i].getFoodId() == foodId) result.push_back(relations[i].getEaterId());
    return result;
}

FoodWebMember* FoodWeb::makeMember(int id, const string& name, bool isPredator) const
{
    if (isPredator) return new Predator(id, name);
    return new Prey(id, name);
}
