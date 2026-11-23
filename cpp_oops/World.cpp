/*
 * World.cpp  -  game rules
 *
 * One turn (= one day) after the player presses a key:
 *   1. player moves (eat grass / drink / hide)
 *   2. player gets hungry and thirsty
 *   3. every other animal takes its turn (polymorphism: animals[i]->takeTurn())
 *   4. day + 1
 *   5. grass that was eaten long ago grows back      (circular queue)
 *   6. events planned for today happen               (priority queue / heap)
 *   7. check: starved? thirsty? survived 50 days?
 */
#include <cstdlib>
#include <iostream>
#include <sstream>
#include "World.h"
#include "Event.h"
#include "Utils.h"
#include "../c_ds/sort.h"

// animals in the 3 normal levels (array of structures, index 0 not used)
//                              fox eagle tiger leopard snake croc deer
static LevelSetup presets[4] = { {0, 0,    0,    0,      0,    0,   0},    // (not used)
                                 {1, 1,    0,    0,      0,    0,   3},    // 1 easy
                                 {2, 1,    0,    0,      1,    1,   2},    // 2 normal
                                 {1, 1,    1,    1,      1,    1,   2} };  // 3 hard

World::World()
{
    player = NULL;
    level = 1;
    hunterCount = 0;
    day = 0;
    nextId = 1;
    droughtLeft = 0;
    started = false;
    over = false;
    won = false;
    saved = false;
    for (int i = 0; i < 8; i++) nameCount[i] = 0;
    grid_fill(&grid, T_GROUND);
    q_init(&regrowQueue);
    heap_init(&events);
    log_init(&log);
}

World::~World()
{
    clearGame();
}

// deletes every dynamic object of the old game
void World::clearGame()
{
    if (player != NULL) {
        delete player;
        player = NULL;
    }
    for (int i = 0; i < (int)animals.size(); i++)
        delete animals[i];              // virtual destructor
    animals.clear();
    log_free(&log);
    q_init(&regrowQueue);
    heap_init(&events);
}

// =====================================================================
// New game
// =====================================================================
void World::newGame(string name, char animal, int lvl, LevelSetup custom)
{
    if (lvl < 1 || lvl > CUSTOM_LEVEL)
        throw GameError("Level must be 1, 2, 3 or 4 (custom)");

    // which animals? normal level -> preset, custom level -> what the user chose
    LevelSetup setup;
    if (lvl == CUSTOM_LEVEL) {
        setup = custom;
        int each[7] = {
            setup.fox, setup.eagle, setup.tiger, setup.leopard,
            setup.snake, setup.croc, setup.deer
        };
        for (int i = 0; i < 7; i++)
            if (each[i] < 0 || each[i] > MAX_EACH)
                throw GameError("Custom level: choose 0 to 3 of each animal");
        if (setup.fox + setup.eagle + setup.tiger + setup.leopard +
            setup.snake + setup.croc == 0)
            throw GameError("Custom level: choose at least one hunter");
    } else {
        setup = presets[lvl];
    }

    // keep only letters and digits in the name (safe for the file and JSON)
    string cleanName = "";
    for (int i = 0; i < (int)name.size() && cleanName.size() < 15; i++) {
        char ch = name[i];
        if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9'))
            cleanName += ch;
        else if (ch == ' ' || ch == '_')
            cleanName += '_';
    }
    if (cleanName == "") cleanName = "Player";

    Pos start(ROWS / 2, COLS / 2);
    Player* newPlayer = makePlayer(animal, start); // throws for an unknown animal letter

    clearGame();
    player = newPlayer;
    playerName = cleanName;
    level = lvl;
    day = 0;
    nextId = 1;
    droughtLeft = 0;
    over = false;
    won = false;
    saved = false;
    endReason = "";
    for (int i = 0; i < 8; i++) nameCount[i] = 0;

    grid_generate(&grid, start.r, start.c);

    // create the animals (hunters far away from the player, deer can be closer)
    for (int i = 0; i < setup.fox; i++)
        addFox();
    for (int i = 0; i < setup.eagle; i++)
        spawn(new Eagle(nextId, nextName("Eagle", EAGLE), start), 8);
    for (int i = 0; i < setup.tiger; i++)
        spawn(new Tiger(nextId, nextName("Tiger", TIGER), start), 8);
    for (int i = 0; i < setup.leopard; i++)
        spawn(new Leopard(nextId, nextName("Leopard", LEOPARD), start), 8);
    for (int i = 0; i < setup.snake; i++)
        spawn(new Snake(nextId, nextName("Snake", SNAKE), start), 8);
    for (int i = 0; i < setup.croc; i++)
        spawn(new Crocodile(nextId, nextName("Croc", CROCODILE), start), 3);
    for (int i = 0; i < setup.deer; i++)
        spawn(new Deer(nextId, nextName("Deer", DEER), start), 3);

    hunterCount = setup.fox + setup.eagle + setup.tiger + setup.leopard +
                  setup.snake + setup.croc;

    // plan the events: random days, the heap keeps them in order of day
    int howMany = 6 + lvl;
    if (lvl == CUSTOM_LEVEL) howMany = 8;
    for (int i = 0; i < howMany; i++) {
        EventItem e;
        e.day = 5 + rand() % (GOAL_DAYS - 8);     // between day 5 and day 46
        e.type = 1 + rand() % 5;                  // EV_DROUGHT .. EV_HUNTER
        heap_push(&events, e);
    }

    started = true;
    addMessage("Welcome " + playerName + "! You are a " + player->getName() +
               ". Survive " + to_string(GOAL_DAYS) + " days.");
}

string World::nextName(string base, int type)
{
    nameCount[type]++;
    return base + " " + to_string(nameCount[type]);
}

// put a new animal on a random cell where it can stand, far from the player
void World::spawn(Animal* a, int minDist)
{
    a->setPos(randomCellFor(a, minDist));
    animals.push_back(a);
    nextId++;
}

Pos World::randomCellFor(Animal* a, int minDist)
{
    Pos candidate;
    // first try far away, if that does not work try closer (but never right next to the player)
    for (int pass = 0; pass < 2; pass++) {
        int need = (pass == 0) ? minDist : 2;
        for (int tries = 0; tries < 1000; tries++) {
            candidate = Pos(rand() % ROWS, rand() % COLS);
            char candidateTile = tileAt(candidate);
            if (!a->canWalkOn(candidateTile)) continue;
            if (candidateTile == T_BUSH) continue;               // keep bushes free for hiding
            if (manhattan(candidate, player->getPos()) < need) continue;
            if (animalAt(candidate) != NULL) continue;
            return candidate;
        }
    }
    return candidate; // almost never happens
}

void World::addFox()
{
    spawn(new Fox(nextId, nextName("Fox", FOX), Pos(0, 0)), 8);
}

bool World::removeOneHunter(string& whoRemoved)
{
    for (int i = 0; i < (int)animals.size(); i++) {
        if (animals[i]->isPredator()) {
            whoRemoved = animals[i]->getName();
            delete animals[i];
            animals.erase(animals.begin() + i);
            return true;
        }
    }
    return false;
}

// =====================================================================
// Player move
// =====================================================================
void World::playerMove(char dir)
{
    if (!started) throw GameError("Start a new game first");
    if (over)     throw GameError("Game over. Start a new game.");

    Pos d;
    switch (dir) {
        case 'U': d = Pos(-1, 0); break;
        case 'D': d = Pos(1, 0);  break;
        case 'L': d = Pos(0, -1); break;
        case 'R': d = Pos(0, 1);  break;
        case 'X': d = Pos(0, 0);  break;     // stay and rest
        default:  throw GameError("Unknown direction. Use U, D, L, R or X");
    }

    if (dir == 'X') {
        if (tileAt(player->getPos()) == T_WATER)   // turtle resting in the pond drinks
            player->drink(isDrought() ? DROUGHT_DRINK : DRINK_WATER);
        endTurn();
        return;
    }

    Pos target = player->getPos() + d;
    if (!inside(target)) {
        addMessage("That is the edge of the forest.");
        return;                                     // no day passes
    }

    char targetTile = tileAt(target);
    Animal* animalAtTarget = animalAt(target);

    // 1. walked into a hunter (and it is not a hiding place)
    if (animalAtTarget != NULL && animalAtTarget->isPredator() &&
        !player->hiddenOn(targetTile)) {
        if (player->canWalkOn(targetTile)) player->setPos(target);
        addMessage("You ran into " + animalAtTarget->getName() + "!");
        playerCaught(animalAtTarget);
        if (!over) endTurn();                       // hedgehog survived
        return;
    }

    // 2. a deer is in the way
    if (animalAtTarget != NULL && !animalAtTarget->isPredator() &&
        !animalAtTarget->flies()) {
        addMessage(animalAtTarget->getName() + " is in the way.");
        return;
    }

    // 3. pond, but this animal can not swim -> drink from the shore
    if (targetTile == T_WATER && !player->canWalkOn(targetTile)) {
        int amount = isDrought() ? DROUGHT_DRINK : DRINK_WATER;
        player->drink(amount);
        addMessage("Slurp! You drank water (+" + to_string(amount) + ").");
        endTurn();
        return;
    }

    // 4. tile not allowed for this animal (rock, or bush for some animals)
    if (!player->canWalkOn(targetTile)) {
        addMessage("A " + player->getName() + " can not go there.");
        return;
    }

    // 5. normal move
    player->setPos(target);

    if (targetTile == T_GRASS) {
        player->eat();                              // uses grassFood() of this animal
        setTile(target, T_GROUND);
        scheduleRegrow(target, T_GRASS);
        addMessage("Yum! You ate grass (+" + to_string(player->grassFood()) + " food).");
    }
    if (targetTile == T_WATER) {                    // turtle swimming
        player->drink(isDrought() ? DROUGHT_DRINK : DRINK_WATER);
    }

    endTurn();
}

void World::endTurn()
{
    player->takeTurn(*this);

    // animals[i] is an Animal* but the Fox / Tiger / Snake ... version runs (late binding)
    for (int i = 0; i < (int)animals.size(); i++) {
        if (over) return;
        animals[i]->takeTurn(*this);
    }
    if (over) return;

    day++;
    processRegrow();
    processEvents();

    if (droughtLeft > 0) {
        droughtLeft--;
        if (droughtLeft == 0)
            addMessage("The drought is over.");
    }

    if (player->getFood() <= 0)
        finishGame(false, "You starved. Eat grass more often!");
    else if (player->getWater() <= 0)
        finishGame(false, "You died of thirst. Visit the ponds!");
    else if (day >= GOAL_DAYS)
        finishGame(true, "You survived " + to_string(GOAL_DAYS) + " days. You win!");
}

// All plants wait the same REGROW_DAYS, so the queue is already in time order
void World::processRegrow()
{
    QItem item;
    while (q_peek(&regrowQueue, &item) && item.day <= day) {
        q_dequeue(&regrowQueue, &item);
        Pos p(item.r, item.c);
        char now = tileAt(p);
        if (now == T_GROUND || now == T_BURNT)
            setTile(p, item.tile);
    }
}

void World::processEvents()
{
    EventItem e;
    while (heap_peek(&events, &e) && e.day <= day) {
        heap_pop(&events, &e);
        Event* event = makeEvent(e.type); // base class pointer
        if (event == NULL) continue;
        string message = event->apply(*this); // child's apply() runs
        addMessage("EVENT - " + event->getName() + ": " + message);
        delete event;
    }
}

void World::finishGame(bool win, string reason)
{
    over = true;
    won = win;
    endReason = reason;
    addMessage((win ? "WIN: " : "GAME OVER: ") + reason + " Score: " + to_string(getScore()));
}

// a hunter reached the player
void World::playerCaught(Animal* hunter)
{
    if (player->loseLife()) {
        // still alive (hedgehog spikes) -> hunter runs away to another place
        addMessage("Ouch! " + hunter->getName() + " got spiked and ran away. Lives left: "
                   + to_string(player->getLives()));
        hunter->setPos(randomCellFor(hunter, 8));
        return;
    }
    finishGame(false, "Caught by " + hunter->getName() + "!");
}

int World::getScore() const
{
    if (player == NULL) return 0;
    int s = day * 10 + player->getGrassEaten() * 5 + player->getDrinks() * 2;
    if (won) s = s + 200;

    int times = level;                       // easy x1, normal x2, hard x3
    if (level == CUSTOM_LEVEL)
        times = 1 + hunterCount / 3;         // custom: more hunters = more points
    return s * times;
}

// =====================================================================
// Helpers used by animals and events
// =====================================================================
bool World::inside(Pos p) const
{
    return grid_inside(p.r, p.c) == 1;
}

char World::tileAt(Pos p) const
{
    return grid_get(&grid, p.r, p.c);
}

void World::setTile(Pos p, char t)
{
    grid_set(&grid, p.r, p.c, t);
}

bool World::playerHidden() const
{
    return player->hiddenOn(tileAt(player->getPos()));   // virtual: depends on the animal
}

// is a walking / swimming animal standing here? eagles do not block
bool World::groundAnimalAt(Pos p, Animal* except) const
{
    for (int i = 0; i < (int)animals.size(); i++) {
        if (animals[i] == except) continue;
        if (animals[i]->flies()) continue;
        if (animals[i]->getPos() == p) return true;
    }
    return false;
}

Animal* World::animalAt(Pos p) const
{
    for (int i = 0; i < (int)animals.size(); i++)
        if (animals[i]->getPos() == p)
            return animals[i];
    return NULL;
}

// 1 = this animal can stand there, 0 = can not (used by BFS)
void World::makeWalkMap(int map[ROWS][COLS], Animal* a) const
{
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            Pos p(r, c);
            map[r][c] = 0;
            if (!a->canWalkOn(tileAt(p))) continue;
            if (!a->flies() && groundAnimalAt(p, a)) continue;
            map[r][c] = 1;
        }
    }
}

void World::addMessage(string msg)
{
    string line = "Day " + to_string(day) + ": " + msg;
    log_add(&log, line.c_str());
}

void World::scheduleRegrow(Pos p, char tile)
{
    QItem item;
    item.r = p.r;
    item.c = p.c;
    item.day = day + REGROW_DAYS;
    item.tile = tile;
    q_enqueue(&regrowQueue, item);     // if the queue is full the plant just does not come back
}

void World::startDrought(int days)
{
    droughtLeft = days;
}

void World::endDrought()
{
    if (droughtLeft > 0)
        addMessage("The rain ended the drought.");
    droughtLeft = 0;
}

// =====================================================================
// Output
// =====================================================================

// puts quotes around a text and escapes " and \ (for JSON)
static string jsonString(string value)
{
    string out = "\"";
    for (int i = 0; i < (int)value.size(); i++) {
        if (value[i] == '"' || value[i] == '\\') out += '\\';
        out += value[i];
    }
    return out + "\"";
}

static string jsonBoolean(bool value)
{
    return value ? "true" : "false";
}

string World::toJson()
{
    if (!started)
        return "{\"ok\":true,\"started\":false}";

    ostringstream js;
    Pos playerPos = player->getPos();

    js << "{\"ok\":true,\"started\":true";
    js << ",\"name\":" << jsonString(playerName) << ",\"level\":" << level;
    js << ",\"day\":" << day << ",\"goal\":" << GOAL_DAYS;
    js << ",\"food\":" << player->getFood() << ",\"water\":" << player->getWater();
    js << ",\"lives\":" << player->getLives();
    js << ",\"grass\":" << player->getGrassEaten() << ",\"drinks\":" << player->getDrinks();
    js << ",\"score\":" << getScore();
    js << ",\"over\":" << jsonBoolean(over) << ",\"won\":" << jsonBoolean(won)
       << ",\"reason\":" << jsonString(endReason);
    js << ",\"drought\":" << droughtLeft << ",\"hidden\":" << jsonBoolean(playerHidden());
    js << ",\"player\":{\"r\":" << playerPos.r << ",\"c\":" << playerPos.c
       << ",\"kind\":\"" << player->letter() << "\""
       << ",\"animal\":" << jsonString(player->getName())
       << ",\"info\":" << jsonString(player->describe()) << "}";
    js << ",\"totalAnimals\":" << Animal::getTotalAnimals();

    // map: one string per row
    js << ",\"map\":[";
    for (int r = 0; r < ROWS; r++) {
        if (r > 0) js << ",";
        string mapRow = "";
        for (int c = 0; c < COLS; c++) mapRow += grid.tile[r][c];
        js << jsonString(mapRow);
    }
    js << "]";

    // animals
    js << ",\"animals\":[";
    for (int i = 0; i < (int)animals.size(); i++) {
        Animal* animal = animals[i];
        if (i > 0) js << ",";
        js << "{\"id\":" << animal->getId() << ",\"type\":\"" << animal->letter() << "\""
           << ",\"name\":" << jsonString(animal->getName())
           << ",\"r\":" << animal->getPos().r << ",\"c\":" << animal->getPos().c
           << ",\"info\":" << jsonString(animal->describe()) << "}";
    }
    js << "]";

    // danger radar: hunters sorted by distance (insertion sort in C)
    RadarItem radar[30];
    int n = 0;
    for (int i = 0; i < (int)animals.size() && n < 30; i++) {
        if (!animals[i]->isPredator()) continue;
        radar[n].id = animals[i]->getId();
        string nm = animals[i]->getName();
        int len = nm.copy(radar[n].name, 19);
        radar[n].name[len] = '\0';
        radar[n].dist = manhattan(animals[i]->getPos(), playerPos);
        n++;
    }
    insertion_sort_radar(radar, n);
    js << ",\"radar\":[";
    for (int i = 0; i < n; i++) {
        if (i > 0) js << ",";
        js << "{\"name\":" << jsonString(radar[i].name)
           << ",\"dist\":" << radar[i].dist << "}";
    }
    js << "]";

    // forecast = the top of the heap (next event)
    EventItem next;
    if (!over && heap_peek(&events, &next))
        js << ",\"forecast\":"
           << jsonString(eventName(next.type) + " on day " + to_string(next.day));
    else
        js << ",\"forecast\":\"\"";

    // log messages (linked list, newest first)
    js << ",\"log\":[";
    LogNode* node = log.head;
    bool first = true;
    while (node != NULL) {
        if (!first) js << ",";
        js << jsonString(node->text);
        first = false;
        node = node->next;
    }
    js << "]}";

    return js.str();
}

void World::printText()
{
    if (!started) {
        cout << "No game yet.\n";
        return;
    }
    // copy the map and draw animals on top
    char view[ROWS][COLS];
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            view[r][c] = grid.tile[r][c];
    for (int i = 0; i < (int)animals.size(); i++)
        view[animals[i]->getPos().r][animals[i]->getPos().c] = animals[i]->letter();
    view[player->getPos().r][player->getPos().c] = player->letter();

    cout << "\n";
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) cout << view[r][c] << ' ';
        cout << "\n";
    }
    cout << player->getName() << "  Day " << day << "/" << GOAL_DAYS
         << "  Food " << player->getFood() << "  Water " << player->getWater()
         << "  Lives " << player->getLives() << "  Score " << getScore()
         << (playerHidden() ? "  [hidden]" : "")
         << (isDrought() ? "  [DROUGHT]" : "") << "\n";

    LogNode* node = log.head;
    int shown = 0;
    while (node != NULL && shown < 4) {
        cout << "  - " << node->text << "\n";
        node = node->next;
        shown++;
    }
}
