#include "backgammon.hpp"
void Backgammon::init(const std::string& lobbyId,
             const std::vector<std::string>& playerIds){
                init_board();   
                this->remaining_pieces = std::pair<int,int>(24,24);
                this->turnIdx = 0;
                this->players = playerIds;
                this->lobbyId = lobbyId;
             }
void Backgammon::init_board(){
    this->board.resize(24);
    this->board[0] = {"black",2};
    this->board[5] = {"white",5};
    this->board[7] = {"white",3};
    this->board[11] = {"black",5};
    this->board[12] = {"white",5};
    this->board[16] = {"black",3};
    this->board[18] = {"black",5};
    this->board[23] = {"white",2};
    for (size_t i = 0; i < board.size(); i++)
    {
        std::cout<<board[i].first << board[i].second;
    }
    
    
}
void Backgammon::handleMove(const std::string& playerId, 
                   const crow::json::rvalue& payload){
                        return;
                   }
    
crow::json::wvalue Backgammon::getState() const{return nullptr;}
bool Backgammon::isFinished() const {return true;}