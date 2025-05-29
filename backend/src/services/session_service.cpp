#include "session_service.hpp"
#include <aws/dynamodb/model/PutItemRequest.h>
#include <aws/dynamodb/model/DeleteItemRequest.h>
#include <aws/dynamodb/model/ScanRequest.h>
#include <aws/dynamodb/model/GetItemRequest.h>
#include <aws/dynamodb/model/UpdateItemRequest.h>
#include <aws/dynamodb/model/AttributeValue.h>
#include <aws/core/utils/Outcome.h>
#include <iostream>
#include "../utils/db_utils.hpp"

// #include <nlohmann/json.hpp> // For JSON serialization of players

// using nlohmann::json;
// Add these helpers:


SessionService::SessionService(std::shared_ptr<Aws::DynamoDB::DynamoDBClient> ddb_client)
    : dynamo_client(ddb_client) {}

std::string SessionService::makeSessionId() {
    static const char alphanum[] =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string sessionId;
    for (int i = 0; i < 16; ++i) {
        sessionId += alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    return sessionId;
}

void SessionService::set_conn_session_id(crow::websocket::connection& conn, const std::string& sessionId) {
    conn.userdata(new std::string(sessionId));
}

std::string SessionService::get_conn_session_id(crow::websocket::connection& conn) {
    void* ptr = conn.userdata();
    if (!ptr) {
        CROW_LOG_ERROR << "No sessionId found in connection user data";
        return "";
    }
    return *static_cast<std::string*>(ptr);
}

FindOrCreateResult SessionService::findOrCreateSession(
    const std::string& gameType,
    const std::string& username,
    GameManager& gameManager)
{
    std::lock_guard<std::mutex> lock(sessions_mutex);
    FindOrCreateResult result;

    // 1. Check if user is already in a session
    {
        Aws::DynamoDB::Model::ScanRequest scanReq;
        scanReq.SetTableName(sessions_table);
        scanReq.SetFilterExpression("contains(players, :user) AND gameType = :gtype");
        scanReq.AddExpressionAttributeValues(":user", Aws::DynamoDB::Model::AttributeValue(username));
        scanReq.AddExpressionAttributeValues(":gtype", Aws::DynamoDB::Model::AttributeValue(gameType));
        auto scanOutcome = dynamo_client->Scan(scanReq);
        if (scanOutcome.IsSuccess()) {
            const auto& items = scanOutcome.GetResult().GetItems();
            for (const auto& item : items) {
                auto sessIdIt = item.find("sessionId");
                auto playersIt = item.find("players");
                if (sessIdIt != item.end() && playersIt != item.end()) {
                    result.sessionId = sessIdIt->second.GetS();
                    auto players = split_players(playersIt->second.GetS());
                    result.isMatched = (players.size() == 2);
                    return result;
                }
            }
        }
    }

    // 2. Check waiting_sessions table for a waiting session
    {
        Aws::DynamoDB::Model::GetItemRequest getReq;
        getReq.SetTableName(waiting_sessions_table);
        getReq.AddKey("gameType", Aws::DynamoDB::Model::AttributeValue(gameType));
        auto getOutcome = dynamo_client->GetItem(getReq);
        if (!getOutcome.IsSuccess() || !getOutcome.GetResult().GetItem().count("sessionId")) {
            // No one waiting: create session, add to waiting_sessions
            result.sessionId = makeSessionId();

            Aws::DynamoDB::Model::PutItemRequest putSessReq;
            putSessReq.SetTableName(sessions_table);
            putSessReq.AddItem("sessionId", Aws::DynamoDB::Model::AttributeValue(result.sessionId));
            putSessReq.AddItem("gameType", Aws::DynamoDB::Model::AttributeValue(gameType));
            putSessReq.AddItem("players", Aws::DynamoDB::Model::AttributeValue(username));
            putSessReq.AddItem("created", Aws::DynamoDB::Model::AttributeValue().SetN(std::to_string(time(nullptr))));
            dynamo_client->PutItem(putSessReq);

            Aws::DynamoDB::Model::PutItemRequest putWaitReq;
            putWaitReq.SetTableName(waiting_sessions_table);
            putWaitReq.AddItem("gameType", Aws::DynamoDB::Model::AttributeValue(gameType));
            putWaitReq.AddItem("sessionId", Aws::DynamoDB::Model::AttributeValue(result.sessionId));
            dynamo_client->PutItem(putWaitReq);

            result.isMatched = false;
            return result;
        } else {
            // Someone is waiting: join them
            std::string sessionId = getOutcome.GetResult().GetItem().at("sessionId").GetS();

            // Get session from sessions table
            Aws::DynamoDB::Model::GetItemRequest getSessReq;
            getSessReq.SetTableName(sessions_table);
            getSessReq.AddKey("sessionId", Aws::DynamoDB::Model::AttributeValue(sessionId));
            auto getSessOutcome = dynamo_client->GetItem(getSessReq);
            if (!getSessOutcome.IsSuccess()) {
                result.error = "Failed to fetch waiting session";
                return result;
            }
            auto sessionItem = getSessOutcome.GetResult().GetItem();
            auto playersIt = sessionItem.find("players");
            if (playersIt == sessionItem.end()) {
                result.error = "Invalid session state";
                return result;
            }
            auto players = split_players(playersIt->second.GetS());
            if (players.size() != 1 || players[0] == username) {
                result.error = "Session full or invalid state";
                return result;
            }
            players.push_back(username);

            // Update session with both players
            Aws::DynamoDB::Model::UpdateItemRequest updateSessReq;
            updateSessReq.SetTableName(sessions_table);
            updateSessReq.AddKey("sessionId", Aws::DynamoDB::Model::AttributeValue(sessionId));
            updateSessReq.SetUpdateExpression("SET players = :players");
            updateSessReq.AddExpressionAttributeValues(":players", Aws::DynamoDB::Model::AttributeValue(join_players(players)));
            dynamo_client->UpdateItem(updateSessReq);

            // Remove from waiting_sessions
            Aws::DynamoDB::Model::DeleteItemRequest delWaitReq;
            delWaitReq.SetTableName(waiting_sessions_table);
            delWaitReq.AddKey("gameType", Aws::DynamoDB::Model::AttributeValue(gameType));
            dynamo_client->DeleteItem(delWaitReq);

            result.sessionId = sessionId;
            result.isMatched = true;
            return result;
        }
    }
}
