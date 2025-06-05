// node.c - UDP Peer Discovery + TCP Block Exchange + Simple File-based Blockchain
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <pthread.h>

#define BASE_PORT 10000
#define MAX_PORT 10010
#define MAX_PEERS 100
#define BUF_SIZE 1024
#define BLOCKCHAIN_FILE "chain.dat"

const char *DISCOVERY_MSG = "DISCOVERY";

typedef struct {
    struct sockaddr_in addr;
} Peer;

Peer peers[MAX_PEERS];
int peer_count = 0;
int my_port;

// Add peer if not known
void add_peer(struct sockaddr_in *addr) {
    for (int i = 0; i < peer_count; ++i) {
        if (peers[i].addr.sin_addr.s_addr == addr->sin_addr.s_addr &&
            peers[i].addr.sin_port == addr->sin_port) return;
    }
    if (peer_count < MAX_PEERS) {
        peers[peer_count++].addr = *addr;
        printf("[+] Added peer %s:%d\n", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
    }
}

// Save block to blockchain file
void save_block(const char *block) {
    FILE *fp = fopen(BLOCKCHAIN_FILE, "a");
    if (fp) {
        fprintf(fp, "%s\n", block);
        fclose(fp);
        printf("[✔] Block saved to file: %s\n", block);
    }
}

// Read blockchain from file
void print_blockchain() {
    FILE *fp = fopen(BLOCKCHAIN_FILE, "r");
    if (!fp) return;
    char line[BUF_SIZE];
    printf("\n📄 Blockchain history:\n");
    while (fgets(line, sizeof(line), fp)) {
        printf(" - %s", line);
    }
    fclose(fp);
}

// Bind to available UDP port
int bind_udp_port(int sockfd, struct sockaddr_in *addr) {
    for (int port = BASE_PORT; port <= MAX_PORT; ++port) {
        addr->sin_family = AF_INET;
        addr->sin_addr.s_addr = INADDR_ANY;
        addr->sin_port = htons(port);
        if (bind(sockfd, (struct sockaddr *)addr, sizeof(*addr)) == 0) return port;
    }
    return -1;
}

// Broadcast discovery message
void broadcast_discovery(int sockfd, int my_port) {
    struct sockaddr_in bcast_addr = {0};
    bcast_addr.sin_family = AF_INET;
    bcast_addr.sin_port = htons(BASE_PORT);
    inet_pton(AF_INET, "255.255.255.255", &bcast_addr.sin_addr);

    for (int port = BASE_PORT; port <= MAX_PORT; ++port) {
        if (port == my_port) continue;
        bcast_addr.sin_port = htons(port);
        sendto(sockfd, DISCOVERY_MSG, strlen(DISCOVERY_MSG), 0,
               (struct sockaddr *)&bcast_addr, sizeof(bcast_addr));
    }
}

// TCP: handle incoming block
void *tcp_server(void *arg) {
    int serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr = {.sin_family = AF_INET, .sin_port = htons(my_port), .sin_addr.s_addr = INADDR_ANY};

    bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    listen(serv_sock, 5);

    while (1) {
        int client_sock = accept(serv_sock, NULL, NULL);
        char buf[BUF_SIZE] = {0};
        int len = read(client_sock, buf, sizeof(buf) - 1);
        if (len > 0) {
            printf("[RECV] Block: %s\n", buf);
            save_block(buf);
        }
        close(client_sock);
    }
}

// TCP: send block to all peers
void broadcast_block(const char *block) {
    for (int i = 0; i < peer_count; ++i) {
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (connect(sockfd, (struct sockaddr *)&peers[i].addr, sizeof(peers[i].addr)) == 0) {
            send(sockfd, block, strlen(block), 0);
        }
        close(sockfd);
    }
    save_block(block);
}

int main() {
    int udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    int yes = 1;
    setsockopt(udp_sock, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes));

    struct sockaddr_in my_addr = {0};
    my_port = bind_udp_port(udp_sock, &my_addr);
    if (my_port == -1) {
        perror("Port bind failed");
        return 1;
    }

    printf("🔗 Node running on UDP/TCP port %d\n", my_port);
    fcntl(udp_sock, F_SETFL, O_NONBLOCK);
    broadcast_discovery(udp_sock, my_port);

    // Start TCP server
    pthread_t tcp_thread;
    pthread_create(&tcp_thread, NULL, tcp_server, NULL);

    // Handle UDP discovery replies
    struct sockaddr_in sender;
    socklen_t slen = sizeof(sender);
    char buf[BUF_SIZE];

    while (1) {
        int len = recvfrom(udp_sock, buf, BUF_SIZE - 1, 0,
                           (struct sockaddr *)&sender, &slen);
        if (len > 0) {
            buf[len] = '\0';
            if (strcmp(buf, DISCOVERY_MSG) == 0) {
                add_peer(&sender);
            }
        }

        // User input
        printf("\n⛏ Enter block data (or 'print'): ");
        fflush(stdout);
        if (fgets(buf, sizeof(buf), stdin)) {
            buf[strcspn(buf, "\n")] = 0;
            if (strcmp(buf, "print") == 0) {
                print_blockchain();
            } else if (strlen(buf) > 0) {
                broadcast_block(buf);
            }
        }
    }

    close(udp_sock);
    return 0;
}
