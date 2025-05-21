#include "user_services.hpp"

UserService::UserService(const std::string& conn_str)
    : db_conn(conn_str) {}

bool UserService::signup(const std::string& username, const std::string& password) {
    try {
        pqxx::work txn(db_conn);
        pqxx::result r = txn.exec_params(
            "INSERT INTO users (username, password) VALUES ($1, $2) ON CONFLICT DO NOTHING RETURNING username",
            username, password
        );
        txn.commit();
        return !r.empty(); // If row was inserted, signup succeeded
    } catch (const std::exception& e) {
        // Log error
        return false;
    }
}

bool UserService::login(const std::string& username, const std::string& password) {
    try {
        pqxx::work txn(db_conn);
        pqxx::result r = txn.exec_params(
            "SELECT password FROM users WHERE username = $1",
            username
        );
        txn.commit();
        if (r.empty()) return false;
        return r[0][0].as<std::string>() == password;
    } catch (const std::exception& e) {
        // Log error
        return false;
    }
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
