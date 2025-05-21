#pragma once
#include <string>
#include <unordered_map>
#include <mutex>
#include <pqxx/pqxx>

class UserService {
public:
    UserService(const std::string& conn_str);
    bool signup(const std::string& username, const std::string& password);
    bool login(const std::string& username, const std::string& password);
    void logout(const std::string& username);
    bool exists(const std::string& username);

   private:
    pqxx::connection db_conn;
};
