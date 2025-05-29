#include "game.hpp"
#include "../utils/dice.hpp"
#include <iostream>
#include <random>
class Backgammon: public Game{
    private:
    std::vector<std::pair<std::string,int>> board;
    std::vector<int> usedDice; // Add this to your Backgammon class
    std::pair<int,int> eatenPieces;
    std::pair<int,int> remainingPieces;
    std::pair<int,int> cubes;
    int turnIdx;
    std::vector<std::string> players;
    std::string lobbyId;
    void initBoard();


public:
    Backgammon() = default;
    ~Backgammon() override = default;
    
    void init(const std::string& lobbyId,
             const std::vector<std::string>& playerIds) override;
    
    void handleMove(const std::string& playerId, 
                   const crow::json::rvalue& payload) override;
    
    crow::json::wvalue getState() const override;
    bool isFinished() const override;
    void startTurn() override;
    std::string serialize() const override;
    void deserialize(const std::string& data) override;
};