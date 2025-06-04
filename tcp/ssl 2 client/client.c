#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345
#define BUFFER_SIZE 1024

int sockfd;

// Thread nhận message từ server và in ra màn hình
void *recv_thread(void *arg) {
    char buffer[BUFFER_SIZE];
    while (1) {
        ssize_t n = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) {
            printf("Disconnected from server.\n");
            exit(0);
        }
        buffer[n] = '\0';
        printf("\nMessage from other client: %s\n> ", buffer);
        fflush(stdout);
    }
}

int main() {
    struct sockaddr_in server_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        return 1;
    }

    printf("Connected to server. You can start chatting!\n");

    pthread_t tid;
    pthread_create(&tid, NULL, recv_thread, NULL);

    // Đọc input từ bàn phím, gửi cho server
    char buffer[BUFFER_SIZE];
    while (1) {
        printf("> ");
        fflush(stdout);
        if (!fgets(buffer, sizeof(buffer), stdin)) break;

        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') buffer[len - 1] = '\0'; // Xóa newline

        if (send(sockfd, buffer, strlen(buffer), 0) < 0) {
            perror("send");
            break;
        }
    }

    close(sockfd);
    return 0;
}
