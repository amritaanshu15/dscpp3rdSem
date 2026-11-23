# =====================================================================
#  Rabbit Run - Makefile
#  Windows (MinGW):  mingw32-make        Linux / Mac:  make
#
#  make        -> builds build/rabbit_run(.exe)
#  make clean  -> deletes compiled files
#
#  .c   files -> gcc   (data structures)
#  .cpp files -> g++   (OOP classes + server)
#  g++ links everything into one program
# =====================================================================

CC       = gcc
CXX      = g++
CFLAGS   = -Wall -std=c99
CXXFLAGS = -Wall -std=c++11

ifeq ($(OS),Windows_NT)
  EXE    = .exe
  LIBS   = -lws2_32
  CLEAN  = del /Q build\*.o build\*.exe
else
  EXE    =
  LIBS   =
  CLEAN  = rm -f build/*.o build/rabbit_run
endif

C_OBJS   = build/grid.o build/queue.o build/stack.o build/pathfind.o \
           build/event_heap.o build/log_list.o build/score_bst.o build/sort.o

CPP_OBJS = build/Animal.o build/Event.o build/World.o build/ScoreBoard.o \
           build/server.o build/main.o

all: build/rabbit_run$(EXE)

build/rabbit_run$(EXE): $(C_OBJS) $(CPP_OBJS)
	$(CXX) $(C_OBJS) $(CPP_OBJS) -o $@ $(LIBS)

# C files
build/%.o: c_ds/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# C++ files
build/%.o: cpp_oop/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

build/%.o: server/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(CLEAN)
