//handle_session.cpp

#include <iostream>
#include <unordered_map>
#include <string>
#include <random>
#include <sstream>
#include <iomanip>

namespace handle_session {
    std::unordered_map<std::string, std::string> sessionDB;

    std::string setSessionID() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        std::stringstream ss;
        for (int i = 0; i < 16; ++i) {
            ss << std::hex << std::setw(2) << std::setfill('0') << dis(gen);
        }
        return ss.str();
    }

    std::string getSessionID(const std::string &request) {
        size_t pos = request.find("Cookie: ");
        if (pos == std::string::npos) return "";
        
        pos += 8;
        size_t end = request.find("\r\n", pos);
        std::string cookie_header = request.substr(pos, end - pos);
        
        size_t session_pos = cookie_header.find("SessionID=");
        if (session_pos == std::string::npos) return "";

        session_pos += 10;
        size_t session_end = cookie_header.find(";", session_pos);
        
        if (session_end == std::string::npos) session_end = cookie_header.size(); 
        return cookie_header.substr(session_pos, session_end - session_pos);
    }

    void removeSession(const std::string &session_id) {
        sessionDB.erase(session_id);
    }
    
    bool isValidSession(const std::string &session_id) {
        return !session_id.empty() && sessionDB.find(session_id) != sessionDB.end();
    }
}
