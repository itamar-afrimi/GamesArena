struct CORS {
    struct context {};
    // need to change "FRONTEND_URL" to local URL when testing locally
    void before_handle(crow::request& req, crow::response& res, context&) {
        
        std::string origin = req.get_header_value("Origin");
        
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
        // Handle CORS headers for the response
        std::string origin = req.get_header_value("Origin");

        res.set_header("Access-Control-Allow-Origin", origin);
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
    }
};
