#include "tic_tac_toe.hpp"
#include "../utils/db_utils.hpp"
#include <algorithm>



void TicTacToe::init(const std::string& lobbyId,
                  const std::vector<std::string>& playerIds){
    this->lobbyId = lobbyId;
    this->players = playerIds;
    this->turnIdx = 0;
    this->board.resize(3, std::vector<std::string>(3, ""));
}

void TicTacToe::handleMove(const std::string& playerId, const crow::json::rvalue& payload) {
    if (players.size() < 2) return; // Not enough players yet
    if (playerId != players[turnIdx]) return;    // ignore if not your turn
    int row = payload["row"].i();
    int col = payload["col"].i();
    if (board[row][col].empty()) {
        board[row][col] = turnIdx == 0 ? "X" : "O";
        turnIdx = 1 - turnIdx;
    }
}
// Returns "X" if X wins, "O" if O wins, "" otherwise
std::string TicTacToe::getWinningMark() const {
    // Check rows and columns
    for (int i = 0; i < 3; ++i) {
        if (board[i][0] != "" && board[i][0] == board[i][1] && board[i][1] == board[i][2])
            return board[i][0];
        if (board[0][i] != "" && board[0][i] == board[1][i] && board[1][i] == board[2][i])
            return board[0][i];
    }
    // Check diagonals
    if (board[0][0] != "" && board[0][0] == board[1][1] && board[1][1] == board[2][2])
        return board[0][0];
    if (board[0][2] != "" && board[0][2] == board[1][1] && board[1][1] == board[2][0])
        return board[0][2];
    return "";
}

std::string TicTacToe::checkWinner() const {
    std::string mark = getWinningMark();
    if (mark == "X") return players[0];
    if (mark == "O") return players[1];
    return "";
}


crow::json::wvalue TicTacToe::getState() const {
    crow::json::wvalue state;
    state["board"] = board;
    state["currentPlayer"] = (players.size() > turnIdx) ? players[turnIdx] : "";
    state["winner"] = checkWinner();
    return state;
}

bool TicTacToe::isFinished() const {
    if (getWinningMark() != "")
        return true;

    // Check for a draw (no empty cells)
    for (const auto& row : board)
        for (const auto& cell : row)
            if (cell == "")
                return false;
    return true; // Draw
}

// In tic_tac_toe.cpp
std::string TicTacToe::serialize() const {
    crow::json::wvalue json;
    
    // Serialize board
    crow::json::wvalue::list board_json;
    for (const auto& row : board) {
        crow::json::wvalue::list row_json;
        for (const auto& cell : row) {
            row_json.push_back(cell.empty() ? " " : cell);
        }
        json["board"] = std::move(row_json);
    }
    
    // Other fields
    json["turnIdx"] = turnIdx;
    json["players"] = join_players(players);
    json["lobbyId"] = lobbyId;
    
    return json.dump();
}

void TicTacToe::deserialize(const std::string& data){
    auto json = crow::json::load(data);
    
    // Board
    board.clear();
    const auto& json_board = json["board"];
    for (size_t i = 0; i < 3; i++) {
        std::vector<std::string> row;
        for (size_t j = 0; j < 3; j++) {
            std::string val = json_board[i][j].s();
            row.push_back(val == " " ? "" : val);
        }
        board.push_back(row);
    }
    
    // Other fields
    turnIdx = json["turnIdx"].i();
    players = split_players(json["players"].s());
    lobbyId = json["lobbyId"].s();
}


