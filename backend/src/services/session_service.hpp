#pragma once
#include <string>
#include <mutex>
#include <memory>
#include <aws/dynamodb/DynamoDBClient.h>
#include "../models/session.hpp"
#include "../compoments/game_manager.hpp"

struct FindOrCreateResult {
    std::string sessionId;
    bool isMatched = false;
    std::string error; // empty if no error
};

class SessionService {
public:
    SessionService(std::shared_ptr<Aws::DynamoDB::DynamoDBClient> ddb_client);

    std::string makeSessionId();
    void set_conn_session_id(crow::websocket::connection& conn, const std::string& sessionId);
    std::string get_conn_session_id(crow::websocket::connection& conn);
    FindOrCreateResult findOrCreateSession(const std::string& gameType, const std::string& username, GameManager& gameManager);
    const std::string sessions_table = "sessions";
    const std::string waiting_sessions_table = "waiting_sessions";
    std::shared_ptr<Aws::DynamoDB::DynamoDBClient> dynamo_client;
private:
    
    std::mutex sessions_mutex;
    
};
