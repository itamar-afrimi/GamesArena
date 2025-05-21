#pragma once
#include <string>
#include <unordered_map>
#include <mutex>

class UserService {
public:
    bool signup(const std::string& username, const std::string& password);
    bool login(const std::string& username, const std::string& password);
    void logout(const std::string& username);
    bool exists(const std::string& username);

    std::unordered_map<std::string, std::string> users;
    std::mutex user_mutex;
};
