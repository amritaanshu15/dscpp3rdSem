#ifndef POPULATION_COMBINED_H
#define POPULATION_COMBINED_H

#include <string>
#include "../../c_modules/module1_population/organism_array.h"
#include "../../c_modules/module2_linkedlist/pop_list.h"
#include "../../cpp_modules/module5_ecosystem/Ecosystem.h"

class PopulationCombined {
    PopList&   list;
    Ecosystem& eco;
public:
    PopulationCombined(PopList& list, Ecosystem& eco) : list(list), eco(eco) {}

    void        buildArray(OrganismArray& arr) const;
    std::string recordJson(const OrganismRec& r) const;

    std::string organismsJson() const;
    std::string searchByIdJson(int id) const;
    std::string searchByNameJson(const std::string& name) const;
    std::string statsJson() const;
};

#endif
