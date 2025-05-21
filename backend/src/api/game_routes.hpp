#pragma once
#include "crow.h"
#include "../services/session_service.hpp"
#include "../utils/id_generator.hpp"
#include "../compoments/game_manager.hpp"
#include "../cors_middleware.h"

void register_game_routes(crow::App<CORS>& app, SessionService& sessionService, GameManager& gameManager);
