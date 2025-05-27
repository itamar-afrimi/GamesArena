#include "user_services.hpp"
#include "bcrypt.h"
#include <regex>
#include <algorithm>
#include <unordered_set>

UserService::UserService(const std::string& conn_str)
    : db_conn(conn_str) {}

bool UserService::signup(const std::string& username, const std::string& password) {
        // Validate username
        std::string uname = username;
        std::transform(uname.begin(), uname.end(), uname.begin(), ::tolower);
        if (!isValidUsername(uname)) return false;

        // Validate password
        if (!isStrongPassword(password)) return false;

        // Check for existing user
        if (exists(uname)) return false;

        // Hash password
        std::string hash = bcrypt::generateHash(password);

        // Store in DB
        try {
            pqxx::work txn(db_conn);
            pqxx::result r = txn.exec_params(
                "INSERT INTO users (username, password) VALUES ($1, $2) ON CONFLICT DO NOTHING RETURNING username",
                uname, hash
            );
            txn.commit();
            return !r.empty();
        } catch (const std::exception& e) {
            return false;
        }
}
bool UserService::login(const std::string& username, const std::string& password) {
    
    std::string uname = username;
    std::transform(uname.begin(), uname.end(), uname.begin(), ::tolower);

    try {
        pqxx::work txn(db_conn);
        pqxx::result r = txn.exec_params(
            "SELECT password FROM users WHERE username = $1",
            uname
        );
        txn.commit();
        if (r.empty()) return false;
        std::string hash = r[0][0].as<std::string>();
        return bcrypt::validatePassword(password, hash);
    } catch (const std::exception& e) {
        return false;
    }
}
bool UserService::isValidUsername(const std::string& username) const {
    if (username.length() < 3 || username.length() > 20) return false;
    static std::unordered_set<std::string> reserved = {"admin", "root", "system"};
    if (reserved.count(username)) return false;
    std::regex valid("^[a-zA-Z0-9_]+$");
    return std::regex_match(username, valid);
}

bool UserService::isStrongPassword(const std::string& password) const {
    if (password.length() < 8) return false;
    static std::unordered_set<std::string> common = {"password", "123456", "qwerty"};
    if (common.count(password)) return false;
    bool upper = false, lower = false, digit = false, special = false;
    for (char c : password) {
        if (std::isupper(c)) upper = true;
        else if (std::islower(c)) lower = true;
        else if (std::isdigit(c)) digit = true;
        else special = true;
    }
    return upper && lower && digit && special;
}

void UserService::logout(const std::string& username) {
    // No-op or implement session management as needed
}

bool UserService::exists(const std::string& username) {
    try {
        pqxx::work txn(db_conn);
        pqxx::result r = txn.exec_params(
            "SELECT 1 FROM users WHERE username = $1",
            username
        );
        txn.commit();
        return !r.empty();
    } catch (const std::exception& e) {
        // Log error
        return false;
    }
}
