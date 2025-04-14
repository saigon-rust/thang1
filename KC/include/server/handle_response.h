#ifndef HANDLE_RESPONSE_H
#define HANDLE_RESPONSE_H

namespace handle_response {
    void send_response(int client_fd, const std::string &content, const std::string &content_type,
                       const std::string &session_id = "", const std::string &extra_headers = "");

    std::string get_content_type(const std::string &filename);

    void serve_file(int client_fd, const std::string &filename, const std::string &session_id = "");

} // namespace response

#endif // HANDLE_RESPONSE_H
