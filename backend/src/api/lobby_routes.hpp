#pragma once
#include "crow.h"
#include "../cors_middleware.h"
#include "../services/online_service.hpp"

void register_lobby_routes(crow::App<CORS>& app, OnlineService& onlineService);
