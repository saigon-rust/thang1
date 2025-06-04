// node_auto_port.c - UDP P2P Node with auto port binding and peer discovery
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_PEERS 100
#define BUF_SIZE 1024
#define PORT_START 9999
#define PORT_END 10010

#define DISCOVERY_MSG "DISCOVERY_REQUEST"
#define RESPONSE_MSG "DISCOVERY_RESPONSE"
#define HELLO_MSG "hello"

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

int bind_auto_port(int sock) {
    struct sockaddr_in addr;
    for (int port = PORT_START; port <= PORT_END; port++) {
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);
        if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
            return port;
        }
        if (errno != EADDRINUSE) {
            perror("bind");
            return -1;
        }
    }
    return -1; // Không bind được port nào
}

void send_to_all_ports(int sock, const char *msg, int my_port) {
    for (int port = PORT_START; port <= PORT_END; port++) {
        if (port == my_port) continue;
        struct sockaddr_in peer_addr = {0};
        peer_addr.sin_family = AF_INET;
        peer_addr.sin_port = htons(port);
        inet_pton(AF_INET, "127.0.0.1", &peer_addr.sin_addr);

        sendto(sock, msg, strlen(msg), 0, (struct sockaddr*)&peer_addr, sizeof(peer_addr));
    }
}

int main() {
    int sock;
    struct sockaddr_in recv_addr;
    char buffer[BUF_SIZE];
    socklen_t addr_len = sizeof(recv_addr);

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    int broadcast_enable = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable));

    int my_port = bind_auto_port(sock);
    if (my_port < 0) {
        fprintf(stderr, "Failed to bind any port in range %d-%d\n", PORT_START, PORT_END);
        close(sock);
        return 1;
    }

    printf("[INFO] Node is running on UDP port %d\n", my_port);

    // Gửi DISCOVERY_REQUEST và hello đến tất cả port khác
    send_to_all_ports(sock, DISCOVERY_MSG, my_port);
    send_to_all_ports(sock, HELLO_MSG, my_port);

    while (1) {
        int recv_len = recvfrom(sock, buffer, BUF_SIZE - 1, 0,
                                (struct sockaddr *)&recv_addr, &addr_len);
        if (recv_len <= 0) continue;

        buffer[recv_len] = '\0';

        if (strcmp(buffer, DISCOVERY_MSG) == 0) {
            // Trả lời DISCOVERY_REQUEST
            sendto(sock, RESPONSE_MSG, strlen(RESPONSE_MSG), 0,
                   (struct sockaddr *)&recv_addr, addr_len);
            add_peer(&recv_addr);
        }
        else if (strcmp(buffer, RESPONSE_MSG) == 0) {
            add_peer(&recv_addr);
        }
        else if (strcmp(buffer, HELLO_MSG) == 0) {
            add_peer(&recv_addr);
        }
        else {
            printf("[MSG] From %s:%d => %s\n",
                   inet_ntoa(recv_addr.sin_addr), ntohs(recv_addr.sin_port), buffer);
        }
    }

    close(sock);
    return 0;
}
