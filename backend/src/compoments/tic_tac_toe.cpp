#include "tic_tac_toe.hpp"



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
std::string TicTacToe::get_winning_mark() const {
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

std::string TicTacToe::check_winner() const {
    std::string mark = get_winning_mark();
    if (mark == "X") return players[0];
    if (mark == "O") return players[1];
    return "";
}


crow::json::wvalue TicTacToe::getState() const {
    crow::json::wvalue state;
    state["board"] = board;
    state["currentPlayer"] = (players.size() > turnIdx) ? players[turnIdx] : "";
    state["winner"] = check_winner();
    return state;
}

bool TicTacToe::isFinished() const {
    if (get_winning_mark() != "")
        return true;

    // Check for a draw (no empty cells)
    for (const auto& row : board)
        for (const auto& cell : row)
            if (cell == "")
                return false;
    return true; // Draw
}


