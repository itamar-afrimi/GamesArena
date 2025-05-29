#pragma once
#include "game.hpp"
#include <crow/json.h>
#include <vector>
#include <string>

class TicTacToe : public Game {
private:
    std::vector<std::vector<std::string>> board;
    int turnIdx;
    std::vector<std::string> players;
    std::string lobbyId;
    std::string checkWinner() const;
    std::string getWinningMark() const;

public:
    TicTacToe() = default;
    ~TicTacToe() override = default;
    
    void init(const std::string& lobbyId,
             const std::vector<std::string>& playerIds) override;
    
    void handleMove(const std::string& playerId, 
                   const crow::json::rvalue& payload) override;
    
    crow::json::wvalue getState() const override;
    bool isFinished() const override;
    std::string serialize() const override;
    void deserialize(const std::string& data) ;
    
};
