#include "crow.h"
#include <crow/websocket.h>
#include "cors_middleware.h"
#include <unordered_map>
#include <string>
#include <mutex>
#include <unordered_set>
#include "../src/compoments/game.hpp"
#include "../src/compoments/game_manager.cpp"

std::unordered_map<std::string, std::string> users; // username -> password
std::mutex user_mutex;
std::unordered_set<std::string> online_users; // usernames of online users
std::mutex online_users_mutex;
std::unordered_map<std::string, std::unordered_set<std::string>> game_sessions;  // game -> users
// std::unordered_map<std::string, GameState> game_states;  // game -> GameState

struct Session {
  std::string                gameType;
  std::vector<std::string>   players;
  std::unique_ptr<Game>      game;
  std::vector<crow::websocket::connection*> conns;
    bool finished = false; 

};

std::mutex                                            sessions_mutex;
std::unordered_map<std::string, Session>             sessions;           // sessionId → Session

// waiting list by gameType
std::unordered_map<std::string, std::string>         waiting_session;    // gameType → sessionId

// Helper: store sessionId in connection's user data
void set_conn_session_id(crow::websocket::connection& conn, const std::string& sessionId) {
    // Allocate a new string and store the pointer in user data
    conn.userdata(new std::string(sessionId));
}

// Helper: get sessionId from connection's user data
std::string get_conn_session_id(crow::websocket::connection& conn) {
    void* ptr = conn.userdata();
    std::cerr << "conn.userdata(): " << conn.userdata() << std::endl;

    if (!ptr){
        std::cerr << "Warning: userdata is null in get_conn_session_id\n";
        CROW_LOG_ERROR << "No sessionId found in connection user data";
        return "";
    } 
    return *static_cast<std::string*>(ptr);
}

// onopen: wait for client to send sessionId in first message
void on_game_open(crow::websocket::connection& conn) {
    // Do nothing here; wait for sessionId in first message
    conn.userdata(nullptr);
}

// onmessage: expect sessionId in first message, then associate conn with session
void on_game_message(crow::websocket::connection& conn, const std::string& msg, bool is_binary) {
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
        set_conn_session_id(conn, sessionId);

        std::lock_guard<std::mutex> lock(sessions_mutex);
        auto it = sessions.find(sessionId);
        if (it == sessions.end()) {
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
        sessionId = get_conn_session_id(conn);
    }

    // Now handle the game move...
    if (!payload.has("username")) {
        CROW_LOG_ERROR << "No username in message";
        return;
    }
    std::string playerId = payload["username"].s();

    std::lock_guard<std::mutex> lock(sessions_mutex);
    auto it = sessions.find(sessionId);
    if (it == sessions.end() || !it->second.game) {
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

void on_game_close(crow::websocket::connection& conn, const std::string& reason, uint16_t code) {
    void* ptr = conn.userdata();
    if (!ptr) {
        // Already cleaned up or never set
        CROW_LOG_INFO << "[WS] on_game_close called but userdata was null";
        return;
    }
    std::string sessionId = *static_cast<std::string*>(ptr);

    {
        std::lock_guard<std::mutex> lock(sessions_mutex);
        auto it = sessions.find(sessionId);
        if (it != sessions.end()) {
            auto& conns = it->second.conns;
            auto connIt = std::remove(conns.begin(), conns.end(), &conn);
            if (connIt != conns.end()) {
                conns.erase(connIt, conns.end());
                CROW_LOG_INFO << "[WS] Connection closed for session " << sessionId << ". Remaining: " << conns.size();
                if (conns.empty()) {
                    sessions.erase(it);
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





std::string makeSessionId() {
    static const char alphanum[] =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string sessionId;
    for (int i = 0; i < 16; ++i) {
        sessionId += alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    return sessionId;
}



int main()
{
    crow::App<CORS> app;  // Add CORS middleware here

    // Entry point for the API
    CROW_ROUTE(app, "/")
    ([](const crow::request&, crow::response& res){
        res.write("Hello!");
        res.end();
    });
    CROW_ROUTE(app, "/api/signup").methods("POST"_method)([](const crow::request& req, crow::response& res){
        auto body = crow::json::load(req.body);
        if (!body) {
            res.code = 400;
            res.end("Invalid JSON");
            return;
        }
    
        std::string username = body["username"].s();
        std::string password = body["password"].s();
    
        std::lock_guard<std::mutex> lock(user_mutex);
        if (users.count(username)) {
            res.code = 409;  // Conflict
            res.end("Username already exists");
            return;
        }
        
        users[username] = password;
        {
            std::lock_guard<std::mutex> lobby_lock(online_users_mutex);
            online_users.insert(username);
        }
        res.code = 200;
        res.set_header("Content-Type", "application/json");
        res.write(R"({"message":"Signup successful"})");
        res.end();

    });
    
    CROW_ROUTE(app, "/api/login").methods("POST"_method)([](const crow::request& req, crow::response& res){
        auto body = crow::json::load(req.body);
        if (!body) {
            res.code = 400;
            res.end("Invalid JSON");
            return;
        }
    
        std::string username = body["username"].s();
        std::string password = body["password"].s();
    
        std::lock_guard<std::mutex> lock(user_mutex);
        if (users.count(username) == 0 || users[username] != password) {
            res.code = 401;
            res.end("Invalid credentials");
            return;
        }
        
        {
            std::lock_guard<std::mutex> lobby_lock(online_users_mutex);
            online_users.insert(username);
        }
    
        res.code = 200;
        res.end("Login successful");
    });
    CROW_ROUTE(app, "/api/lobby").methods("GET"_method)([](const crow::request&, crow::response& res){
        crow::json::wvalue result;
        {
            std::lock_guard<std::mutex> lock(online_users_mutex);
            int i = 0;
            for (const auto& user : online_users) {
                result["users"][i++] = user;
            }
        }
    
        res.code = 200;
        res.set_header("Content-Type", "application/json");


        res.write(result.dump());  // Writing back online users.
        res.end();
    });
    CROW_ROUTE(app, "/api/games").methods("GET"_method)([](const crow::request&, crow::response& res) {
        crow::json::wvalue result;
        result["games"] = {"Tic Tac Toe", "Game 2", "Game 3"};  // Hardcoded list, or you can fetch it from a database
        res.code = 200;
        res.write(result.dump());  // Serialize the list of games to JSON
        res.end();
    });
    
    CROW_ROUTE(app, "/api/start_or_join").methods("POST"_method)(
    [](const crow::request& req){
        auto body = crow::json::load(req.body);
        crow::json::wvalue resp;
        if (!body || !body.has("username") || !body.has("gameType")) {
            resp["error"] = "Invalid request";
            return crow::response{400, resp.dump()};
        }

        std::string username = body["username"].s();
        std::string gameType = body["gameType"].s();
        std::string sessionId;
        bool isMatched = false;

        std::lock_guard<std::mutex> lock(sessions_mutex);

        // 1. Check if this user is already in a waiting session
        for (auto& [sessId, sess] : sessions) {
            if (sess.gameType == gameType && !sess.players.empty() && sess.players[0] == username) {
                if (sess.finished) continue;
                sessionId = sessId;
                isMatched = (sess.players.size() == 2 && sess.game != nullptr);
                CROW_LOG_INFO << "[JOIN] " << username << " already in session " << sessionId << ", isMatched=" << isMatched;
                resp["sessionId"] = sessionId;
                resp["isMatched"] = isMatched;
                return crow::response{200, resp.dump()};
            }
        }

        // 2. Otherwise, check if someone is waiting
        auto waitIt = waiting_session.find(gameType);
        if (waitIt == waiting_session.end()) {
            // No one waiting: create session
            sessionId = makeSessionId();
            sessions[sessionId] = Session{ gameType, {username}, nullptr, {} };
            waiting_session[gameType] = sessionId;
            isMatched = false;
            CROW_LOG_INFO << "[CREATE] " << username << " created session " << sessionId;
        } else {
            // Someone is waiting: join them
            sessionId = waitIt->second;
            Session& sess = sessions[sessionId];

            // Defensive: Only allow one more player, and not the same user
            if (sess.players.size() == 1 && sess.players[0] != username && !sess.game) {
                sess.players.push_back(username);
                GameManager GameManager;
                sess.game = GameManager.create_game(sess.gameType);
                if (!sess.game) {
                    resp["error"] = "Failed to create game";
                    CROW_LOG_ERROR << "[ERROR] Failed to create game for session " << sessionId;
                    return crow::response{500, resp.dump()};
                }
                sess.game->init(sessionId, sess.players);
                waiting_session.erase(waitIt);
                isMatched = true;
                CROW_LOG_INFO << "[MATCH] " << username << " joined session " << sessionId << " with " << sess.players[0];
            } else if (sess.players.size() == 2) {
                // Already matched, just return info
                isMatched = (sess.game != nullptr);
                CROW_LOG_INFO << "[INFO] Session " << sessionId << " already matched.";
                
            } else {
                // Defensive: more than 2 players or same user or game already created
                resp["error"] = "Session full or invalid state";
                CROW_LOG_ERROR << "[ERROR] Session " << sessionId << " full or invalid";
                return crow::response{400, resp.dump()};
            }
        }
        
        resp["sessionId"] = sessionId;
        resp["isMatched"] = isMatched;
        return crow::response{200, resp.dump()};
    }
);
    CROW_ROUTE(app, "/api/logout").methods("POST"_method)
    ([](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body || !body.has("username")) {
            return crow::response(400, "Missing username");
        }
        std::string username = body["username"].s();

        {
            std::lock_guard<std::mutex> lock(online_users_mutex);
            online_users.erase(username);
        }

        crow::json::wvalue res;
        res["message"] = "Logged out";
        return crow::response{res};
    });


    CROW_ROUTE(app, "/ws/game/<string>")  
    .websocket(&app)  
    .onopen(on_game_open)  
    .onmessage(on_game_message)  
    .onclose(on_game_close);

    
    const char* port_env = std::getenv("PORT");
    uint16_t port = port_env ? static_cast<uint16_t>(std::stoi(port_env)) : 8080;

    app.port(port).multithreaded().run();

}