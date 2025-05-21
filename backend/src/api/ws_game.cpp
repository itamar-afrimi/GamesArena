#include "ws_game.hpp"
#include "crow.h"
#include <crow/websocket.h>
#include <iostream>
#include <algorithm> // for std::remove
#include "../services/session_service.hpp"
// ... (all your session/user state and helper functions here) ...

void on_game_open(crow::websocket::connection& conn) {
    conn.userdata(nullptr);
}

void on_game_message(crow::websocket::connection& conn, const std::string& msg, bool is_binary, SessionService& sessionService) {
    auto payload = crow::json::load(msg);
    if (!payload) {
        CROW_LOG_ERROR << "Invalid JSON in message";
        return;
    }
    std::string sessionId;
    // If this is the first message, associate connection with session
    if (!conn.userdata()) {
        if (!payload.has("sessionId")) {
            CROW_LOG_ERROR << "No sessionId in first message";
            return;
        }
        sessionId = payload["sessionId"].s();
        sessionService.set_conn_session_id(conn, sessionId);

        std::lock_guard<std::mutex> lock(sessionService.sessions_mutex);
        auto it = sessionService.sessions.find(sessionId);
        if (it == sessionService.sessions.end()) {
            CROW_LOG_ERROR << "Session not found!";
            return;
        }
        auto& session = it->second;
        session.conns.push_back(&conn);
        CROW_LOG_INFO << "[WS] Connection registered to session " << sessionId << ", conns=" << session.conns.size();

        // Defensive: Only send game state if game exists
        if (!session.game) {
            crow::json::wvalue waitMsg;
            waitMsg["waiting"] = true;
            conn.send_text(waitMsg.dump());
            CROW_LOG_INFO << "[WS] Sent waiting to " << sessionId;

            return;
        }

        conn.send_text(session.game->getState().dump());
        return; // First message used for registration only
    } else {
        sessionId = sessionService.get_conn_session_id(conn);
    }

    // Now handle the game move...
    if (!payload.has("username")) {
        CROW_LOG_ERROR << "No username in message";
        return;
    }
    std::string playerId = payload["username"].s();

    std::lock_guard<std::mutex> lock(sessionService.sessions_mutex);
    auto it = sessionService.sessions.find(sessionId);
    if (it == sessionService.sessions.end() || !it->second.game) {
        CROW_LOG_ERROR << "Session or game not found!";
        return;
    }
    auto& sess = it->second;

    // 1. Apply the move
    sess.game->handleMove(playerId, payload);

    // 2. Grab the updated state
    auto stateJson = sess.game->getState();
    bool finished = sess.game->isFinished();

    // 3. If finished, add a “finished” flag
    stateJson["finished"] = finished;

    // 4. Broadcast to all connected clients
    std::string out = stateJson.dump();
    for (auto* c : sess.conns) {
        c->send_text(out);
    }
    CROW_LOG_INFO << "[WS] Broadcasted state to session " << sessionId << ", finished=" << finished;

    // 5. If the game is over, clean up the session
    if (finished) {
        sess.finished = true;
        // sessions.erase(it);
        CROW_LOG_INFO << "[WS] Marked session " << sessionId << " as finished.";

        // (optional) also remove from waiting_session if present
    }
}


void on_game_close(crow::websocket::connection& conn, const std::string& reason, uint16_t code, SessionService& sessionService) {
    void* ptr = conn.userdata();
    if (!ptr) {
        // Already cleaned up or never set
        CROW_LOG_INFO << "[WS] on_game_close called but userdata was null";
        return;
    }
    std::string sessionId = *static_cast<std::string*>(ptr);

    {
        std::lock_guard<std::mutex> lock(sessionService.sessions_mutex);
        auto it = sessionService.sessions.find(sessionId);
        if (it != sessionService.sessions.end()) {
            auto& conns = it->second.conns;
            auto connIt = std::remove(conns.begin(), conns.end(), &conn);
            if (connIt != conns.end()) {
                conns.erase(connIt, conns.end());
                CROW_LOG_INFO << "[WS] Connection closed for session " << sessionId << ". Remaining: " << conns.size();
                if (conns.empty()) {
                    sessionService.sessions.erase(it);
                    CROW_LOG_INFO << "[WS] All connections closed. Session " << sessionId << " erased.";
                }
            }
        }
    }
    // Always clean up user data pointer
    conn.userdata(nullptr); 
    delete static_cast<std::string*>(ptr);
    // Set to nullptr immediately after deletion
    CROW_LOG_INFO << "[WS] Cleaned up user data pointer for session " << sessionId;
}

// This function registers the websocket route with the app
void register_ws_game(crow::App<CORS>& app, SessionService& sessionService) {
    CROW_ROUTE(app, "/ws/game/<string>")
        .websocket(&app)
        .onopen(on_game_open)
        .onmessage([&sessionService](auto& conn, const std::string& msg, bool is_binary) {
            on_game_message(conn, msg, is_binary, sessionService);
        })
        .onclose([&sessionService](auto& conn, const std::string& reason, uint16_t code) {
            on_game_close(conn, reason, code, sessionService);
        });
}
