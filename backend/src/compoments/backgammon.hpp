#include "game.hpp"
#include "../utils/dice.hpp"
#include <iostream>
#include <random>
class Backgammon: public Game{
    private:
    std::vector<std::pair<std::string,int>> board;
    
    std::pair<int,int> eaten_pieces;
    std::pair<int,int> remaining_pieces;
    std::pair<int,int> cubes;
    int turnIdx;
    std::vector<std::string> players;
    std::string lobbyId;
    void init_board();
    void generate_cubes();

public:
    Backgammon() = default;
    ~Backgammon() override = default;
    
    void init(const std::string& lobbyId,
             const std::vector<std::string>& playerIds) override;
    
    void handleMove(const std::string& playerId, 
                   const crow::json::rvalue& payload) override;
    
    crow::json::wvalue getState() const override;
    bool isFinished() const override;
};