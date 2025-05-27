#pragma once
#include <string>
#include <unordered_set>
#include <mutex>
#include <pqxx/pqxx>

enum class SignupResult {
    Success,
    InvalidUsername,
    WeakPassword,
    UsernameExists,
    DatabaseError
};
enum class PasswordStrength {
    Valid,
    TooShort,
    CommonPassword,
    MissingUpper,
    MissingLower,
    MissingDigit,
    MissingSpecial
};

class UserService {
public:
    UserService(const std::string& conn_str);
    SignupResult signup(const std::string& username, const std::string& password);
    PasswordStrength checkPasswordStrength(const std::string& password) const;
    bool login(const std::string& username, const std::string& password);
    void logout(const std::string& username);
    bool exists(const std::string& username);
    PasswordStrength lastPasswordError = PasswordStrength::Valid;
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
