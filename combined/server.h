#ifndef ECOLIFE_SERVER_H
#define ECOLIFE_SERVER_H

#include <string>
#include "module5/EcoLifeEngine.h"

int runServer(EcoLifeEngine& engine, int port, const std::string& frontendDir);

#endif
