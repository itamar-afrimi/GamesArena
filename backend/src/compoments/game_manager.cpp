#include "memory"
#include "game.hpp"
#include "tic_tac_toe.cpp"

class GameManager{
    public:
        std::unique_ptr<Game> create_game(const std::string& game_type){
            std::unique_ptr<Game> game = nullptr;
            switch (game_type)
            {
            case "Tic Tac Toe":
                game = std::make_unique<TicTacToe>();
                break;
            
            default:
                break;
            }
            return game;
        }
        
        
};

