#include "backgammon.hpp"
#include "../utils/db_utils.hpp"



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

void Backgammon::handleMove(const std::string& playerId, const crow::json::rvalue& payload) {
    if (players.empty() || playerId != players[turnIdx]) return; // Not your turn

    if (!payload.has("from") || !payload.has("to")) {
        CROW_LOG_ERROR << "payload do not has to or from values" << payload;
        return;
    }
    int from = payload["from"].i();
    int to = payload["to"].i();

    // Check bounds
    if (from < 0 || from >= 24 || to < -1 || to > 24) return;

    // Map player to color and direction
    std::string color = (turnIdx == 0) ? "black" : "white";
    int dir = (color == "black") ? 1 : -1;

    // Check if move is in the right direction
    if ((color == "black" && to <= from) || (color == "white" && to >= from)) return;

    // Check if there is a checker to move
    if (board[from].first != color || board[from].second == 0) return;

    // Calculate distance (move length)
    int moveLen = std::abs(to - from);

    // Check if move matches available dice
    int die = -1;
    if (moveLen == cubes.first && std::find(usedDice.begin(), usedDice.end(), cubes.first) == usedDice.end()) {
        die = cubes.first;
    } else if (moveLen == cubes.second && std::find(usedDice.begin(), usedDice.end(), cubes.second) == usedDice.end()) {
        die = cubes.second;
    } else {
        return; // No matching unused die
    }

    // Check destination
    if (to >= 0 && to < 24) {
        // Allow move if empty or has own checkers or only one opponent checker (hit)
        if (board[to].first.empty() || board[to].first == color || board[to].second == 1) {
            // If hitting, remove opponent checker
            if (board[to].first != color && board[to].second == 1 && !board[to].first.empty()) {
                // Optionally handle bar/eaten pieces here
                board[to].first = color;
                // You may want to increment eatenPieces for the opponent
            }
            // Move checker
            board[from].second--;
            if (board[from].second == 0) board[from].first.clear();
            if (board[to].first.empty()) board[to].first = color;
            board[to].second++;
        } else {
            return; // Blocked
        }
    } else if ((color == "black" && to == 24) || (color == "white" && to == -1)) {
        // Bearing off
        board[from].second--;
        if (board[from].second == 0) board[from].first.clear();
        if (color == "black") remainingPieces.first--;
        else remainingPieces.second--;
    } else {
        return; // Invalid destination
    }

    usedDice.push_back(die);

    // If both dice used, switch turn and roll dice
    if (usedDice.size() >= 2) {
        usedDice.clear();
        turnIdx = 1 - turnIdx;
        startTurn();
    }
}

    
crow::json::wvalue Backgammon::getState() const {
    crow::json::wvalue state;
    std::vector<crow::json::wvalue> boardJson;
    for (const auto& point : board) {
        crow::json::wvalue pt;
        pt["color"] = point.first;
        pt["count"] = point.second;
        boardJson.push_back(std::move(pt));
    }
    state["board"] = std::move(boardJson);
    state["players"] = players;
    if (!players.empty() && turnIdx < players.size()){
        state["currentPlayer"] = players[turnIdx];
    } else{
        state["currentPlayer"] = nullptr;
    }
    
    std::vector<int> diceVec = {cubes.first, cubes.second};
    state["dice"] = diceVec;
    if (isFinished() && !players.empty() && turnIdx < players.size()){
    state["winner"] = isFinished() ? players[turnIdx] : nullptr;
    } else{
        state["winner"] = nullptr;
    }
    state["finished"] = isFinished();
    state["waiting"] = players.size() < 2; // or other waiting logic
    return state;
}

bool Backgammon::isFinished() const {
    if (remainingPieces.first == 0 || remainingPieces.second == 0 ){
        return true;
    }
    return false;

}

void Backgammon::startTurn() {
    static Dice dice;
    int die1 = dice.roll();
    int die2 = dice.roll();
    cubes = {die1, die2};    
}
// In backgammon.cpp
std::string Backgammon::serialize() const {
    crow::json::wvalue json;
    
    // Serialize board
    crow::json::wvalue::list board_json;
    for (const auto& [color, count] : board) {
        crow::json::wvalue point;
        point["color"] = color;
        point["count"] = count;
        board_json.push_back(std::move(point));
    }
    json["board"] = std::move(board_json);
    
    // Other fields
    json["usedDice"] = usedDice;
    json["eatenWhite"] = eatenPieces.first;
    json["eatenBlack"] = eatenPieces.second;
    json["remainingWhite"] = remainingPieces.first;
    json["remainingBlack"] = remainingPieces.second;
    json["cubeWhite"] = cubes.first;
    json["cubeBlack"] = cubes.second;
    json["turnIdx"] = turnIdx;
    json["players"] = join_players(players);
    json["lobbyId"] = lobbyId;
    
    return json.dump();
}

void Backgammon::deserialize(const std::string& data) {
    auto json = crow::json::load(data);
    
    // Board
    board.clear();
    const auto& json_board = json["board"];
    for (size_t i = 0; i < 24; i++) {
        std::string color = json_board[i]["color"].s();
        int count = json_board[i]["count"].i();
        board.emplace_back(color, count);
    }
    
    // Other fields
    usedDice = {static_cast<int>(json["usedDice"][0].i()), static_cast<int>(json["usedDice"][1].i())};
    eatenPieces = {json["eatenWhite"].i(), json["eatenBlack"].i()};
    remainingPieces = {json["remainingWhite"].i(), json["remainingBlack"].i()};
    cubes = {json["cubeWhite"].i(), json["cubeBlack"].i()};
    turnIdx = json["turnIdx"].i();
    players = split_players(json["players"].s());
    lobbyId = json["lobbyId"].i();
}
