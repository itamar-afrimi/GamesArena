#include "auth_routes.hpp"
#include "../cors_middleware.h"



void register_auth_routes(crow::App<CORS>& app, UserService& userService, OnlineService& onlineService) {

    CROW_ROUTE(app, "/api/signup").methods("POST"_method)
    ([&userService, &onlineService](const crow::request& req, crow::response& res){
        auto body = crow::json::load(req.body);
        std::cout << "[DEBUG] SIGNUP HANDLER CALLED" << std::endl;

        if (!body) { res.code = 400; res.end("Invalid JSON"); return; }

        std::string username = body["username"].s();
        std::string password = body["password"].s();

        auto result = userService.signup(username, password);
        if (result == SignupResult::InvalidUsername) {
            crow::json::wvalue errorJson;
            errorJson["message"] = "Invalid username (3-20 chars, alphanumeric/underscore)";
            res.code = 400;
            res = crow::response(res.code, errorJson); // Clean and clear!
            res.end();
            return;
        } else if (result == SignupResult::WeakPassword) {
            crow::json::wvalue errorJson;

            switch (userService.lastPasswordError) {
                case PasswordStrength::TooShort:
                    errorJson["message"] = "Password must be at least 8 characters.";
                    break;
                case PasswordStrength::CommonPassword:
                    errorJson["message"] = "Password is too common.";
                    break;
                case PasswordStrength::MissingUpper:
                    errorJson["message"] = "Password must contain at least one uppercase letter.";
                    break;
                case PasswordStrength::MissingLower:
                    errorJson["message"] = "Password must contain at least one lowercase letter.";
                    break;
                case PasswordStrength::MissingDigit:
                    errorJson["message"] = "Password must contain at least one digit.";
                    break;
                case PasswordStrength::MissingSpecial:
                    errorJson["message"] = "Password must contain at least one special character.";
                    break;
                default:
                    errorJson["message"] = "Password does not meet requirements.";
            }
            res.code = 400;
            res = crow::response(res.code, errorJson); // Clean and clear!
            res.end();
            return;

        } else if (result == SignupResult::DatabaseError) {
            crow::json::wvalue errorJson;
            errorJson["message"] = "Database error";
            res.code = 500;
            res = crow::response(res.code, errorJson); // Clean and clear!
            res.end();
            return;
        }
        else if (result == SignupResult::UsernameExists) {
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
            crow::json::wvalue errorJson;
            errorJson["message"] = "Invalid credentials";
            res.code = 409;
            res = crow::response(res.code, errorJson); // Clean and clear!
            res.end();
            return;
        }
        onlineService.add(username);
        
        crow::json::wvalue successJson;
        successJson["message"] = "Login successful";
        res.code = 200;
        res = crow::response(res.code, successJson);
        res.end();

    });

    CROW_ROUTE(app, "/api/logout").methods("POST"_method)
    ([&onlineService](const crow::request& req){
        auto body = crow::json::load(req.body);
        if (!body || !body.has("username")) return crow::response(400, "Missing username");
        std::cout << "[DEBUG] LogOut HANDLER CALLED" << body["username"] << std::endl;

        std::string username = body["username"].s();
        onlineService.remove(username);
        crow::json::wvalue res;
        res["message"] = "Logged out";
        return crow::response{res};
    });
    CROW_ROUTE(app, "/api/heartbeat").methods("POST"_method)
    ([&onlineService](const crow::request& req, crow::response& res){
        std::cout << "[DEBUG] HEARTBEAT HANDLER CALLED" << std::endl;
        auto body = crow::json::load(req.body);
        if (!body || !body.has("username")){
            res.code = 400;
            res.write("Missing username");
            res.end();
            return;
        } 
        std::string username = body["username"].s();
        onlineService.add(username); // Refresh user's online status
        crow::json::wvalue resp;
        resp["message"] = "Heartbeat received";
        res.code = 200;
        res.write(resp.dump()); // Clean and clear!
        res.end();
    });
    
   

}







