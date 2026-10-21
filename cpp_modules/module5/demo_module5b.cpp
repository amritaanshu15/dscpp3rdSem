#include <iostream>
#include <iomanip>
#include "Ecosystem.h"

using namespace std;

static void show(const Ecosystem& eco)
{
    cout << "Day " << setw(3) << eco.getDay() << " | ";
    for (size_t i = 0; i < eco.getOrganisms().size(); i++) {
        Organism* o = eco.getOrganisms()[i];
        cout << o->getName() << "=" << (int)(o->getPopulation() + 0.5) << " ";
    }
    cout << "| health " << (int)eco.calculateHealth() << " (" << eco.healthLabel() << ")\n";
}

int main()
{
    Ecosystem eco;
    eco.loadDefaultSpecies();
    show(eco);
    for (int d = 1; d <= 60; d++) {
        if (d == 20) {
            Drought drought;
            cout << "  >> Drought: " << eco.applyEvent(drought) << "\n";
        }
        eco.simulateDay();
        if (d % 10 == 0) show(eco);
    }
    return 0;
}
