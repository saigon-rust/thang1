#ifndef POST_H
#define POST_H

namespace POST {
    void handle_post(int client_fd, const std::string& request, std::string& session_id);
}

#endif // REQUESTHANDLER_H
