// node.c - UDP P2P Node with Broadcast Discovery + Simple Consensus
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <time.h>

#define MAX_PEERS 100
#define BUF_SIZE 1024
#define BASE_PORT 10000
#define MAX_PORT 10010
#define DISCOVERY_MSG "DISCOVERY_REQUEST"
#define RESPONSE_MSG "DISCOVERY_RESPONSE"
#define HELLO_MSG "HELLO"
#define BLOCK_PREFIX "BLOCK:"
#define ACCEPT_PREFIX "ACCEPT:"

typedef struct {
    struct sockaddr_in addr;
} Peer;

Peer peers[MAX_PEERS];
int peer_count = 0;

char current_block[BUF_SIZE] = {0};
int accept_count = 0;
int my_port = -1;

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

void send_to_all_ports(int sock, const char *msg, int exclude_port) {
    for (int port = BASE_PORT; port <= MAX_PORT; port++) {
        if (port == exclude_port) continue;
        struct sockaddr_in peer_addr = {0};
        peer_addr.sin_family = AF_INET;
        peer_addr.sin_port = htons(port);
        inet_pton(AF_INET, "127.0.0.1", &peer_addr.sin_addr);
        sendto(sock, msg, strlen(msg), 0, (struct sockaddr *)&peer_addr, sizeof(peer_addr));
    }
}

void propose_block(int sock, const char *block_data) {
    snprintf(current_block, sizeof(current_block), "%s", block_data);
    accept_count = 0;

    char msg[BUF_SIZE];
    snprintf(msg, sizeof(msg), "%s%s", BLOCK_PREFIX, block_data);
    send_to_all_ports(sock, msg, my_port);

    printf("[PROPOSE] Sent block proposal: %s\n", block_data);
}

void handle_message(int sock, char *msg, struct sockaddr_in *sender) {
    if (strncmp(msg, BLOCK_PREFIX, strlen(BLOCK_PREFIX)) == 0) {
        char reply[BUF_SIZE];
        const char *block_data = msg + strlen(BLOCK_PREFIX);
        snprintf(reply, sizeof(reply), "%s%s", ACCEPT_PREFIX, block_data);
        sendto(sock, reply, strlen(reply), 0, (struct sockaddr *)sender, sizeof(*sender));
        add_peer(sender);
        printf("[RECV] Block proposal from %s:%d - %s\n", inet_ntoa(sender->sin_addr), ntohs(sender->sin_port), block_data);
    }
    else if (strncmp(msg, ACCEPT_PREFIX, strlen(ACCEPT_PREFIX)) == 0) {
        const char *accepted_block = msg + strlen(ACCEPT_PREFIX);
        if (strcmp(accepted_block, current_block) == 0) {
            accept_count++;
            printf("[ACCEPT] Block '%s' accepted by %s:%d (total accepts: %d)\n",
                   accepted_block, inet_ntoa(sender->sin_addr), ntohs(sender->sin_port), accept_count);
            if (accept_count > peer_count / 2) {
                printf("[CONSENSUS] Block '%s' đạt đồng thuận 🎉\n", current_block);
                current_block[0] = 0;
                accept_count = 0;
            }
        }
    }
    else if (strcmp(msg, DISCOVERY_MSG) == 0) {
        sendto(sock, RESPONSE_MSG, strlen(RESPONSE_MSG), 0,
               (struct sockaddr *)sender, sizeof(*sender));
        add_peer(sender);
    }
    else if (strcmp(msg, RESPONSE_MSG) == 0 || strcmp(msg, HELLO_MSG) == 0) {
        add_peer(sender);
    }
    else {
        printf("[MSG] From %s:%d => %s\n",
               inet_ntoa(sender->sin_addr), ntohs(sender->sin_port), msg);
    }
}

int bind_available_port(int sock) {
    struct sockaddr_in addr;
    for (int port = BASE_PORT; port <= MAX_PORT; port++) {
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);

        if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
            return port;
        }
    }
    return -1;
}

int main() {
    int sock;
    struct sockaddr_in recv_addr;
    socklen_t addr_len = sizeof(recv_addr);
    char buffer[BUF_SIZE];

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    int yes = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes));

    my_port = bind_available_port(sock);
    if (my_port == -1) {
        fprintf(stderr, "Không tìm được cổng khả dụng trong khoảng %d - %d\n", BASE_PORT, MAX_PORT);
        return 1;
    }

    printf("🧩 Node is running on UDP port %d. Type 'send <data>' to propose block.\n", my_port);

    // Khám phá các node khác
    send_to_all_ports(sock, DISCOVERY_MSG, my_port);
    send_to_all_ports(sock, HELLO_MSG, my_port);

    fcntl(sock, F_SETFL, O_NONBLOCK);

    while (1) {
        int recv_len = recvfrom(sock, buffer, BUF_SIZE - 1, 0,
                                (struct sockaddr *)&recv_addr, &addr_len);
        if (recv_len > 0) {
            buffer[recv_len] = '\0';
            handle_message(sock, buffer, &recv_addr);
        }

        if (fgets(buffer, sizeof(buffer), stdin)) {
            if (strncmp(buffer, "send ", 5) == 0) {
                char *block_data = buffer + 5;
                block_data[strcspn(block_data, "\n")] = 0;
                propose_block(sock, block_data);
            }
        }

        usleep(100000);
    }

    close(sock);
    return 0;
}
