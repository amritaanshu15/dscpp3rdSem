#include <iostream>
#include "FoodWeb.h"

using namespace std;

int main()
{
    FoodWeb web;
    web.addRelationship(1, "Grass", 2, "Deer");
    web.addRelationship(2, "Deer", 3, "Tiger");
    web.addRelationship(1, "Grass", 4, "Rabbit");
    web.addRelationship(4, "Rabbit", 5, "Fox");

    for (size_t i = 0; i < web.getRelationships().size(); i++)
        cout << web.getRelationships()[i].describe() << "\n";

    cout << "Eaters of Grass: " << web.eatersOf(1).size() << "\n";

    FoodWebMember* m1 = web.makeMember(3, "Tiger", true);
    FoodWebMember* m2 = web.makeMember(2, "Deer", false);
    cout << m1->describe() << "\n" << m2->describe() << "\n";
    delete m1; delete m2;
    return 0;
}
