#include "ResourceCombined.h"
#include "../common/json.h"

using namespace std;

void ResourceCombined::syncToObjects()
{
    for (PopNode* n = list.head; n != NULL; n = n->next) {
        Organism* o = eco.findOrganism(n->id);
        if (o != 0) o->setPopulation(n->population);
    }
}

void ResourceCombined::syncToList()
{
    const vector<Organism*>& all = eco.getOrganisms();
    for (size_t i = 0; i < all.size(); i++)
        list_update_population(&list, all[i]->getId(), all[i]->getPopulation());
}

void ResourceCombined::rebuildList()
{
    list_free(&list);
    list_init(&list);
    const vector<Organism*>& all = eco.getOrganisms();
    for (size_t i = 0; i < all.size(); i++)
        list_insert(&list, all[i]->getId(), all[i]->getName().c_str(), all[i]->typeCode(), all[i]->getPopulation());
}

string ResourceCombined::insert(const string& rawName, int type, double population)
{
    string name = rawName;
    while (!name.empty() && name[0] == ' ') name.erase(0, 1);
    while (!name.empty() && name[name.size() - 1] == ' ') name.erase(name.size() - 1);
    if (name.size() > NAME_LEN - 1) name = name.substr(0, NAME_LEN - 1);

    if (name.empty())                         return json::error("Name is required");
    if (type < 0 || type >= TYPE_COUNT)       return json::error("Type must be 0 (Plant), 1 (Herbivore), 2 (Carnivore) or 3 (Bird)");
    if (population < 0 || population > 100000) return json::error("Population must be between 0 and 100000");

    const vector<Organism*>& all = eco.getOrganisms();
    for (size_t i = 0; i < all.size(); i++)
        if (eco_stricmp(all[i]->getName().c_str(), name.c_str()) == 0)
            return json::error("An organism named '" + name + "' already exists");

    int id = eco.smallestFreeId();
    if (id == 0) return json::error("The ecosystem is full (maximum " + json::num(MAX_ORGANISMS) + " organisms)");

    Organism* obj = createOrganism(type, id, name, population);
    eco.addOrganism(obj);
    list_insert(&list, id, name.c_str(), type, population);

    return json::ok("\"id\":" + json::num(id) + ",\"message\":" + json::str(name + " added (ID " + json::num(id) + ")") +
                    ",\"listCount\":" + json::num(list_count(&list)));
}

string ResourceCombined::remove(int id)
{
    PopNode* node = list_search(&list, id);
    if (node == NULL) return json::error("No organism with ID " + json::num(id));
    string name = node->name;

    list_delete(&list, id);
    eco.removeOrganism(id);
    return json::ok("\"message\":" + json::str(name + " removed") + ",\"listCount\":" + json::num(list_count(&list)));
}

string ResourceCombined::updatePopulation(int id, double population)
{
    if (population < 0 || population > 100000) return json::error("Population must be between 0 and 100000");
    if (!list_update_population(&list, id, population)) return json::error("No organism with ID " + json::num(id));
    Organism* o = eco.findOrganism(id);
    if (o != 0) o->setPopulation(population);
    eco.refreshFood();
    return json::ok("\"message\":" + json::str("Population updated to " + json::whole(population)));
}

string ResourceCombined::searchJson(int id) const
{
    PopNode* n = list_search(&list, id);
    if (n == NULL) return json::error("No organism with ID " + json::num(id) + " in the linked list");
    return json::ok("\"node\":{\"id\":" + json::num(n->id) + ",\"name\":" + json::str(n->name) +
                    ",\"typeName\":" + json::str(eco_type_name(n->type)) +
                    ",\"population\":" + json::whole(n->population) + "}");
}

string ResourceCombined::listJson() const
{
    string nodes;
    for (PopNode* n = list.head; n != NULL; n = n->next) {
        if (!nodes.empty()) nodes += ",";
        nodes += "{\"id\":" + json::num(n->id) + ",\"name\":" + json::str(n->name) +
                 ",\"type\":" + json::num(n->type) + ",\"typeName\":" + json::str(eco_type_name(n->type)) +
                 ",\"population\":" + json::whole(n->population) + "}";
    }
    return json::ok("\"count\":" + json::num(list_count(&list)) + ",\"nodes\":[" + nodes + "]");
}

static string resourceJson(const Resource& r)
{
    return "{\"name\":" + json::str(r.getName()) + ",\"amount\":" + json::whole(r.getAmount()) +
           ",\"capacity\":" + json::whole(r.getCapacity()) + ",\"percent\":" + json::num(r.ratio() * 100, 0) +
           ",\"status\":" + json::str(r.status()) + ",\"description\":" + json::str(r.describe()) + "}";
}

string ResourceCombined::resourcesJson() const
{
    const Environment& env = eco.getEnvironment();

    string effects;
    vector<string> active = eco.activeResourceEffects();
    for (size_t i = 0; i < active.size(); i++) {
        if (i > 0) effects += ",";
        effects += json::str(active[i]);
    }

    string herb;
    for (PopNode* n = list.head; n != NULL; n = n->next) {
        if (n->type != TYPE_HERBIVORE) continue;
        if (!herb.empty()) herb += ",";
        herb += "{\"name\":" + json::str(n->name) + ",\"population\":" + json::whole(n->population) + "}";
    }

    return json::ok("\"food\":" + resourceJson(env.getFood()) +
                    ",\"water\":" + resourceJson(env.getWater()) +
                    ",\"shelter\":" + resourceJson(env.getShelter()) +
                    ",\"growthModifier\":" + json::num(env.getGrowthModifier(), 2) +
                    ",\"modifierDaysLeft\":" + json::num(env.getModifierDaysLeft()) +
                    ",\"season\":" + json::str(eco.getSeason().name()) +
                    ",\"weather\":" + json::str(eco.getWeather().name()) +
                    ",\"activeEffects\":[" + effects + "]" +
                    ",\"herbivores\":[" + herb + "]");
}
