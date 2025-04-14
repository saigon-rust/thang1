#include <iostream>
#include <string>

#include <server/handle_get.h>

#include <server/handle_session.h>
#include <server/handle_response.h>


namespace GET {
    void handle_get(int client_fd, const std::string& request, std::string& session_id) {
        // Extract the path from the GET request
        if (request.rfind("GET /", 0) != 0) {
            std::cout << "Not a GET request or doesn't start with 'GET /'." << std::endl;
            handle_response::send_response(client_fd, R"({"status": "error", "message": "Invalid GET request"})", "application/json", session_id);
            return;
        }

        size_t start = 4;
        size_t end = request.find(" ", start);
        if (end == std::string::npos) {
            std::cout << "Invalid request format (no space after path)." << std::endl;
            handle_response::send_response(client_fd, R"({"status": "error", "message": "Invalid request format"})", "application/json", session_id);
            return;
        }

        std::string path = request.substr(start, end - start);
        std::cout << "Extracted path: " << path << std::endl;

        if (path == "/") {
            handle_response::serve_file(client_fd, "../../frontend/index.html", session_id);
        }   
    }
}
