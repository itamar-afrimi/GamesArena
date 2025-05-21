#include "user_services.hpp"

bool UserService::signup(const std::string& username, const std::string& password) {
    std::lock_guard<std::mutex> lock(user_mutex);
    if (users.count(username)) {
        return false; // Username already exists
    }
    users[username] = password;
    return true;
}

bool UserService::login(const std::string& username, const std::string& password) {
    std::lock_guard<std::mutex> lock(user_mutex);
    auto it = users.find(username);
    if (it == users.end() || it->second != password) {
        return false; // User not found or password mismatch
    }
    return true;
}

void UserService::logout(const std::string& username) {
    // No-op for user map, but you might want to remove from online users elsewhere
    // (You can add online user management here if you wish)
}

bool UserService::exists(const std::string& username) {
    std::lock_guard<std::mutex> lock(user_mutex);
    return users.count(username) > 0;
}
