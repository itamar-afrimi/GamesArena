#pragma once
#include <unordered_set>
#include <string>
#include <mutex>

class OnlineService {
public:
    void add(const std::string& username);
    void remove(const std::string& username);
    std::unordered_set<std::string> get_online_users();

private:
    std::unordered_set<std::string> online_users;
    std::mutex online_users_mutex;
};
