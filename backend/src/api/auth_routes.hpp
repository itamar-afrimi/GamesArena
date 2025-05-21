#pragma once
#include "crow.h"
#include "../services/user_services.hpp"
#include "../services/online_service.hpp"
#include "../cors_middleware.h"
void register_auth_routes(crow::App<CORS>& app, UserService& userService, OnlineService& onlineService);
