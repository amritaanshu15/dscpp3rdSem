#include <iostream>
#include <vector>
#include "Organism.h"

using namespace std;

int main()
{

    vector<Organism*> list;
    list.push_back(new Plant(1, "Grass", 500));
    list.push_back(new Herbivore(2, "Deer", 60));
    list.push_back(new Carnivore(3, "Tiger", 6));
    list.push_back(new Bird(4, "Sparrow", 80));

    for (size_t i = 0; i < list.size(); i++) {
        Organism* o = list[i];

        cout << o->getName() << " [" << o->typeName() << "] eats: " << o->diet() << "\n"
                  << "   " << o->describe() << "\n";
    }
    for (size_t i = 0; i < list.size(); i++) delete list[i];
    return 0;
}
