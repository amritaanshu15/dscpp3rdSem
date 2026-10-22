# ============================================================================
#  EcoLife Makefile  (NOT a build.bat)
#
#  Windows (MinGW):   mingw32-make            Linux / macOS:   make
#
#    make          -> builds the engine  build/ecolife_engine(.exe)
#    make demos    -> builds one small demo program for every C and C++ module
#    make clean    -> deletes everything inside the build folder
#
#  C   files are compiled with gcc   (c_modules/...)
#  C++ files are compiled with g++   (cpp_modules/..., combined/...)
#  g++ links everything together into one program.
# ============================================================================

CC       = gcc
CXX      = g++
CFLAGS   = -Wall -Wextra -std=c99   -O2 -MMD -MP
CXXFLAGS = -Wall -Wextra -std=c++11 -O2 -MMD -MP

ifeq ($(OS),Windows_NT)
  EXE = .exe
  LDLIBS = -lws2_32                # Windows sockets (needed by the built-in web server)
  CLEAN = del /Q build\*.o build\*.d build\*.exe
else
  EXE =
  LDLIBS =
  CLEAN = rm -f build/*.o build/*.d build/ecolife_engine build/demo_*
endif

# where make should look for source files
vpath %.c   c_modules/module1_population c_modules/module2_linkedlist c_modules/module3_events \
            c_modules/module4_graph c_modules/module5_analysis
vpath %.cpp cpp_modules/module1_organisms cpp_modules/module2_resources cpp_modules/module3_events \
            cpp_modules/module4_foodweb cpp_modules/module5_ecosystem \
            combined combined/module1 combined/module2 combined/module3 combined/module4 combined/module5

# ---- object files -----------------------------------------------------------
C_OBJS   = build/organism_array.o build/pop_list.o build/event_queue.o build/graph.o build/analysis.o
CPP_OBJS = build/Organism.o build/Resource.o build/Event.o build/FoodWeb.o build/Ecosystem.o
COMB_OBJS = build/PopulationCombined.o build/ResourceCombined.o build/EventCombined.o \
            build/FoodWebCombined.o build/EcoLifeEngine.o build/server.o build/main.o

ENGINE = build/ecolife_engine$(EXE)

all: $(ENGINE)

$(ENGINE): $(C_OBJS) $(CPP_OBJS) $(COMB_OBJS)
	$(CXX) $^ -o $@ $(LDLIBS)

# ---- generic rules ----------------------------------------------------------
build:
	mkdir build

build/%.o: %.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: %.cpp | build
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ---- stand-alone demo programs (one per component) ---------------------------
DEMOS = build/demo_module1$(EXE) build/demo_module2$(EXE) build/demo_module3$(EXE) \
        build/demo_module4$(EXE) build/demo_module5$(EXE) \
        build/demo_module1b$(EXE) build/demo_module2b$(EXE) build/demo_module3b$(EXE) \
        build/demo_module4b$(EXE) build/demo_module5b$(EXE)

demos: $(DEMOS)

# C demos (pure C, linked with gcc)
build/demo_module1$(EXE): build/demo_module1.o build/organism_array.o
	$(CC) $^ -o $@
build/demo_module2$(EXE): build/demo_module2.o build/pop_list.o
	$(CC) $^ -o $@
build/demo_module3$(EXE): build/demo_module3.o build/event_queue.o
	$(CC) $^ -o $@
build/demo_module4$(EXE): build/demo_module4.o build/graph.o
	$(CC) $^ -o $@
build/demo_module5$(EXE): build/demo_module5.o build/analysis.o
	$(CC) $^ -o $@

# C++ demos (pure C++, linked with g++)
build/demo_module1b$(EXE): build/demo_module1b.o build/Organism.o
	$(CXX) $^ -o $@
build/demo_module2b$(EXE): build/demo_module2b.o build/Resource.o
	$(CXX) $^ -o $@
build/demo_module3b$(EXE): build/demo_module3b.o build/Event.o build/Resource.o
	$(CXX) $^ -o $@
build/demo_module4b$(EXE): build/demo_module4b.o build/FoodWeb.o
	$(CXX) $^ -o $@
build/demo_module5b$(EXE): build/demo_module5b.o build/Ecosystem.o build/FoodWeb.o build/Event.o build/Resource.o build/Organism.o
	$(CXX) $^ -o $@

clean:
	-$(CLEAN)

.PHONY: all demos clean
-include $(wildcard build/*.d)
