#include "game_manager.hpp"
#include "tic_tac_toe.hpp"
#include <stdexcept>

std::unique_ptr<Game> GameManager::create_game(const std::string& game_type) {
    std::unique_ptr<Game> game = nullptr;
    if (game_type == "Tic Tac Toe") {
        game = std::make_unique<TicTacToe>();
    } else {
        // Handle other game types or throw an error
        throw std::invalid_argument("Unknown game type");
    }
    return game;
}
