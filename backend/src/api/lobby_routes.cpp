#include "lobby_routes.hpp"

void register_lobby_routes(crow::App<CORS>& app, OnlineService& onlineService) {
    CROW_ROUTE(app, "/api/lobby").methods("GET"_method)
    ([&onlineService](const crow::request&, crow::response& res){
        crow::json::wvalue result;
        auto users = onlineService.get_online_users();
        size_t i = 0;
        for (const auto& user : users) {
            result["users"][i++] = user;
        }
        res.code = 200;
        res.set_header("Content-Type", "application/json");
        res.write(result.dump());
        res.end();
    });

    CROW_ROUTE(app, "/api/games").methods("GET"_method)
    ([](const crow::request&, crow::response& res) {
        crow::json::wvalue result;
        result["games"] = {"Tic Tac Toe", "Game 2", "Game 3"};
        res.code = 200;
        res.write(result.dump());
        res.end();
    });
}
