#include "backgammon.hpp"




void Backgammon::init(const std::string& lobbyId,
             const std::vector<std::string>& playerIds){
                initBoard();   
                this->remainingPieces = std::pair<int,int>(24,24);
                this->turnIdx = 0;
                this->players = playerIds;
                this->lobbyId = lobbyId;
             }
void Backgammon::initBoard(){
    this->board.resize(24);
    this->board[0] = {"black",2};
    this->board[5] = {"white",5};
    this->board[7] = {"white",3};
    this->board[11] = {"black",5};
    this->board[12] = {"white",5};
    this->board[16] = {"black",3};
    this->board[18] = {"black",5};
    this->board[23] = {"white",2};
}

void Backgammon::handleMove(const std::string& playerId, 
                   const crow::json::rvalue& payload){
                        return;
                   }
    
crow::json::wvalue Backgammon::getState() const{return nullptr;}
bool Backgammon::isFinished() const {return true;}

void Backgammon::startTurn() {
    static Dice dice;
    int die1 = dice.roll();
    int die2 = dice.roll();
    cubes = {die1, die2};    
}
