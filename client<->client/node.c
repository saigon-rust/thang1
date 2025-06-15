#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

typedef struct {
    int sockfd;
    struct sockaddr_in addr;
} Peer;

Peer peers[MAX_CLIENTS];
int peer_count = 0;
pthread_mutex_t peer_lock = PTHREAD_MUTEX_INITIALIZER;

// Gửi tin nhắn đến tất cả peer
void broadcast_message(const char *message) {
    pthread_mutex_lock(&peer_lock);
    for (int i = 0; i < peer_count; ++i) {
        send(peers[i].sockfd, message, strlen(message), 0);
    }
    pthread_mutex_unlock(&peer_lock);
}

// Gửi tin đến tất cả peer ngoại trừ 1 socket
void forward_message(const char *message, int exclude_sock) {
    pthread_mutex_lock(&peer_lock);
    for (int i = 0; i < peer_count; ++i) {
        if (peers[i].sockfd != exclude_sock) {
            send(peers[i].sockfd, message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&peer_lock);
}

// Nhận tin nhắn từ peer
void *receive_from_peer(void *arg) {
    int sock = *(int *)arg;
    free(arg);
    char buffer[BUFFER_SIZE];

    while (1) {
        int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) break;
        buffer[bytes] = '\0';

        printf("[Received] %s\n", buffer);
        forward_message(buffer, sock);  // forward cho peer khác
    }

    close(sock);
    return NULL;
}

// Server thread: lắng nghe các kết nối đến
void *server_thread(void *arg) {
    int listen_port = *(int *)arg;
    free(arg);

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(listen_port),
        .sin_addr.s_addr = INADDR_ANY
    };

    bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_sock, MAX_CLIENTS);

    printf("Server listening on port %d...\n", listen_port);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_len);

        pthread_mutex_lock(&peer_lock);
        if (peer_count < MAX_CLIENTS) {
            peers[peer_count].sockfd = client_sock;
            peers[peer_count].addr = client_addr;
            peer_count++;
        }
        pthread_mutex_unlock(&peer_lock);

        int *sock_ptr = malloc(sizeof(int));
        *sock_ptr = client_sock;
        pthread_t tid;
        pthread_create(&tid, NULL, receive_from_peer, sock_ptr);
        pthread_detach(tid);
    }

    return NULL;
}

// Kết nối đến một peer
void connect_to_peer(const char *ip, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in peer_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = inet_addr(ip)
    };

    if (connect(sock, (struct sockaddr *)&peer_addr, sizeof(peer_addr)) < 0) {
        perror("Connect failed");
        return;
    }

    printf("Connected to peer %s:%d\n", ip, port);

    // Bắt đầu nhận tin ngay lập tức
    int *sock_ptr = malloc(sizeof(int));
    *sock_ptr = sock;
    pthread_t tid;
    pthread_create(&tid, NULL, receive_from_peer, sock_ptr);
    pthread_detach(tid);

    // Ghi nhận peer
    pthread_mutex_lock(&peer_lock);
    if (peer_count < MAX_CLIENTS) {
        peers[peer_count].sockfd = sock;
        peers[peer_count].addr = peer_addr;
        peer_count++;
    } else {
        printf("Peer list full\n");
    }
    pthread_mutex_unlock(&peer_lock);
}


// Gửi tin từ người dùng
void *input_thread(void *arg) {
    char message[BUFFER_SIZE];
    while (fgets(message, sizeof(message), stdin)) {
        broadcast_message(message);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <listen_port> [peer_ip peer_port]\n", argv[0]);
        return 1;
    }

    int *listen_port = malloc(sizeof(int));
    *listen_port = atoi(argv[1]);

    pthread_t server_tid;
    pthread_create(&server_tid, NULL, server_thread, listen_port);

    // Nếu có peer
    if (argc == 4) {
        const char *peer_ip = argv[2];
        int peer_port = atoi(argv[3]);
        connect_to_peer(peer_ip, peer_port);
    }

    pthread_t input_tid;
    pthread_create(&input_tid, NULL, input_thread, NULL);

    pthread_join(server_tid, NULL);
    pthread_join(input_tid, NULL);
    return 0;
}
