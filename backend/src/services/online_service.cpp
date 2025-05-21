#include "online_service.hpp"
#include <aws/dynamodb/model/PutItemRequest.h>
#include <aws/dynamodb/model/DeleteItemRequest.h>
#include <aws/dynamodb/model/ScanRequest.h>
#include <aws/dynamodb/model/AttributeValue.h>
#include <aws/core/utils/Outcome.h>
#include <iostream>

OnlineService::OnlineService(std::shared_ptr<Aws::DynamoDB::DynamoDBClient> ddb_client)
    : dynamo_client(ddb_client) {}

void OnlineService::add(const std::string& username) {
    Aws::DynamoDB::Model::PutItemRequest req;
    req.SetTableName(table_name);
    req.AddItem("username", Aws::DynamoDB::Model::AttributeValue(username));
    // Optionally, add a timestamp or TTL attribute here
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
