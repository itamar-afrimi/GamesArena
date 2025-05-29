#include "online_service.hpp"
#include <aws/dynamodb/model/PutItemRequest.h>
#include <aws/dynamodb/model/DeleteItemRequest.h>
#include <aws/dynamodb/model/ScanRequest.h>
#include <aws/dynamodb/model/AttributeValue.h>
#include <aws/core/utils/Outcome.h>
#include <iostream>
#include <chrono>

OnlineService::OnlineService(std::shared_ptr<Aws::DynamoDB::DynamoDBClient> ddb_client)
    : dynamo_client(ddb_client) {}
/**
 * @brief Adds a user to the online users table in DynamoDB.
 */
void OnlineService::add(const std::string& username) {

    Aws::DynamoDB::Model::PutItemRequest req;
    std::cout << "[INFO] OnlineService::add called for user: " << username << std::endl;


    req.SetTableName(table_name);
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    req.AddItem("username", Aws::DynamoDB::Model::AttributeValue(username));

    // Add a timestamp or TTL attribute here
    req.AddItem("timestamp", Aws::DynamoDB::Model::AttributeValue().SetN(std::to_string(timestamp)));
    auto ttl = timestamp + 300;
    req.AddItem("ttl", Aws::DynamoDB::Model::AttributeValue().SetN(std::to_string(ttl)));



    auto outcome = dynamo_client->PutItem(req);
    if (!outcome.IsSuccess()) {
        std::cerr << "Failed to add user online: " << outcome.GetError().GetMessage() << std::endl;
    }
}

void OnlineService::remove(const std::string& username) {
    Aws::DynamoDB::Model::DeleteItemRequest req;
    req.SetTableName(table_name);
    req.AddKey("username", Aws::DynamoDB::Model::AttributeValue(username));
    auto outcome = dynamo_client->DeleteItem(req);
    if (!outcome.IsSuccess()) {
        std::cerr << "Failed to remove user online: " << outcome.GetError().GetMessage() << std::endl;
    }
}

std::unordered_set<std::string> OnlineService::get_online_users() {

    std::unordered_set<std::string> result;
    Aws::DynamoDB::Model::ScanRequest req;
    req.SetTableName(table_name);

    auto now = std::chrono::system_clock::now();
    auto cutoff = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count() - 300; // 5 minutes ago

    req.SetFilterExpression("#ts > :cutoff");
    req.AddExpressionAttributeNames("#ts", "timestamp");
    req.AddExpressionAttributeValues(":cutoff", Aws::DynamoDB::Model::AttributeValue().SetN(std::to_string(cutoff)));

    auto outcome = dynamo_client->Scan(req);
    if (outcome.IsSuccess()) {
        const auto& items = outcome.GetResult().GetItems();
        for (const auto& item : items) {
            auto it = item.find("username");
            if (it != item.end()) {
                result.insert(it->second.GetS());
            }
        }
    } else {
        std::cerr << "Failed to scan online users: " << outcome.GetError().GetMessage() << std::endl;
    }
    return result;
}
