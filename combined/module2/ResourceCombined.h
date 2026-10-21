#ifndef RESOURCE_COMBINED_H
#define RESOURCE_COMBINED_H

#include <string>
#include "../../c_modules/module2_linkedlist/pop_list.h"
#include "../../cpp_modules/module5_ecosystem/Ecosystem.h"

class ResourceCombined {
    PopList&   list;
    Ecosystem& eco;
public:
    ResourceCombined(PopList& list, Ecosystem& eco) : list(list), eco(eco) {}

    void syncToObjects();
    void syncToList();
    void rebuildList();

    std::string insert(const std::string& name, int type, double population);
    std::string remove(int id);
    std::string updatePopulation(int id, double population);
    std::string searchJson(int id) const;
    std::string listJson() const;

    std::string resourcesJson() const;
};

#endif
