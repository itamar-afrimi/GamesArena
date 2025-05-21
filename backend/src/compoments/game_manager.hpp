#pragma once
#include <memory>
#include <string>
#include "game.hpp"

class GameManager {
public:
    std::unique_ptr<Game> create_game(const std::string& game_type);
};
