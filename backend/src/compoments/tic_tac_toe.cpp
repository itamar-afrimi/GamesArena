#include "game.hpp"
class TicTacToe: public Game {
    private:
        std::vector<std::vector<std::string>> board;
        int turnIdx;
        std::vector<std::string> players;
        std::string lobbyId;
    public:
        TicTacToe();
        ~TicTacToe();
        void init(const std::string& lobbyId,
                  const std::vector<std::string>& playerIds) override;
        void handleMove(const std::string&, const crow::json::rvalue&) override;
        crow::json::wvalue getState() const override;
        bool isFinished() const override;
    
};
void TicTacToe::init(const std::string& lobbyId,
                  const std::vector<std::string>& playerIds){
    this->lobbyId = lobbyId;
    this->players = playerIds;
    this->turnIdx = 0;
    this->board.resize(3, std::vector<std::string>(3, ""));
}

void TicTacToe::handleMove(const std::string& playerId, const crow::json::rvalue& payload) {
    if (playerId != players[turnIdx]) return;    // ignore if not your turn
    int row = payload["row"].i();
    int col = payload["col"].i();
    if (board[row][col].empty()) {
        board[row][col] = turnIdx == 0 ? "X" : "O";
        turnIdx = 1 - turnIdx;
    }
}

crow::json::wvalue TicTacToe::getState() const {
    crow::json::wvalue state;
    state["board"] = board;
    state["turn"] = players[turnIdx];
    return state;
}
bool TicTacToe::isFinished() const {
    // Check rows, columns, and diagonals for a win
    for (int i = 0; i < 3; ++i) {
        if ((board[i][0] == board[i][1] && board[i][1] == board[i][2] && board[i][0] != "") ||
            (board[0][i] == board[1][i] && board[1][i] == board[2][i] && board[0][i] != "")) {
            return true;
        }
    }
    if ((board[0][0] == board[1][1] && board[1][1] == board[2][2] && board[0][0] != "") ||
        (board[0][2] == board[1][1] && board[1][1] == board[2][0] && board[0][2] != "")) {
        return true;
    }
    // Check for a draw
    for (const auto& row : board) {
        for (const auto& cell : row) {
            if (cell == "") {
                return false; // Game is still ongoing
            }
        }
    }
    return true; // Game is a draw
}

