#include "crow.h"
#include "cors_middleware.h"
#include <unordered_map>
#include <string>
#include <mutex>
#include <unordered_set>
#include "../src/compoments/game.hpp"


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
};

std::mutex                                            sessions_mutex;
std::unordered_map<std::string, Session>             sessions;           // sessionId → Session

// waiting list by gameType
std::unordered_map<std::string, std::string>         waiting_session;    // gameType → sessionId


void on_game_open(connection& conn, const string& sessionId) {
  auto& session = sessions[sessionId];
  session.conns.push_back(&conn);
  conn.send_text(session.game->getState().dump());
}
void on_game_message(connection& conn, const string& msg, const string& sessionId) {
    auto payload = crow::json::load(msg);
    string playerId = payload["username"].s();
    auto& sess = sessions[sessionId];

    // 1. Apply the move
    sess.game->handleMove(playerId, payload);

    // 2. Grab the updated state
    auto stateJson = sess.game->getState();
    bool finished   = sess.game->isFinished();

    // 3. If finished, add a “finished” flag
    stateJson["finished"] = finished;

    // 4. Broadcast to all connected clients
    string out = stateJson.dump();
    for (auto* c : sess.conns) {
        c->send_text(out);
    }

    // 5. If the game is over, clean up the session
    if (finished) {
        std::lock_guard<std::mutex> lock(sessions_mutex);
        sessions.erase(sessionId);
        // (optional) also remove from waiting_session if present
    }
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
        res.end("Signup successful");
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
    
    CROW_ROUTE(app, "/api/start_or_join").methods("POST"_method)
    ([](const crow::request& req){
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

        {
        std::lock_guard<std::mutex> lock(sessions_mutex);
        auto waitIt = waiting_session.find(gameType);

        if (waitIt == waiting_session.end()) {
            // First player: create and wait
            sessionId = makeSessionId();
            sessions[sessionId] = Session{ gameType, {username}, nullptr, {} };
            waiting_session[gameType] = sessionId;
            isMatched = false;

        } else {
            // Second player: join, init, remove from waiting
            sessionId = waitIt->second;
            Session& sess = sessions[sessionId];
            sess.players.push_back(username);
            sess.game = makeGame(sess.gameType);
            sess.game->init(sessionId, sess.players);
            waiting_session.erase(waitIt);
            isMatched = true;
        }
        }

        resp["sessionId"] = sessionId;
        resp["isMatched"] = isMatched;
        return crow::response{200, resp.dump()};
    });

    CROW_ROUTE(app, "/ws/game/<string>")  
    .websocket()  
    .onopen(on_game_open)  
    .onmessage(on_game_message)  
    .onclose(on_game_close);

    // CROW_ROUTE(app, "/api/check_match").methods("POST"_method)([](const crow::request& req, crow::response& res) {
    //     auto body = crow::json::load(req.body);
    //     crow::json::wvalue result;
    //     if (!body) {
    //         res.code = 400;
    //         res.end("Invalid JSON");
    //         return;
    //     }
    
    //     std::string username = body["username"].s();
    //     result["ready"] = true;
    //     // res.end("Game started successfully");
    //     res.write(result.dump());
    //     res.end();


    // });
    const char* port_env = std::getenv("PORT");
    uint16_t port = port_env ? static_cast<uint16_t>(std::stoi(port_env)) : 8080;

    app.port(port).multithreaded().run();

}