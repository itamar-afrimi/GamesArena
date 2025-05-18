#include "crow.h"
#include "cors_middleware.h"
#include <unordered_map>
#include <string>
#include <mutex>
#include <unordered_set>


std::unordered_map<std::string, std::string> users; // username -> password
std::mutex user_mutex;
std::unordered_set<std::string> online_users; // usernames of online users
std::mutex online_users_mutex;
std::unordered_map<std::string, std::unordered_set<std::string>> game_sessions;  // game -> users
// std::unordered_map<std::string, GameState> game_states;  // game -> GameState


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
        result["games"] = {"Game 1", "Game 2", "Game 3"};  // Hardcoded list, or you can fetch it from a database
        res.code = 200;
        res.write(result.dump());  // Serialize the list of games to JSON
        res.end();
    });
    
    CROW_ROUTE(app, "/api/start_game").methods("POST"_method)([](const crow::request& req, crow::response& res) {
        auto body = crow::json::load(req.body);
        if (!body) {
            res.code = 400;
            res.end("Invalid JSON");
            return;
        }
    });
    CROW_ROUTE(app, "/api/check_match").methods("POST"_method)([](const crow::request& req, crow::response& res) {
        auto body = crow::json::load(req.body);
        crow::json::wvalue result;
        if (!body) {
            res.code = 400;
            res.end("Invalid JSON");
            return;
        }
    
        std::string username = body["username"].s();
        result["ready"] = true;
        // res.end("Game started successfully");
        res.write(result.dump());
        res.end();


    });
    uint16_t port = std::stoi(std::getenv("PORT"));

    app.port(port).multithreaded().run();
}