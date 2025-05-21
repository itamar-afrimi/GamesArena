#include "game_routes.hpp"
#include "../utils/id_generator.hpp"
#include <crow/json.h>
#include "../cors_middleware.h"

void register_game_routes(crow::App<CORS>& app, SessionService& sessionService, GameManager& gameManager) {
    CROW_ROUTE(app, "/api/start_or_join").methods("POST"_method)
    ([&sessionService, &gameManager](const crow::request& req, crow::response& res) {
        auto body = crow::json::load(req.body);
        if (!body) {
            res.code = 400;
            res.end("Invalid JSON");
            return;
        }
        // Defensive: check if keys exist
        if (!body.has("username") || !body.has("gameType")) {
            res.code = 400;
            res.end("Missing username or gameType");
            return;
        }

        std::string username = body["username"].s();
        std::string gameType = body["gameType"].s();

        // Try to join a waiting session, or create a new one
        auto result = sessionService.findOrCreateSession(gameType, username, gameManager);
        // std::cout << sessionId << std::endl;
        crow::json::wvalue response;
        if (!result.error.empty()) {
            response["error"] = result.error;
            res.code = 400;
        } else {
            response["sessionId"] = result.sessionId;
            response["isMatched"] = result.isMatched;
            res.code = 200;
        }
        res.set_header("Content-Type", "application/json");
        res.write(response.dump());
        res.end();
    });
}
