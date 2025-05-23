#include "auth_routes.hpp"
#include "../cors_middleware.h"

void register_auth_routes(crow::App<CORS>& app, UserService& userService, OnlineService& onlineService) {
    std::cout << "[INFO] Registering /api/signup" << std::endl;

    CROW_ROUTE(app, "/api/signup").methods("POST"_method)
    ([&userService, &onlineService](const crow::request& req, crow::response& res){
        auto body = crow::json::load(req.body);
        std::cout << "[INFO] /api/signup handler called" << std::endl;

        if (!body) { res.code = 400; res.end("Invalid JSON"); return; }
        std::string username = body["username"].s();
        std::string password = body["password"].s();
        if (!userService.signup(username, password)) {

            crow::json::wvalue errorJson;
            errorJson["message"] = "Username already exists";
            res.code = 409;
            res = crow::response(res.code, errorJson); // Clean and clear!
            res.end();
            return;
            

        }
        onlineService.add(username);
        res.code = 200;
        res.set_header("Content-Type", "application/json");
        res.write(R"({"message":"Signup successful"})");
        res.end();
    });

    CROW_ROUTE(app, "/api/login").methods("POST"_method)
    ([&userService, &onlineService](const crow::request& req, crow::response& res){
        auto body = crow::json::load(req.body);
        if (!body) { res.code = 400; res.end("Invalid JSON"); return; }
        std::string username = body["username"].s();
        std::string password = body["password"].s();
        if (!userService.login(username, password)) {
            res.code = 401; res.end("Invalid credentials"); return;
        }
        onlineService.add(username);
        res.code = 200;
        res.end("Login successful");
    });

    CROW_ROUTE(app, "/api/logout").methods("POST"_method)
    ([&onlineService](const crow::request& req){
        auto body = crow::json::load(req.body);
        if (!body || !body.has("username")) return crow::response(400, "Missing username");
        std::string username = body["username"].s();
        onlineService.remove(username);
        crow::json::wvalue res;
        res["message"] = "Logged out";
        return crow::response{res};
    });
   

}
