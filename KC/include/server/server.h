// server.h

#ifndef SERVER_H
#define SERVER_H

namespace server {
    class WebServer {
    public:
        explicit WebServer(int port);
        ~WebServer();
        void start();
    private:
        int server_fd;
        int port;
        static void* handle_client_thread(void* arg);
    };
}

#endif // SERVER_H

