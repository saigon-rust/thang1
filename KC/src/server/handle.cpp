// handle.cpp

#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <sstream>

#include <server/handle.h>
#include <server/handle_session.h>
#include <server/handle_response.h>
#include <server/handle_get.h>
#include <server/handle_post.h>


#define BUFFER_SIZE 4096

namespace handle {
    void handle_request(int client_fd) {
        char buffer[BUFFER_SIZE] = {0};
        ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);

        if (bytes_read == -1) {
            std::cerr << "Error reading from client socket." << std::endl;
            close(client_fd);
            return;
        } else if (bytes_read == 0) {
            std::cout << "Client disconnected." << std::endl;
            close(client_fd);
            return;
        }

        std::string request(buffer, bytes_read);
        std::istringstream iss(request);
        std::string method;
        iss >> method;

        std::string session_id = handle_session::getSessionID(request);

        if (method == "GET") {
            GET::handle_get(client_fd, request, session_id);
        } else if (method == "POST") {
            POST::handle_post(client_fd, request, session_id);
        } else {
            handle_response::send_response(client_fd, R"({"status": "error", "message": "Method Not Allowed"})", "application/json", session_id);
        }

        close(client_fd);
    }
}
