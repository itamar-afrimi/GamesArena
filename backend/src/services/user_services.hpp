#pragma once
#include <string>
#include <unordered_set>
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
    bool isValidUsername(const std::string& username) const;
    bool isStrongPassword(const std::string& password) const;
    bool isCommonPassword(const std::string& password) const;

    // Reserved usernames (could also be static in .cpp)
    const std::unordered_set<std::string> reservedUsernames = {
        "admin", "root", "system"
    };

    
};
