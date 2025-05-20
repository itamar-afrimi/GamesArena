struct CORS {
    struct context {};
    // need to change "FRONTEND_URL" to local URL when testing locally
    void before_handle(crow::request& req, crow::response& res, context&) {
        //res.set_header("Access-Control-Allow-Origin", std::getenv("FRONTEND_URL"));
        
        std::string origin = req.get_header_value("Origin");
        // ctx.origin = origin;
        // if (origin == "http://localhost:5173/" || origin == "https://games-arena-4bfe.vercel.app") {
        //     res.set_header("Access-Control-Allow-Origin", origin);
        // }
        res.set_header("Access-Control-Allow-Origin", origin);
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");

        // Respond immediately to preflight (OPTIONS) requests
        if (req.method == crow::HTTPMethod::Options) {
            res.code = 204;
            res.end();
        }
    }

    void after_handle(crow::request& req, crow::response& res, context&) {
        // Do NOT add headers again here
        // std::cout << "---- Incoming Request Headers ----" << std::endl;
        // std::cout << "origin: " << ctx.origin << std::endl;
        // // for (const auto& header : req.headers) {
        // //     std::cout << header.first << ": " << header.second << std::endl;
        // // }
        // std::cout << "----------------------------------" << std::endl;
        // std::string origin = req.get_header_value("Origin");
        auto it = res.headers.find("X-Set-Origin");
        if (it != res.headers.end()) {
            std::cout << it->second << std::endl;
            std::string origin = it->second;
            res.set_header("Access-Control-Allow-Origin", origin);
            res.headers.erase(it); // Remove custom header
        }
        
        res.set_header("Access-Control-Allow-Origin", "https://games-arena-4bfe.vercel.app");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
    }
};
