#include <aws/core/Aws.h>
#include <aws/dynamodb/DynamoDBClient.h>
#include <aws/dynamodb/model/ListTablesRequest.h>
#include <iostream>

int main() {
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    {
        Aws::Client::ClientConfiguration config;
        // Optionally set region: config.region = "us-east-1";
        Aws::DynamoDB::DynamoDBClient client(config);

        Aws::DynamoDB::Model::ListTablesRequest request;
        auto outcome = client.ListTables(request);

        if (outcome.IsSuccess()) {
            std::cout << "DynamoDB tables in your account:" << std::endl;
            for (const auto& name : outcome.GetResult().GetTableNames()) {
                std::cout << "  " << name << std::endl;
            }
        } else {
            std::cerr << "Error listing tables: " << outcome.GetError().GetMessage() << std::endl;
        }
    }
    Aws::ShutdownAPI(options);
    return 0;
}
