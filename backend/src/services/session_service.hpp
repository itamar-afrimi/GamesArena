#pragma once
#include <unordered_map>
#include <string>
#include <mutex>
#include "../models/session.hpp"
#include "../compoments/game_manager.hpp"

struct FindOrCreateResult {
    std::string sessionId;
    bool isMatched = false;
    std::string error; // empty if no error
};

class SessionService {
public:
    std::unordered_map<std::string, Session> sessions;
    std::mutex sessions_mutex;
    std::unordered_map<std::string, std::string> waiting_session; // gameType → sessionId

    std::string makeSessionId();
    void set_conn_session_id(crow::websocket::connection& conn, const std::string& sessionId);
    std::string get_conn_session_id(crow::websocket::connection& conn);
    FindOrCreateResult findOrCreateSession(const std::string& gameType, const std::string& username, GameManager& gameManager);
};
