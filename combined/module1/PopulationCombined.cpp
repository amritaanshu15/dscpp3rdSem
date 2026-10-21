#include "PopulationCombined.h"
#include "../common/json.h"

using namespace std;

void PopulationCombined::buildArray(OrganismArray& arr) const
{
    arr_init(&arr);
    for (PopNode* n = list.head; n != NULL; n = n->next)
        arr_add(&arr, n->id, n->name, n->type, (int)(n->population + 0.5));
}

string PopulationCombined::recordJson(const OrganismRec& r) const
{
    Organism* o = eco.findOrganism(r.id);
    string s = "{";
    s += "\"id\":" + json::num(r.id);
    s += ",\"name\":" + json::str(r.name);
    s += ",\"type\":" + json::num(r.type);
    s += ",\"typeName\":" + json::str(eco_type_name(r.type));
    s += ",\"population\":" + json::num(r.population);
    if (o != 0) {
        s += ",\"diet\":" + json::str(o->diet());
        s += ",\"description\":" + json::str(o->describe());
        s += ",\"appetite\":" + json::num(o->appetite(), 2);
        s += ",\"birthRate\":" + json::num(o->birthRate(), 3);
        s += ",\"deathRate\":" + json::num(o->deathRate(), 3);
        s += ",\"capacity\":" + json::num(o->capacity());
    }
    return s + "}";
}

string PopulationCombined::organismsJson() const
{
    OrganismArray arr;
    buildArray(arr);
    string items;
    for (int i = 0; i < arr.count; i++) {
        if (i > 0) items += ",";
        items += recordJson(arr.items[i]);
    }
    return json::ok("\"count\":" + json::num(arr.count) + ",\"organisms\":[" + items + "]");
}

string PopulationCombined::searchByIdJson(int id) const
{
    OrganismArray arr;
    buildArray(arr);
    int index = arr_search_by_id(&arr, id);
    if (index < 0) return json::error("No organism with ID " + json::num(id));
    return json::ok("\"index\":" + json::num(index) + ",\"comparisons\":" + json::num(index + 1) +
                    ",\"organism\":" + recordJson(arr.items[index]));
}

string PopulationCombined::searchByNameJson(const string& name) const
{
    OrganismArray arr;
    buildArray(arr);
    int index = arr_search_by_name(&arr, name.c_str());
    if (index < 0) return json::error("No organism named '" + name + "'");
    return json::ok("\"index\":" + json::num(index) + ",\"comparisons\":" + json::num(index + 1) +
                    ",\"organism\":" + recordJson(arr.items[index]));
}

string PopulationCombined::statsJson() const
{
    OrganismArray arr;
    PopStats st;
    buildArray(arr);
    arr_stats(&arr, &st);

    string types;
    for (int t = 0; t < TYPE_COUNT; t++) {
        if (t > 0) types += ",";
        types += "{\"type\":" + json::num(t) + ",\"typeName\":" + json::str(eco_type_name(t)) +
                 ",\"species\":" + json::num(arr_count_by_type(&arr, t)) +
                 ",\"population\":" + json::num(st.populationByType[t]) + "}";
    }
    return json::ok("\"totalSpecies\":" + json::num(st.totalSpecies) +
                    ",\"totalPopulation\":" + json::num(st.totalPopulation) +
                    ",\"byType\":[" + types + "]");
}
