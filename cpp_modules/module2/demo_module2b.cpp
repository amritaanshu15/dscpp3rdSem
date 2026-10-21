#include <iostream>
#include "Resource.h"

using namespace std;

int main()
{
    Environment env;
    cout << env.getFood().describe()  << " -> " << env.getFood().status()  << "\n";
    cout << env.getWater().describe() << " -> " << env.getWater().status() << "\n";

    env.getWater().reduce(600);
    cout << "After losing 600 water: " << env.getWater().getAmount()
              << " (" << env.getWater().status() << ")\n";
    return 0;
}
