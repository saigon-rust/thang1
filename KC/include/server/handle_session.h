#ifndef HANDLE_SESSION_H
#define HANDLE_SESSION_H

#include <unordered_map>

namespace handle_session {

    extern std::unordered_map<std::string, std::string> sessionDB;
    
    std::string setSessionID();

    std::string getSessionID(const std::string &request);

    void removeSession(const std::string &session_id);

    bool isValidSession(const std::string &session_id);
} // namespace session

#endif // HANDLE_SESSION_H
