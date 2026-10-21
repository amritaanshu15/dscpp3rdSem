#include <iostream>
#include <vector>
#include "Event.h"

using namespace std;

int main()
{
    Environment env;
    Season season;
    Weather weather;

    vector<Event*> events;
    events.push_back(new Drought());
    events.push_back(new ForestFire());
    events.push_back(new PredatorRemoval());

    for (size_t i = 0; i < events.size(); i++) {

        EventEffect fx = events[i]->apply(env, season, weather);
        cout << events[i]->getName() << " (priority " << events[i]->getPriority() << "): "
                  << fx.summary << "\n";
    }
    cout << "Water now: " << env.getWater().getAmount() << "\n";
    for (size_t i = 0; i < events.size(); i++) delete events[i];
    return 0;
}
