// server.cpp
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <server/handle.h>
#include <server/server.h>

namespace server {
    WebServer::WebServer(int port) : port(port), server_fd(0) {}
    WebServer::~WebServer() {
        if (server_fd > 0) close(server_fd);
    }

    void WebServer::start() {
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == -1) {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
        }

        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
            perror("Bind failed");
            exit(EXIT_FAILURE);
        }

        if (listen(server_fd, 10) < 0) {
            perror("Listen failed");
            exit(EXIT_FAILURE);
        }

        std::cout << "Server listening on port " << port << std::endl;

        while (true) {
            int client_fd = accept(server_fd, nullptr, nullptr);
            if (client_fd < 0) continue;

            pthread_t thread_id;
            int* client_sock = new int(client_fd);
            pthread_create(&thread_id, nullptr, handle_client_thread, client_sock);
            pthread_detach(thread_id);
        }
    }

    void* WebServer::handle_client_thread(void* arg) {
        int client_fd = *(int*)arg;
        delete (int*)arg;
        handle::handle_request(client_fd);
        return nullptr;
    }
}

