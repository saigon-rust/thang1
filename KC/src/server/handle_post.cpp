#include <iostream>
#include <string>

#include <server/handle_post.h>
#include <server/handle_session.h>
#include <server/handle_response.h>


namespace POST {
    void handle_post(int client_fd, const std::string& request, std::string& session_id) {
        if (request.find("POST /") != std::string::npos) {
            return;
        }
    }
}
