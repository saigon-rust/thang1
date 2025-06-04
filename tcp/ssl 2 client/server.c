#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 12345
#define MAX_CLIENTS 2
#define BUFFER_SIZE 1024

int clients[MAX_CLIENTS];       // Mảng lưu socket 2 client
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// Gửi message cho client chỉ định
void send_message(int sockfd, const char *msg, size_t len) {
    send(sockfd, msg, len, 0);
}

// Hàm xử lý client trong thread
void *handle_client(void *arg) {
    int idx = *(int *)arg;
    int sockfd = clients[idx];
    char buffer[BUFFER_SIZE];

    printf("Client %d connected.\n", idx + 1);

    while (1) {
        ssize_t n = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) {
            printf("Client %d disconnected.\n", idx + 1);
            break;
        }
        buffer[n] = '\0';

        // Gửi message tới client còn lại
        int other_idx = (idx == 0) ? 1 : 0;

        pthread_mutex_lock(&clients_mutex);
        if (clients[other_idx] != 0) {
            send_message(clients[other_idx], buffer, n);
        }
        pthread_mutex_unlock(&clients_mutex);
    }

    close(sockfd);
    pthread_mutex_lock(&clients_mutex);
    clients[idx] = 0;  // Đánh dấu client này đã rời
    pthread_mutex_unlock(&clients_mutex);
    free(arg);
    return NULL;
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    pthread_t tid;

    // Khởi tạo mảng clients về 0 (chưa có client)
    memset(clients, 0, sizeof(clients));

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    memset(server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_fd);
        exit(1);
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen");
        close(server_fd);
        exit(1);
    }

    printf("Server listening on port %d. Waiting for %d clients...\n", PORT, MAX_CLIENTS);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        pthread_mutex_lock(&clients_mutex);

        // Tìm vị trí trống để lưu client
        int i;
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i] == 0) {
                clients[i] = client_fd;
                break;
            }
        }

        pthread_mutex_unlock(&clients_mutex);

        if (i == MAX_CLIENTS) {
            // Đã đầy 2 client, từ chối
            char *msg = "Server full, try later.\n";
            send(client_fd, msg, strlen(msg), 0);
            close(client_fd);
        } else {
            int *pclient_idx = malloc(sizeof(int));
            *pclient_idx = i;
            pthread_create(&tid, NULL, handle_client, pclient_idx);
            pthread_detach(tid);
        }
    }

    close(server_fd);
    return 0;
}
