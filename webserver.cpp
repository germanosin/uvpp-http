#include <iostream>
#include "http/server.hpp"

using namespace uvpp::http;

int main() {
    Server server;
    int port = 8080;
    if(!server.listen("0.0.0.0", port, [](Request& req, Response& res) {
        std::string body = req.get_body(); // Now you can write a custom handler for the body content.
        res.set_status(200);
        res.set_header("Content-Type", "text/plain");
        res.end("C++ FTW\n");
    })) return 1; // Failed to run server.

    std::cout << "Server running at http://0.0.0.0:" << port << "/" << std::endl;
    return uvpp::run();
}