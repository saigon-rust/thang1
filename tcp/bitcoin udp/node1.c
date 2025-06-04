// node.c - UDP P2P Node with broadcast-based discovery
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <time.h>

#define MAX_PEERS 100
#define BUF_SIZE 1024
#define BROADCAST_PORT 9999
#define BROADCAST_IP "255.255.255.255"
#define DISCOVERY_MSG "DISCOVERY_REQUEST"
#define RESPONSE_MSG "DISCOVERY_RESPONSE"

typedef struct {
    struct sockaddr_in addr;
} Peer;

Peer peers[MAX_PEERS];
int peer_count = 0;

int is_known_peer(struct sockaddr_in *addr) {
    for (int i = 0; i < peer_count; i++) {
        if (peers[i].addr.sin_addr.s_addr == addr->sin_addr.s_addr &&
            peers[i].addr.sin_port == addr->sin_port) {
            return 1;
        }
    }
    return 0;
}

void add_peer(struct sockaddr_in *addr) {
    if (!is_known_peer(addr) && peer_count < MAX_PEERS) {
        peers[peer_count++].addr = *addr;
        printf("[+] New peer added: %s:%d\n", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
    }
}

int main() {
    int sock;
    struct sockaddr_in my_addr, recv_addr;
    char buffer[BUF_SIZE];
    socklen_t addr_len = sizeof(recv_addr);

    // Tạo socket UDP
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    // Cho phép broadcast
    int broadcast_enable = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable));

    // Gán socket vào cổng
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = htons(BROADCAST_PORT);

    if (bind(sock, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0) {
        perror("bind");
        return 1;
    }

    printf("[INFO] Node is running on port %d\n", BROADCAST_PORT);

    // Gửi broadcast để khám phá các node khác
    struct sockaddr_in broadcast_addr;
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(BROADCAST_PORT);
    inet_pton(AF_INET, BROADCAST_IP, &broadcast_addr.sin_addr);

    sendto(sock, DISCOVERY_MSG, strlen(DISCOVERY_MSG), 0,
           (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr));

    // Vòng lặp nhận dữ liệu
    while (1) {
        memset(buffer, 0, BUF_SIZE);
        int recv_len = recvfrom(sock, buffer, BUF_SIZE - 1, 0,
                                (struct sockaddr *)&recv_addr, &addr_len);
        if (recv_len <= 0) continue;
        buffer[recv_len] = '\0';

        if (strcmp(buffer, DISCOVERY_MSG) == 0) {
            // Node khác đang khám phá, trả lời lại
            sendto(sock, RESPONSE_MSG, strlen(RESPONSE_MSG), 0,
                   (struct sockaddr *)&recv_addr, addr_len);
            add_peer(&recv_addr);
        } else if (strcmp(buffer, RESPONSE_MSG) == 0) {
            add_peer(&recv_addr);
        } else {
            printf("[MSG] From %s:%d => %s\n",
                   inet_ntoa(recv_addr.sin_addr), ntohs(recv_addr.sin_port), buffer);
        }

        // Ví dụ: gửi phản hồi đến tất cả peer
        for (int i = 0; i < peer_count; i++) {
            sendto(sock, "Hello from peer", 16, 0,
                   (struct sockaddr *)&peers[i].addr, sizeof(peers[i].addr));
        }
    }

    close(sock);
    return 0;
}
