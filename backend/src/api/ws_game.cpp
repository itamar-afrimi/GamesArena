#include "ws_game.hpp"
#include "crow.h"
#include <crow/websocket.h>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <vector>
#include <mutex>
#include "../services/session_service.hpp"
#include <aws/dynamodb/DynamoDBClient.h>
#include <aws/dynamodb/model/GetItemRequest.h>
#include <aws/dynamodb/model/AttributeValue.h>
#include <aws/dynamodb/model/UpdateItemRequest.h>
#include "../utils/db_utils.hpp"


// In-memory only: sessionId → vector of websocket connections
std::mutex conns_mutex;
std::unordered_map<std::string, std::vector<crow::websocket::connection*>> session_conns;

// Helper: Fetch session info and game state from DynamoDB
struct SessionDDB {
    std::string sessionId;
    std::string gameType;
    std::vector<std::string> players;
    std::string game_state;
    bool finished = false;
};


SessionDDB fetch_session(SessionService& sessionService, const std::string& sessionId) {
    SessionDDB sess;
    sess.sessionId = sessionId;
    Aws::DynamoDB::Model::GetItemRequest req;
    req.SetTableName(sessionService.sessions_table);
    req.AddKey("sessionId", Aws::DynamoDB::Model::AttributeValue(sessionId));
    auto outcome = sessionService.dynamo_client->GetItem(req);
    if (!outcome.IsSuccess()) return sess;
    const auto& item = outcome.GetResult().GetItem();
    auto it = item.find("gameType");
    if (it != item.end()) sess.gameType = it->second.GetS();
    it = item.find("players");
    if (it != item.end()) sess.players = split_players(it->second.GetS());
    it = item.find("game_state");
    if (it != item.end()) sess.game_state = it->second.GetS();
    it = item.find("finished");
    if (it != item.end()) sess.finished = it->second.GetS() == "1";
    return sess;
}

// Helper: Save game state and finished flag back to DynamoDB
void save_session_state(SessionService& sessionService, const std::string& sessionId, const std::string& game_state, bool finished) {
    Aws::DynamoDB::Model::UpdateItemRequest req;
    req.SetTableName(sessionService.sessions_table);
    req.AddKey("sessionId", Aws::DynamoDB::Model::AttributeValue(sessionId));
    req.SetUpdateExpression("SET game_state = :gs, finished = :fin");
    req.AddExpressionAttributeValues(":gs", Aws::DynamoDB::Model::AttributeValue(game_state));
    req.AddExpressionAttributeValues(":fin", Aws::DynamoDB::Model::AttributeValue(finished ? "1" : "0"));
    sessionService.dynamo_client->UpdateItem(req);
}

void on_game_open(crow::websocket::connection& conn) {
    conn.userdata(nullptr);
}

void on_game_message(crow::websocket::connection& conn, const std::string& msg, bool is_binary, SessionService& sessionService, GameManager& gameManager) {
    auto payload = crow::json::load(msg);
    if (!payload) {
        CROW_LOG_ERROR << "Invalid JSON in message";
        return;
    }
    CROW_LOG_INFO << "[WS] Received message: " << msg;

    std::string sessionId;
    if (!conn.userdata()) {
        if (!payload.has("sessionId") || payload["sessionId"].t() != crow::json::type::String) {
            CROW_LOG_ERROR << "No sessionId in first message";
            return;
        }
        sessionId = payload["sessionId"].s();
        sessionService.set_conn_session_id(conn, sessionId);

        {
            std::lock_guard<std::mutex> lock(conns_mutex);
            session_conns[sessionId].push_back(&conn);
        }

        // Fetch session and game state from DynamoDB
        SessionDDB sess = fetch_session(sessionService, sessionId);

        // If not matched, send waiting
        if (sess.players.size() < 2) {
            crow::json::wvalue waitMsg;
            waitMsg["waiting"] = true;
            conn.send_text(waitMsg.dump());
            CROW_LOG_INFO << "[WS] Sent waiting to " << sessionId;
            return;
        }

        // If matched, send game state (deserialize game and send state)
        auto game = gameManager.create_game(sess.gameType, sess.game_state);
        crow::json::wvalue stateMsg = game->getState();
        stateMsg["waiting"] = false;
        conn.send_text(stateMsg.dump());
        return;
    } else {
        sessionId = sessionService.get_conn_session_id(conn);
    }

    // --- Handle move ---
    if (!payload.has("username") || payload["username"].t() != crow::json::type::String) {
        CROW_LOG_ERROR << "No username in message";
        return;
    }
    std::string playerId = payload["username"].s();

    // Fetch session and game state from DynamoDB
    SessionDDB sess = fetch_session(sessionService, sessionId);
    if (sess.players.size() < 2 || sess.gameType.empty()) {
        CROW_LOG_ERROR << "Session not matched or invalid!";
        return;
    }
    auto game = gameManager.create_game(sess.gameType, sess.game_state);
    if (!game) {
        CROW_LOG_ERROR << "Game object could not be created!";
        return;
    }

    // 1. Apply the move
    game->handleMove(playerId, payload);

    // 2. Grab the updated state
    auto stateJson = game->getState();
    bool finished = game->isFinished();
    stateJson["finished"] = finished;

    // 3. Save updated game state to DynamoDB
    save_session_state(sessionService, sessionId, game->serialize(), finished);

    // 4. Broadcast to all connected clients
    std::string out = stateJson.dump();
    {
        std::lock_guard<std::mutex> lock(conns_mutex);
        for (auto* c : session_conns[sessionId]) {
            c->send_text(out);
        }
    }
    CROW_LOG_INFO << "[WS] Broadcasted state to session " << sessionId << ", finished=" << finished;
}

void on_game_close(crow::websocket::connection& conn, const std::string& reason, uint16_t code, SessionService& sessionService) {
    void* ptr = conn.userdata();
    if (!ptr) {
        CROW_LOG_INFO << "[WS] on_game_close called but userdata was null";
        return;
    }
    std::string sessionId = *static_cast<std::string*>(ptr);

    {
        std::lock_guard<std::mutex> lock(conns_mutex);
        auto it = session_conns.find(sessionId);
        if (it != session_conns.end()) {
            auto& conns = it->second;
            auto connIt = std::remove(conns.begin(), conns.end(), &conn);
            if (connIt != conns.end()) {
                conns.erase(connIt, conns.end());
                if (conns.empty()) {
                    session_conns.erase(it);
                }
            }
        }
    }
    conn.userdata(nullptr); 
    delete static_cast<std::string*>(ptr);
    CROW_LOG_INFO << "[WS] Cleaned up user data pointer for session " << sessionId;
}

void register_ws_game(crow::App<CORS>& app, SessionService& sessionService, GameManager& gameManager) {
    CROW_ROUTE(app, "/ws/game/<string>")
        .websocket(&app)
        .onopen(on_game_open)
        .onmessage([&sessionService, &gameManager](auto& conn, const std::string& msg, bool is_binary) {
            on_game_message(conn, msg, is_binary, sessionService, gameManager);
        })
        .onclose([&sessionService](auto& conn, const std::string& reason, uint16_t code) {
            on_game_close(conn, reason, code, sessionService);
        });
}
