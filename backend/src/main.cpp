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

#include <aws/core/Aws.h>
#include <aws/dynamodb/DynamoDBClient.h>
#include <memory>

int main() {
    // 1. Initialize AWS SDK
    Aws::SDKOptions options;
    Aws::InitAPI(options);

    {
        // 2. Create DynamoDB client
        auto ddb_client = std::make_shared<Aws::DynamoDB::DynamoDBClient>();

        // 3. PostgreSQL connection string (use env vars or config in production!)
        // std::string pg_conn_str = "host=database-users.cpaycuiokmmm.eu-north-1.rds.amazonaws.com user=postgres password=i7xjabmVeQMgUbfR4IPC dbname=postgres";
        std::string pg_conn_str = 
        "host=" + std::string(std::getenv("PG_HOST") ? std::getenv("PG_HOST") : "") +
        " user=" + std::string(std::getenv("PG_USER") ? std::getenv("PG_USER") : "") +
        " password=" + std::string(std::getenv("PG_PASSWORD") ? std::getenv("PG_PASSWORD") : "") +
        " dbname=" + std::string(std::getenv("PG_DB") ? std::getenv("PG_DB") : "");

        // 4. Construct services with dependencies
        UserService userService(pg_conn_str);
        OnlineService onlineService(ddb_client);
        SessionService sessionService;
        GameManager gameManager;

        // 5. Set up Crow app and routes
        crow::App<CORS> app;
        register_auth_routes(app, userService, onlineService);
        register_lobby_routes(app, onlineService);
        register_game_routes(app, sessionService, gameManager);
        register_ws_game(app, sessionService);

        // 6. Run server
        const char* port_env = std::getenv("PORT");
        uint16_t port = port_env ? static_cast<uint16_t>(std::stoi(port_env)) : 8080;
        app.port(port).multithreaded().run();
    }

    // 7. Shutdown AWS SDK
    Aws::ShutdownAPI(options);
    return 0;
}
