#pragma once
#include "crow.h"
#include "../cors_middleware.h"
#include <crow/websocket.h>
#include "../services/session_service.hpp"

void register_ws_game(crow::App<CORS>& app, SessionService& sessionService);
