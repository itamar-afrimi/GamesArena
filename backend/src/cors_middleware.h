struct CORS {
    struct context {};
    // need to change "FRONTEND_URL" to local URL when testing locally
    void before_handle(crow::request& req, crow::response& res, context&) {
        //res.set_header("Access-Control-Allow-Origin", std::getenv("FRONTEND_URL"));
        res.set_header("Access-Control-Allow-Origin", "http://localhost:5173");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");

        // Respond immediately to preflight (OPTIONS) requests
        if (req.method == crow::HTTPMethod::Options) {
            res.code = 204;
            res.end();
        }
    }

    void after_handle(crow::request&, crow::response& res, context&) {
        // Do NOT add headers again here
        //res.set_header("Access-Control-Allow-Origin", std::getenv("FRONTEND_URL"));
        res.set_header("Access-Control-Allow-Origin", "http://localhost:5173");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
    }
};
