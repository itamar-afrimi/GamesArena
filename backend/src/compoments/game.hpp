#pragma once
#include "ostream"
#include "vector"
#include "crow.h"

class Game {
public:
    virtual ~Game() = default;
    virtual void init(const std::string& lobbyId,
                      const std::vector<std::string>& playerIds) = 0;
    virtual void handleMove(const std::string&, const crow::json::rvalue&) = 0;
    virtual crow::json::wvalue getState() const = 0;
    virtual bool isFinished() const = 0;
    virtual void startTurn() { }
    // Add these two:
    virtual std::string serialize() const = 0; // <-- serialize to string (e.g. JSON)
    virtual void deserialize(const std::string& data) = 0; // <-- load from string
};

