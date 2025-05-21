#include "session_service.hpp"
#include <cstdlib>

std::string SessionService::makeSessionId() {
    static const char alphanum[] =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string sessionId;
    for (int i = 0; i < 16; ++i) {
        sessionId += alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    return sessionId;
}
void SessionService::set_conn_session_id(crow::websocket::connection& conn, const std::string& sessionId) {

    conn.userdata(new std::string(sessionId));
}
// Helper: get sessionId from connection's user data
std::string SessionService::get_conn_session_id(crow::websocket::connection& conn) {
    void* ptr = conn.userdata();
    std::cerr << "conn.userdata(): " << conn.userdata() << std::endl;

    if (!ptr){
        std::cerr << "Warning: userdata is null in get_conn_session_id\n";
        CROW_LOG_ERROR << "No sessionId found in connection user data";
        return "";
    } 
    return *static_cast<std::string*>(ptr);
}
FindOrCreateResult SessionService::findOrCreateSession(
    const std::string& gameType,
    const std::string& username,
    GameManager& gameManager)
{
    std::lock_guard<std::mutex> lock(sessions_mutex);
    FindOrCreateResult result;

    // 1. Check if this user is already in a waiting session
    for (auto& [sessId, sess] : sessions) {
        if (sess.gameType == gameType && !sess.players.empty() && sess.players[0] == username) {
            if (sess.finished) continue;
            result.sessionId = sessId;
            result.isMatched = (sess.players.size() == 2 && sess.game != nullptr);
            return result;
        }
    }

    // 2. Otherwise, check if someone is waiting
    auto waitIt = waiting_session.find(gameType);
    if (waitIt == waiting_session.end()) {
        // No one waiting: create session
        result.sessionId = makeSessionId();
        sessions[result.sessionId] = Session{ gameType, {username}, nullptr, {} };
        waiting_session[gameType] = result.sessionId;
        result.isMatched = false;
        return result;
    } else {
        // Someone is waiting: join them
        result.sessionId = waitIt->second;
        Session& sess = sessions[result.sessionId];

        // Defensive: Only allow one more player, and not the same user
        if (sess.players.size() == 1 && sess.players[0] != username && !sess.game) {
            sess.players.push_back(username);
            sess.game = gameManager.create_game(sess.gameType);
            if (!sess.game) {
                result.error = "Failed to create game";
                return result;
            }
            sess.game->init(result.sessionId, sess.players);
            waiting_session.erase(waitIt);
            result.isMatched = true;
            return result;
        } else if (sess.players.size() == 2) {
            // Already matched, just return info
            result.isMatched = (sess.game != nullptr);
            return result;
        } else {
            // Defensive: more than 2 players or same user or game already created
            result.error = "Session full or invalid state";
            return result;
        }
    }
}
