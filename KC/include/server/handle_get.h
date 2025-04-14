#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H

namespace GET {
  void handle_get(int client_fd, const std::string& request, std::string& session_id);
}

#endif // REQUESTHANDLER_H
