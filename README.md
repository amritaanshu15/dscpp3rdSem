# 🌿 EcoLife – Ecosystem Simulation & Analysis System

College PBL project that shows **Data Structures & Algorithms in C** and **Object-Oriented Programming in C++** working together in one application.

| Layer | Technology | Role |
|---|---|---|
| Frontend | HTML + CSS + plain JavaScript | login, dashboard, 5 module pages, charts |
| Web server | C++ (`combined/server.cpp`, built into the engine) | serves the pages, REST API, login token |
| **C modules** | C (`gcc`) | **Data Structures & Algorithms** |
| **C++ modules** | C++ (`g++`) | **Object-Oriented Programming** |
| Combined modules | C++ calling the C code | **Integrated application** + the engine program |

> ⚠️ EcoLife is an **educational simulation**. It uses simple predefined rules; random Heavy Rain / Drought events happen only if you switch them on. It does **not** predict real ecosystems.

---

## 1. Login

* Open the website → enter **any username** and the password **`123@geu`**.
* The password is fixed in `combined/server.cpp` (variable `PASSWORD`).

## 2. What you need on Windows

| Tool | Why | Check with |
|---|---|---|
| **MinGW-w64** (gcc, g++, mingw32-make) – e.g. via [MSYS2](https://www.msys2.org) or [WinLibs](https://winlibs.com) | compile C and C++ | `gcc --version` , `g++ --version` , `mingw32-make --version` |

Make sure the MinGW `bin` folder is in your `PATH`.

## 3. Build (no build.bat)

Open **PowerShell** in the `EcoLife` folder.

### Option A – with make (recommended)

```powershell
mingw32-make            # builds build\ecolife_engine.exe
mingw32-make demos      # (optional) builds one small demo program per C / C++ module
mingw32-make clean      # (optional) removes the compiled files
```

### Option B – plain compiler commands (if you do not have make)

```powershell
mkdir build
gcc -std=c99 -c c_modules/module1_population/organism_array.c -o build/organism_array.o
gcc -std=c99 -c c_modules/module2_linkedlist/pop_list.c       -o build/pop_list.o
gcc -std=c99 -c c_modules/module3_events/event_queue.c        -o build/event_queue.o
gcc -std=c99 -c c_modules/module4_graph/graph.c               -o build/graph.o
gcc -std=c99 -c c_modules/module5_analysis/analysis.c         -o build/analysis.o

g++ -std=c++11 -o build/ecolife_engine.exe build/organism_array.o build/pop_list.o build/event_queue.o build/graph.o build/analysis.o cpp_modules/module1_organisms/Organism.cpp cpp_modules/module2_resources/Resource.cpp cpp_modules/module3_events/Event.cpp cpp_modules/module4_foodweb/FoodWeb.cpp cpp_modules/module5_ecosystem/Ecosystem.cpp combined/module1/PopulationCombined.cpp combined/module2/ResourceCombined.cpp combined/module3/EventCombined.cpp combined/module4/FoodWebCombined.cpp combined/module5/EcoLifeEngine.cpp combined/server.cpp combined/main.cpp -lws2_32
```

The first five lines compile the **C** files with `gcc`; the last line compiles the **C++** files with `g++` and links everything into one program. (On Windows `-lws2_32` is needed for the web server. On Linux/macOS use `-o build/ecolife_engine` without `.exe` and **leave out** `-lws2_32`.)

## 4. Run

```powershell
build\ecolife_engine.exe          # Windows        (Linux / macOS:  ./build/ecolife_engine)
```

Start it from the **EcoLife folder** (it looks for the `frontend` folder next to `build`). Then open
**http://localhost:3000** in your browser. Stop the program with `Ctrl + C`.

Other ways to start it:

| Command | Meaning |
|---|---|
| `build\ecolife_engine.exe 8080` | use port 8080 (if 3000 is busy) |
| `build\ecolife_engine.exe 3000 C:\path\to\frontend` | frontend files are somewhere else |
| `build\ecolife_engine.exe cli` | no web server – type commands in the terminal (`state`, `step|10`, `quit`) |

## 5. Stand-alone demos (to show C and C++ separately)

After `mingw32-make demos`, every component can be run on its own:

| Component | C demo (Data Structure) | C++ demo (OOP) |
|---|---|---|
| Module 1 | `build\demo_module1.exe` – array, linear search | `build\demo_module1b.exe` – inheritance, overriding |
| Module 2 | `build\demo_module2.exe` – linked list | `build\demo_module2b.exe` – resources, composition |
| Module 3 | `build\demo_module3.exe` – FIFO vs priority queue | `build\demo_module3b.exe` – polymorphic events |
| Module 4 | `build\demo_module4.exe` – graph, DFS, BFS | `build\demo_module4b.exe` – food-web classes |
| Module 5 | `build\demo_module5.exe` – sort, search, hash | `build\demo_module5b.exe` – 60-day simulation |

You can also run the engine by hand and type commands: `build\ecolife_engine.exe` then `state`, `step|10`, `organisms` … (`quit` to leave).

## 6. Project structure

```
EcoLife/
├── frontend/                 HTML, CSS, JavaScript (login + dashboard + 5 module pages)
├── c_modules/                C  = DATA STRUCTURES
│   ├── common/                 shared record + helpers
│   ├── module1_population/     array + linear search
│   ├── module2_linkedlist/     singly linked list
│   ├── module3_events/         queue + priority queue (heap) + history
│   ├── module4_graph/          adjacency-matrix graph, DFS, BFS
│   └── module5_analysis/       sorting, binary search, hash table, statistics
├── cpp_modules/              C++ = OOP
│   ├── module1_organisms/      Organism, Plant, Animal, Herbivore, Carnivore, Bird
│   ├── module2_resources/      Resource, Food, Water, Shelter, Environment
│   ├── module3_events/         Weather, Season, Event, Drought, ForestFire, PredatorRemoval, ...
│   ├── module4_foodweb/        FoodWeb, FeedingRelationship, Predator, Prey
│   └── module5_ecosystem/      Ecosystem (coordinates the simulation)
├── combined/                 C + C++ INTEGRATION
│   ├── module1/ … module4/     one combined class per module
│   ├── module5/                EcoLifeEngine (owns everything, command dispatcher)
│   ├── common/json.h           tiny JSON writer
│   ├── server.h / server.cpp   built-in web server (pages + REST API + login)
│   └── main.cpp                program entry: starts the server (or 'cli' mode)
├── tests/test_engine.py      optional automated check of the engine
├── documentation/            architecture, modules, data structures, OOP, viva Q&A, API, evaluation
├── Makefile                  build instructions (there is NO build.bat)
└── README.md
```

## 7. Architecture in one picture

```
Browser (frontend)  ──HTTP / JSON──►  ecolife_engine  (ONE program, C + C++)
  login → dashboard                   ┌─────────────────────────────────────────────┐
  5 module pages                      │ combined/server.cpp   web server + API      │
                                      │        │  one command per request           │
                      ◄──JSON──       │        ▼                                    │
                                      │ combined/EcoLifeEngine  (simulation)        │
                                      │   ├─ C   data structures                    │
                                      │   └─ C++ OOP classes                        │
                                      └─────────────────────────────────────────────┘
```

Two parts, one program: the **browser** shows the pages, the **engine program** does everything else.
The program keeps running, so the simulation stays in memory between requests (no database).

Request example: `POST /api/simulation/step {"days":10}` → `server.cpp` turns it into the command `step|10` →
`EcoLifeEngine::handle()` simulates 10 days → returns one JSON line → `server.cpp` sends it back → the page redraws.

## 8. The 5 modules = 15 components

| # | Module | **C – Data Structure** | **C++ – OOP** | **Combined** |
|---|---|---|---|---|
| 1 | Organism & Population | Array + linear search | Organism → Plant / Animal → Herbivore, Carnivore, Bird | Array search returns a record; the matching C++ object adds diet and description |
| 2 | Dynamic Collection & Resources | Linked list (malloc/free) | Resource → Food, Water, Shelter; Environment (composition) | List = master copy of populations; resource rules change them |
| 3 | Events | Queue + priority queue + history | Event → Drought, ForestFire, PredatorRemoval, … (virtual `apply`) | C stores event records, C++ object applies the effect |
| 4 | Food Web | Graph, DFS, BFS | FoodWeb, FeedingRelationship, Predator, Prey | Relationships loaded into the C graph for traversal |
| 5 | Analysis & Simulation | Sorting, binary search, hash table | Ecosystem (coordinator) | Complete simulation + what-if scenarios |

Details, complexities and viva answers are in the **`documentation/`** folder.

## 9. Suggested 5-minute demo

1. **Login** with `123@geu` → **Dashboard**: explain the status table and the health score.
2. **Population**: search "Tiger" (C array search + C++ object description). Insert a new organism "Wolf" (linked-list node + C++ object), then delete it.
3. **Events**: click *Load demo events*, then *Process next* on the FIFO queue and on the priority queue → different order.
4. **Food Web**: run DFS and BFS from Grass and compare the order. Add a relationship.
5. **Simulation**: *Advance 100 days*, choose *Drought*, advance again, then run the **what-if comparison** and discuss why the curves differ.

## 10. Troubleshooting

| Problem | Fix |
|---|---|
| `Engine not found …` | Run `mingw32-make` first; check that `build\ecolife_engine.exe` exists |
| `'mingw32-make' is not recognized` | MinGW `bin` is not in PATH – or use build Option B |
| Browser shows "Cannot reach the server" | Start `build\ecolife_engine.exe` and keep its window open |
| Page jumps back to login after restarting the server | Normal – login tokens are kept in memory only |
| Port 3000 busy | Start with another port: `build\ecolife_engine.exe 3100`, then open http://localhost:3100 |
| "Cannot find frontend/index.html" | Start the program from the EcoLife folder (not from inside `build`), or pass the frontend folder as 2nd argument |
| Linker error about `WSAStartup` / `socket` (Windows) | The Makefile adds `-lws2_32`; with Option B add `-lws2_32` at the end of the g++ line |

## 11. Known limitations (be ready to explain these)

* The rules and constants are **hand-tuned teaching parameters**, not real ecological data.
* A species that reaches zero never comes back (no immigration).
* The maximum is 20 organisms (array / graph size in C).
* One shared simulation per server (no separate sessions per user).
* Built and tested on **Linux (GCC 13)** with an automated test, a memory-error check (AddressSanitizer), a side-by-side comparison with the earlier Node.js version, and a headless-browser run. The Windows part of `server.cpp` (Winsock) follows the standard pattern but was **not run on Windows by me** – build and test it once on your own machine.
* The built-in web server is for a classroom demo: one request at a time, listens on this computer only (127.0.0.1), fixed password, no HTTPS.

See `documentation/07_evaluation_and_limitations.md` for the test results and ideas for improvement.
