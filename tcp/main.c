#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    char *hello = "Hello Client";

    // 1. Tạo socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    printf("Server listening on port %d...\n", PORT);

    while (1) {
        printf("\nWaiting for a new connection...\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
            perror("accept");
            continue;
        }
        printf("Connection accepted from %s:%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t valread = recv(new_socket, buffer, BUFFER_SIZE - 1, 0);

        if (valread < 0) {
            perror("recv failed");
        } else if (valread == 0) {
            printf("Client disconnected.\n");
        } else {
            buffer[valread] = '\0';
            printf("Client: %s\n", buffer);
        }

        // 6. Gửi dữ liệu cho client (Send)
        send(new_socket, hello, strlen(hello), 0);
        printf("Server: Message '%s' sent to client\n", hello);

        // 7. Đóng socket kết nối với client
        close(new_socket);
        printf("Connection with client %s:%d closed.\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
    }
}