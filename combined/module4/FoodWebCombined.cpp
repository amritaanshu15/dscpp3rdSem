#include "FoodWebCombined.h"
#include "../common/json.h"

using namespace std;

void FoodWebCombined::buildGraph(Graph& g) const
{
    graph_init(&g);
    const vector<Organism*>& all = eco.getOrganisms();
    for (size_t i = 0; i < all.size(); i++)
        graph_add_node(&g, all[i]->getId() - 1, all[i]->getName().c_str());

    const vector<FeedingRelationship>& rel = eco.getFoodWeb().getRelationships();
    for (size_t i = 0; i < rel.size(); i++)
        graph_add_edge(&g, rel[i].getFoodId() - 1, rel[i].getEaterId() - 1);
}

string FoodWebCombined::foodwebJson() const
{
    string nodes, edges;
    const vector<Organism*>& all = eco.getOrganisms();
    for (size_t i = 0; i < all.size(); i++) {
        Organism* o = all[i];

        FoodWebMember* m = eco.getFoodWeb().makeMember(o->getId(), o->getName(), eco.isPredator(o->getId()));
        if (i > 0) nodes += ",";
        nodes += "{\"id\":" + json::num(o->getId()) + ",\"name\":" + json::str(o->getName()) +
                 ",\"type\":" + json::num(o->typeCode()) + ",\"typeName\":" + json::str(o->typeName()) +
                 ",\"population\":" + json::whole(o->getPopulation()) +
                 ",\"role\":" + json::str(m->role()) + ",\"roleText\":" + json::str(m->describe()) + "}";
        delete m;
    }
    const vector<FeedingRelationship>& rel = eco.getFoodWeb().getRelationships();
    for (size_t i = 0; i < rel.size(); i++) {
        if (i > 0) edges += ",";
        edges += "{\"food\":" + json::num(rel[i].getFoodId()) + ",\"eater\":" + json::num(rel[i].getEaterId()) +
                 ",\"foodName\":" + json::str(rel[i].getFoodName()) + ",\"eaterName\":" + json::str(rel[i].getEaterName()) +
                 ",\"text\":" + json::str(rel[i].describe()) + "}";
    }
    return json::ok("\"nodes\":[" + nodes + "],\"edges\":[" + edges + "]");
}

string FoodWebCombined::addJson(int foodId, int eaterId)
{
    Organism* food  = eco.findOrganism(foodId);
    Organism* eater = eco.findOrganism(eaterId);
    if (food == 0 || eater == 0)         return json::error("Both organisms must exist");
    if (eater->typeCode() == PLANT)      return json::error("A plant cannot eat other organisms");
    if (foodId == eaterId)               return json::error("An organism cannot eat itself");
    if (!eco.getFoodWeb().addRelationship(foodId, food->getName(), eaterId, eater->getName()))
        return json::error("This relationship already exists");
    return json::ok("\"message\":" + json::str(food->getName() + " is now eaten by " + eater->getName()));
}

string FoodWebCombined::removeJson(int foodId, int eaterId)
{
    if (!eco.getFoodWeb().removeRelationship(foodId, eaterId))
        return json::error("That relationship does not exist");
    return json::ok("\"message\":\"Relationship removed\"");
}

string FoodWebCombined::traversalJson(const string& kind, int startId) const
{
    Graph g;
    int order[MAX_NODES];
    buildGraph(g);

    if (eco.findOrganism(startId) == 0) return json::error("No organism with ID " + json::num(startId));

    int n = (kind == "dfs") ? graph_dfs(&g, startId - 1, order)
                            : graph_bfs(&g, startId - 1, order);
    string list;
    for (int i = 0; i < n; i++) {
        if (i > 0) list += ",";
        list += "{\"step\":" + json::num(i + 1) + ",\"id\":" + json::num(order[i] + 1) +
                ",\"name\":" + json::str(g.names[order[i]]) + "}";
    }
    return json::ok("\"kind\":" + json::str(kind) + ",\"start\":" + json::num(startId) + ",\"count\":" + json::num(n) +
                    ",\"order\":[" + list + "]");
}

string FoodWebCombined::componentsJson() const
{
    Graph g;
    int comp[MAX_NODES];
    buildGraph(g);
    int groups = graph_components(&g, comp);

    string out;
    for (int c = 0; c < groups; c++) {
        string members;
        for (int i = 0; i < MAX_NODES; i++) {
            if (comp[i] != c) continue;
            if (!members.empty()) members += ",";
            members += "{\"id\":" + json::num(i + 1) + ",\"name\":" + json::str(g.names[i]) + "}";
        }
        if (c > 0) out += ",";
        out += "{\"group\":" + json::num(c + 1) + ",\"members\":[" + members + "]}";
    }
    return json::ok("\"groupCount\":" + json::num(groups) + ",\"groups\":[" + out + "]");
}
