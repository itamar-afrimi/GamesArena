#include "crow.h"
#include <crow/websocket.h>
#include "cors_middleware.h"
#include "api/auth_routes.hpp"
#include "api/lobby_routes.hpp"
#include "api/game_routes.hpp"
#include "api/ws_game.hpp"
#include "services/user_services.hpp"
#include "services/online_service.hpp"
#include "services/session_service.hpp"
#include "compoments/game_manager.hpp"

int main() {
    crow::App<CORS> app;

    UserService userService;
    OnlineService onlineService;
    SessionService sessionService;
    GameManager gameManager;

    register_auth_routes(app, userService, onlineService);
    register_lobby_routes(app, onlineService);
    register_game_routes(app, sessionService, gameManager);
    register_ws_game(app, sessionService);

    const char* port_env = std::getenv("PORT");
    uint16_t port = port_env ? static_cast<uint16_t>(std::stoi(port_env)) : 8080;
    app.port(port).multithreaded().run();
}
