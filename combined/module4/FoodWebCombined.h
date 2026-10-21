#ifndef FOODWEB_COMBINED_H
#define FOODWEB_COMBINED_H

#include <string>
#include "../../c_modules/module4_graph/graph.h"
#include "../../cpp_modules/module5_ecosystem/Ecosystem.h"

class FoodWebCombined {
    Ecosystem& eco;
public:
    explicit FoodWebCombined(Ecosystem& eco) : eco(eco) {}

    void buildGraph(Graph& g) const;

    std::string foodwebJson() const;
    std::string addJson(int foodId, int eaterId);
    std::string removeJson(int foodId, int eaterId);
    std::string traversalJson(const std::string& kind, int startId) const;
    std::string componentsJson() const;
};

#endif
