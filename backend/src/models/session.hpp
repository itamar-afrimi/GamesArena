#pragma once
#include <string>
#include <vector>
#include <memory>
#include <crow/websocket.h>
#include "../compoments/game.hpp"

struct Session {
    std::string gameType;
    std::vector<std::string> players;

    std::vector<crow::websocket::connection*> conns;
    bool finished = false;
};
