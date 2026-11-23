/*
 * server.h  -  tiny web server (adapted from our EcoLife combined/server.cpp)
 * The browser asks for pages and /api/... addresses, this program answers.
 */
#ifndef SERVER_H
#define SERVER_H

#include <string>
#include "../cpp_oop/World.h"
#include "../cpp_oop/ScoreBoard.h"

int runServer(World& world, ScoreBoard& board, int port, std::string webDir);

#endif
