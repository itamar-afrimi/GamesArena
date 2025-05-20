#include "memory"
#include "game.hpp"
#include "tic_tac_toe.cpp"

class GameManager{
    public:
        std::unique_ptr<Game> create_game(const std::string& game_type){
            std::unique_ptr<Game> game = nullptr;
            if (game_type == "Tic Tac Toe") {
                game = std::make_unique<TicTacToe>();
            } else {
                // Handle other game types or throw an error
                throw std::invalid_argument("Unknown game type");
            }
            return game;
         
            
        }
        
        
};

