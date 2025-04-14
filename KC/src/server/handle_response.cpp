//handle_response.cpp

#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <vector>

namespace handle_response {
    void send_response(int client_fd, const std::string &content, const std::string &content_type, 
                       const std::string &session_id = "", const std::string &extra_headers = "") {
        std::ostringstream response;
        response << "HTTP/1.1 200 OK\r\n"
                 << "Content-Type: " << content_type << "\r\n"
                 << "Content-Length: " << content.size() << "\r\n";
        
        if (!session_id.empty()) {
            response << "Set-Cookie: SessionID=" << session_id << "; Path=/; HttpOnly\r\n";
        }
        
        if (!extra_headers.empty()) {
            response << extra_headers;
        }

        response << "Connection: close\r\n\r\n" << content;
        std::string response_str = response.str();
        write(client_fd, response_str.c_str(), response_str.size());
    }

    std::string get_content_type(const std::string &filename) {
        static const std::unordered_map<std::string, std::string> content_types = {
            {".html", "text/html"}, {".css", "text/css"}, {".js", "application/javascript"},
            {".jpg", "image/jpeg"}, {".jpeg", "image/jpeg"}, {".png", "image/png"},
            {".gif", "image/gif"}, {".txt", "text/plain"}, {".json", "application/json"},
            {".pdf", "application/pdf"}, {".ico", "image/x-icon"}
        };
        size_t dot_pos = filename.rfind('.');
        return (dot_pos != std::string::npos && content_types.count(filename.substr(dot_pos))) 
               ? content_types.at(filename.substr(dot_pos)) 
               : "application/octet-stream";
    }
    
    void serve_file(int client_fd, const std::string &filename, const std::string &session_id) {
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            send_response(client_fd, "<h1>404 File Not Found</h1>", "text/html", session_id);
            return;
        }

        file.seekg(0, std::ios::end);
        size_t file_size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<char> buffer(file_size);
        file.read(buffer.data(), file_size);
        
        send_response(client_fd, std::string(buffer.begin(), buffer.end()), get_content_type(filename), session_id);
    }
}
