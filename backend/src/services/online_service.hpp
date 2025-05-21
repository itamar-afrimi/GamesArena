#pragma once
#include <string>
#include <unordered_set>
#include <aws/dynamodb/DynamoDBClient.h>
#include <memory>

class OnlineService {
public:
    OnlineService(std::shared_ptr<Aws::DynamoDB::DynamoDBClient> ddb_client);

    void add(const std::string& username);
    void remove(const std::string& username);
    std::unordered_set<std::string> get_online_users();

private:
    std::shared_ptr<Aws::DynamoDB::DynamoDBClient> dynamo_client;
    const std::string table_name = "online_users";
};
