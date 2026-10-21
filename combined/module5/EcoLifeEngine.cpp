#include "EcoLifeEngine.h"
#include "../common/json.h"
#include <cstdlib>
#include <cerrno>

using namespace std;

static bool toInt(const string& s, int& out)
{
    if (s.empty()) return false;
    char* end = 0;
    errno = 0;
    long v = strtol(s.c_str(), &end, 10);
    if (*end != '\0' || errno != 0) return false;
    out = (int)v;
    return true;
}

static bool toDouble(const string& s, double& out)
{
    if (s.empty()) return false;
    char* end = 0;
    out = strtod(s.c_str(), &end);
    return *end == '\0';
}

static string arg(const vector<string>& a, size_t i)
{
    return i < a.size() ? a[i] : string();
}

EcoLifeEngine::EcoLifeEngine()
    : m1(list, eco), m2(list, eco), m3(fifo, pq, history, eco), m4(eco), rng(42), scenario("normal"), randomEvents(false)
{
    list_init(&list);
    reset();
}

EcoLifeEngine::~EcoLifeEngine()
{
    list_free(&list);
}

void EcoLifeEngine::reset()
{
    eco.loadDefaultSpecies();
    m2.rebuildList();
    m3.reset();
    rng.seed(42);
    scenario = "normal";
    trend.clear();
    recordDay();
}

void EcoLifeEngine::recordDay()
{
    DayRecord r;
    r.day = eco.getDay();
    r.health = eco.calculateHealth();
    r.plants = eco.totalPopulationOfType(PLANT);
    r.herbivores = eco.totalPopulationOfType(HERBIVORE);
    r.carnivores = eco.totalPopulationOfType(CARNIVORE);
    r.birds = eco.totalPopulationOfType(BIRD);
    trend.push_back(r);
    if (trend.size() > 1000) trend.erase(trend.begin());
}

bool EcoLifeEngine::applyScenario(const string& name)
{
    int eventType = 0;
    if      (name == "normal")   eventType = 0;
    else if (name == "drought")  eventType = DROUGHT;
    else if (name == "fire")     eventType = FOREST_FIRE;
    else if (name == "predator") eventType = PREDATOR_REMOVAL;
    else return false;

    reset();
    scenario = name;
    if (eventType != 0) {
        m3.enqueue(eventType, true);
        m3.processAll();
        recordDay();
    }
    return true;
}

int EcoLifeEngine::advanceOneDay()
{

    if (randomEvents) {
        uniform_real_distribution<double> chance(0.0, 1.0);
        if (chance(rng) < 0.03)
            m3.enqueue(chance(rng) < 0.7 ? HEAVY_RAIN : DROUGHT, true);
    }

    int processed = m3.processAll();

    eco.simulateDay();
    recordDay();
    return processed;
}

int EcoLifeEngine::advance(int days)
{
    int total = 0;
    for (int d = 0; d < days; d++) total += advanceOneDay();
    return total;
}

string EcoLifeEngine::stateJson() const
{
    const Environment& env = eco.getEnvironment();
    string species;
    const vector<Organism*>& all = eco.getOrganisms();
    for (size_t i = 0; i < all.size(); i++) {
        if (i > 0) species += ",";
        species += "{\"id\":" + json::num(all[i]->getId()) + ",\"name\":" + json::str(all[i]->getName()) +
                   ",\"type\":" + json::num(all[i]->typeCode()) + ",\"typeName\":" + json::str(all[i]->typeName()) +
                   ",\"population\":" + json::whole(all[i]->getPopulation()) + "}";
    }

    string s = "{";
    s += "\"day\":" + json::num(eco.getDay());
    s += ",\"scenario\":" + json::str(scenario);
    s += ",\"season\":" + json::str(eco.getSeason().name());
    s += ",\"weather\":" + json::str(eco.getWeather().name());
    s += ",\"health\":" + json::num(eco.calculateHealth(), 1);
    s += ",\"healthLabel\":" + json::str(eco.healthLabel());
    s += ",\"totalOrganisms\":" + json::whole(eco.totalPopulation());
    s += ",\"speciesCount\":" + json::num((double)all.size());
    s += ",\"plants\":" + json::whole(eco.totalPopulationOfType(PLANT));
    s += ",\"herbivores\":" + json::whole(eco.totalPopulationOfType(HERBIVORE));
    s += ",\"carnivores\":" + json::whole(eco.totalPopulationOfType(CARNIVORE));
    s += ",\"birds\":" + json::whole(eco.totalPopulationOfType(BIRD));
    s += ",\"food\":" + json::whole(env.getFood().getAmount());
    s += ",\"foodCapacity\":" + json::whole(env.getFood().getCapacity());
    s += ",\"water\":" + json::whole(env.getWater().getAmount());
    s += ",\"waterCapacity\":" + json::whole(env.getWater().getCapacity());
    s += ",\"shelter\":" + json::whole(env.getShelter().getAmount());
    s += ",\"shelterCapacity\":" + json::whole(env.getShelter().getCapacity());
    s += ",\"pendingEvents\":" + json::num(m3.pendingCount());
    s += ",\"randomEvents\":" + json::boolean(randomEvents);
    s += ",\"growthModifier\":" + json::num(env.getGrowthModifier(), 2);
    s += ",\"modifierDaysLeft\":" + json::num(env.getModifierDaysLeft());
    s += ",\"lastEvent\":" + m3.lastEventJson();
    s += ",\"species\":[" + species + "]";
    s += "}";
    return s;
}

string EcoLifeEngine::historyJson() const
{
    string s;
    for (size_t i = 0; i < trend.size(); i++) {
        if (i > 0) s += ",";
        s += "{\"day\":" + json::num(trend[i].day) + ",\"health\":" + json::num(trend[i].health, 1) +
             ",\"plants\":" + json::whole(trend[i].plants) + ",\"herbivores\":" + json::whole(trend[i].herbivores) +
             ",\"carnivores\":" + json::whole(trend[i].carnivores) + ",\"birds\":" + json::whole(trend[i].birds) + "}";
    }
    return json::ok("\"history\":[" + s + "],\"events\":" + m3.markersJson());
}

static string recJson(const OrganismRec& r)
{
    return "{\"id\":" + json::num(r.id) + ",\"name\":" + json::str(r.name) + ",\"typeName\":" +
           json::str(eco_type_name(r.type)) + ",\"population\":" + json::num(r.population) + "}";
}

string EcoLifeEngine::analysisJson() const
{
    OrganismArray arr;
    m1.buildArray(arr);

    OrganismRec byPop[MAX_ORGANISMS], byName[MAX_ORGANISMS];
    for (int i = 0; i < arr.count; i++) { byPop[i] = arr.items[i]; byName[i] = arr.items[i]; }
    sort_by_population(byPop, arr.count);
    sort_by_name(byName, arr.count);

    AnalysisStats st;
    analysis_stats(arr.items, arr.count, &st);

    string a, b;
    for (int i = 0; i < arr.count; i++) {
        if (i > 0) { a += ","; b += ","; }
        a += recJson(byPop[i]);
        b += recJson(byName[i]);
    }

    HashTable ht;
    ht_init(&ht);
    for (int i = 0; i < arr.count; i++) ht_insert(&ht, &arr.items[i]);
    string buckets;
    for (int k = 0; k < HASH_SIZE; k++) {
        string names;
        for (HashNode* n = ht.buckets[k]; n != NULL; n = n->next) {
            if (!names.empty()) names += ",";
            names += json::str(n->rec.name);
        }
        if (k > 0) buckets += ",";
        buckets += "{\"bucket\":" + json::num(k) + ",\"names\":[" + names + "]}";
    }
    ht_free(&ht);

    string stats = "{\"total\":" + json::num(st.total) + ",\"average\":" + json::num(st.average, 1) +
                        ",\"median\":" + json::num(st.median, 1);
    if (st.maxIndex >= 0)
        stats += ",\"largest\":" + json::str(arr.items[st.maxIndex].name) +
                 ",\"smallest\":" + json::str(arr.items[st.minIndex].name);
    stats += "}";

    return json::ok("\"byPopulation\":[" + a + "],\"byName\":[" + b + "],\"stats\":" + stats +
                    ",\"hashSize\":" + json::num(HASH_SIZE) + ",\"buckets\":[" + buckets + "]");
}

string EcoLifeEngine::analysisSearchJson(const string& name) const
{
    if (name.empty()) return json::error("Enter a name to search");
    OrganismArray arr;
    m1.buildArray(arr);

    HashTable ht;
    ht_init(&ht);
    for (int i = 0; i < arr.count; i++) ht_insert(&ht, &arr.items[i]);
    int bucket, hashSteps;
    HashNode* hit = ht_search(&ht, name.c_str(), &bucket, &hashSteps);
    string hashPart = "{\"bucket\":" + json::num(bucket) + ",\"comparisons\":" + json::num(hashSteps) +
                           ",\"found\":" + json::boolean(hit != NULL);
    if (hit != NULL) hashPart += ",\"organism\":" + recJson(hit->rec);
    hashPart += "}";
    ht_free(&ht);

    OrganismRec sorted[MAX_ORGANISMS];
    for (int i = 0; i < arr.count; i++) sorted[i] = arr.items[i];
    sort_by_name(sorted, arr.count);
    int steps;
    int idx = binary_search_by_name(sorted, arr.count, name.c_str(), &steps);
    string binPart = "{\"found\":" + json::boolean(idx >= 0) + ",\"comparisons\":" + json::num(steps) +
                          ",\"index\":" + json::num(idx) + "}";

    int lin = arr_search_by_name(&arr, name.c_str());
    string linPart = "{\"found\":" + json::boolean(lin >= 0) + ",\"comparisons\":" +
                          json::num(lin >= 0 ? lin + 1 : arr.count) + "}";

    return json::ok("\"name\":" + json::str(name) + ",\"hash\":" + hashPart + ",\"binary\":" + binPart +
                    ",\"linear\":" + linPart + ",\"items\":" + json::num(arr.count));
}

string EcoLifeEngine::compareJson(int days)
{
    static const char* keys[4]   = { "normal", "drought", "fire", "predator" };
    static const char* labels[4] = { "Normal ecosystem", "Drought", "Forest fire", "Predator removal" };
    string out;

    for (int k = 0; k < 4; k++) {
        EcoLifeEngine tmp;
        tmp.applyScenario(keys[k]);
        tmp.advance(days);

        int alive = 0;
        for (size_t i = 0; i < tmp.eco.getOrganisms().size(); i++)
            if (tmp.eco.getOrganisms()[i]->getPopulation() >= 1) alive++;

        string series;
        for (size_t i = 0; i < tmp.trend.size(); i++) {
            if (i > 0) series += ",";
            series += "{\"day\":" + json::num(tmp.trend[i].day) + ",\"health\":" + json::num(tmp.trend[i].health, 1) + "}";
        }
        if (k > 0) out += ",";
        out += "{\"key\":" + json::str(keys[k]) + ",\"label\":" + json::str(labels[k]) +
               ",\"health\":" + json::num(tmp.eco.calculateHealth(), 1) +
               ",\"healthLabel\":" + json::str(tmp.eco.healthLabel()) +
               ",\"totalOrganisms\":" + json::whole(tmp.eco.totalPopulation()) +
               ",\"plants\":" + json::whole(tmp.eco.totalPopulationOfType(PLANT)) +
               ",\"herbivores\":" + json::whole(tmp.eco.totalPopulationOfType(HERBIVORE)) +
               ",\"carnivores\":" + json::whole(tmp.eco.totalPopulationOfType(CARNIVORE)) +
               ",\"birds\":" + json::whole(tmp.eco.totalPopulationOfType(BIRD)) +
               ",\"speciesAlive\":" + json::num(alive) +
               ",\"series\":[" + series + "]}";
    }
    return json::ok("\"days\":" + json::num(days) + ",\"scenarios\":[" + out + "]");
}

bool EcoLifeEngine::handleSimulation(const vector<string>& a, string& result)
{
    const string& cmd = a[0];
    int days = 0;

    if (cmd == "state") {
        result = json::ok("\"state\":" + stateJson());
    }
    else if (cmd == "reset") {
        reset();
        result = json::ok("\"message\":\"Simulation started at day 0\",\"state\":" + stateJson());
    }
    else if (cmd == "step") {
        if (!toInt(arg(a, 1), days) || days < 1 || days > 365) result = json::error("Days must be a number from 1 to 365");
        else {
            int before = m3.historyCount();
            int events = advance(days);
            result = json::ok("\"daysAdvanced\":" + json::num(days) + ",\"eventsProcessed\":" + json::num(events) +
                              ",\"events\":" + m3.eventsSinceJson(before) + ",\"state\":" + stateJson());
        }
    }
    else if (cmd == "scenario") {
        if (!applyScenario(arg(a, 1))) result = json::error("Unknown scenario (use normal, drought, fire or predator)");
        else result = json::ok("\"message\":" + json::str("Scenario started: " + arg(a, 1)) + ",\"state\":" + stateJson());
    }
    else if (cmd == "random_events") {
        if (arg(a, 1) != "on" && arg(a, 1) != "off") result = json::error("Use random_events|on or random_events|off");
        else {
            setRandomEvents(arg(a, 1) == "on");
            result = json::ok("\"message\":" + json::str(randomEvents ? "Random events are ON" : "Random events are OFF") +
                              ",\"state\":" + stateJson());
        }
    }
    else if (cmd == "history")         result = historyJson();
    else if (cmd == "analysis")        result = analysisJson();
    else if (cmd == "analysis_search") result = analysisSearchJson(arg(a, 1));
    else if (cmd == "compare") {
        if (!toInt(arg(a, 1), days) || days < 1 || days > 200) result = json::error("Days must be a number from 1 to 200");
        else result = compareJson(days);
    }
    else return false;
    return true;
}

bool EcoLifeEngine::handlePopulation(const vector<string>& a, string& result)
{
    const string& cmd = a[0];
    int id = 0;

    if      (cmd == "organisms")   result = m1.organismsJson();
    else if (cmd == "pop_stats")   result = m1.statsJson();
    else if (cmd == "search_id")   result = toInt(arg(a, 1), id) ? m1.searchByIdJson(id) : json::error("ID must be a number");
    else if (cmd == "search_name") result = m1.searchByNameJson(arg(a, 1));
    else return false;
    return true;
}

bool EcoLifeEngine::handleResources(const vector<string>& a, string& result)
{
    const string& cmd = a[0];
    int n = 0;
    double population = 0;

    if (cmd == "list")        result = m2.listJson();
    else if (cmd == "list_search") result = toInt(arg(a, 1), n) ? m2.searchJson(n) : json::error("ID must be a number");
    else if (cmd == "add_organism") {
        if (!toInt(arg(a, 2), n) || !toDouble(arg(a, 3), population)) result = json::error("Type and population must be numbers");
        else result = m2.insert(arg(a, 1), n, population);
    }
    else if (cmd == "delete_organism") result = toInt(arg(a, 1), n) ? m2.remove(n) : json::error("ID must be a number");
    else if (cmd == "update_population") {
        if (!toInt(arg(a, 1), n) || !toDouble(arg(a, 2), population)) result = json::error("ID and population must be numbers");
        else result = m2.updatePopulation(n, population);
    }
    else if (cmd == "resources") result = m2.resourcesJson();
    else return false;
    return true;
}

bool EcoLifeEngine::handleEvents(const vector<string>& a, string& result)
{
    const string& cmd = a[0];
    int type = 0;

    if      (cmd == "events")        result = m3.eventsJson();
    else if (cmd == "add_event")     result = toInt(arg(a, 1), type) ? m3.addJson(type, arg(a, 2)) : json::error("Event type must be a number");
    else if (cmd == "process_event") result = m3.processNextJson(arg(a, 1));
    else if (cmd == "process_all")   result = m3.processAllJson();
    else return false;
    return true;
}

bool EcoLifeEngine::handleFoodWeb(const vector<string>& a, string& result)
{
    const string& cmd = a[0];
    int first = 0, second = 0;

    if (cmd == "foodweb") result = m4.foodwebJson();
    else if (cmd == "fw_add") {
        if (!toInt(arg(a, 1), first) || !toInt(arg(a, 2), second)) result = json::error("IDs must be numbers");
        else result = m4.addJson(first, second);
    }
    else if (cmd == "fw_remove") {
        if (!toInt(arg(a, 1), first) || !toInt(arg(a, 2), second)) result = json::error("IDs must be numbers");
        else result = m4.removeJson(first, second);
    }
    else if (cmd == "fw_dfs") result = toInt(arg(a, 1), first) ? m4.traversalJson("dfs", first) : json::error("Start ID must be a number");
    else if (cmd == "fw_bfs") result = toInt(arg(a, 1), first) ? m4.traversalJson("bfs", first) : json::error("Start ID must be a number");
    else if (cmd == "fw_components") result = m4.componentsJson();
    else return false;
    return true;
}

string EcoLifeEngine::handle(const vector<string>& a)
{
    if (a.empty()) return json::error("Empty command");

    m2.syncToObjects();
    eco.refreshFood();

    string result;
    bool known = handleSimulation(a, result) || handlePopulation(a, result) || handleResources(a, result) ||
                 handleEvents(a, result)     || handleFoodWeb(a, result);
    if (!known) result = json::error("Unknown command: " + a[0]);

    m2.syncToList();
    return result;
}
