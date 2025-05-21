#include "online_service.hpp"

void OnlineService::add(const std::string& username) {
    std::lock_guard<std::mutex> lock(online_users_mutex);
    online_users.insert(username);
}

void OnlineService::remove(const std::string& username) {
    std::lock_guard<std::mutex> lock(online_users_mutex);
    online_users.erase(username);
}

std::unordered_set<std::string> OnlineService::get_online_users() {
    std::lock_guard<std::mutex> lock(online_users_mutex);
    return online_users;
}
