#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h> // Để kiểm tra lỗi
#include <pthread.h> // Để lắng nghe UDP trong một luồng riêng

#define SERVER_IP "127.0.0.1" // Địa chỉ IP của Signaling Server (thay đổi nếu server ở máy khác)
#define SERVER_PORT 8888
#define UDP_LOCAL_PORT 0 // 0 để hệ điều hành tự chọn port ngẫu nhiên
#define BUFFER_SIZE 256
#define P2P_PING_INTERVAL 1 // Giây
#define MAX_P2P_ATTEMPTS 20 // Số lần thử đục lỗ NAT

struct sockaddr_in peer_addr;
socklen_t peer_addr_len = sizeof(peer_addr);
int udp_sock_fd;
int p2p_established = 0; // Cờ hiệu để kiểm tra kết nối P2P

// Hàm lắng nghe gói tin UDP đến
void* udp_listener(void* arg) {
    char buffer[BUFFER_SIZE];
    while (1) {
        ssize_t bytes_received = recvfrom(udp_sock_fd, buffer, sizeof(buffer) - 1, 0,
                                         (struct sockaddr*)&peer_addr, &peer_addr_len);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            printf("[P2P Received]: %s\n", buffer);
            if (strcmp(buffer, "SYN_P2P") == 0 && !p2p_established) {
                printf("[P2P] Received SYN_P2P. Sending ACK_P2P...\n");
                sendto(udp_sock_fd, "ACK_P2P", strlen("ACK_P2P"), 0, (struct sockaddr*)&peer_addr, peer_addr_len);
                p2p_established = 1;
            } else if (strcmp(buffer, "ACK_P2P") == 0 && !p2p_established) {
                printf("[P2P] Received ACK_P2P. P2P connection established!\n");
                p2p_established = 1;
            } else if (p2p_established) {
                // Nếu đã thiết lập P2P, in tin nhắn từ peer
                printf("[P2P Chat]: %s\n", buffer);
            }
        }
    }
    return NULL;
}

int main() {
    int sock_fd_tcp;
    struct sockaddr_in server_addr_tcp, local_udp_addr;
    char buffer[BUFFER_SIZE];

    // --- 1. Kết nối TCP với Signaling Server ---
    sock_fd_tcp = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd_tcp == -1) {
        perror("Socket TCP creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr_tcp.sin_family = AF_INET;
    server_addr_tcp.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr_tcp.sin_addr);

    if (connect(sock_fd_tcp, (struct sockaddr*)&server_addr_tcp, sizeof(server_addr_tcp)) < 0) {
        perror("TCP connection to Signaling Server failed");
        exit(EXIT_FAILURE);
    }
    printf("Connected to Signaling Server.\n");

    // --- 2. Tạo và bind socket UDP cục bộ ---
    udp_sock_fd = socket(AF_INET, SOCK_DGRAM, 0); // SOCK_DGRAM cho UDP
    if (udp_sock_fd == -1) {
        perror("Socket UDP creation failed");
        close(sock_fd_tcp);
        exit(EXIT_FAILURE);
    }

    local_udp_addr.sin_family = AF_INET;
    local_udp_addr.sin_addr.s_addr = INADDR_ANY; // Bind với IP cục bộ của máy
    local_udp_addr.sin_port = htons(UDP_LOCAL_PORT); // Để OS chọn port ngẫu nhiên

    if (bind(udp_sock_fd, (struct sockaddr*)&local_udp_addr, sizeof(local_udp_addr)) < 0) {
        perror("UDP Bind failed");
        close(sock_fd_tcp);
        close(udp_sock_fd);
        exit(EXIT_FAILURE);
    }

    // Lấy thông tin port UDP đã được bind
    socklen_t local_udp_addr_len = sizeof(local_udp_addr);
    getsockname(udp_sock_fd, (struct sockaddr*)&local_udp_addr, &local_udp_addr_len);
    printf("Local UDP socket bound to port: %d\n", ntohs(local_udp_addr.sin_port));

    // --- 3. Gửi thông tin UDP của mình cho Signaling Server ---
    // Trong một hệ thống thực, bạn sẽ gửi IP công cộng và port đã qua STUN ở đây.
    // Đối với ví dụ đơn giản này, chúng ta giả sử IP của server sẽ là IP công cộng của client
    // nếu chúng ở cùng mạng, hoặc bạn phải biết IP công cộng của mình.
    // Để đơn giản, tôi dùng IP của Signaling Server và port UDP cục bộ làm giả định.
    // Hoặc nếu bạn chạy trên máy cục bộ, bạn có thể gửi 127.0.0.1:<port>
    // Để có IP thật: bạn cần một STUN client để khám phá IP công cộng của mình.
    struct sockaddr_in my_public_tcp_addr;
    socklen_t my_public_tcp_addr_len = sizeof(my_public_tcp_addr);
    getsockname(sock_fd_tcp, (struct sockaddr*)&my_public_tcp_addr, &my_public_tcp_addr_len);

    char my_udp_info[BUFFER_SIZE];
    snprintf(my_udp_info, sizeof(my_udp_info), "%s:%d",
             inet_ntoa(my_public_tcp_addr.sin_addr), // Giả định IP công cộng là IP kết nối đến server
             ntohs(local_udp_addr.sin_port));

    printf("Sending my UDP info to server: %s\n", my_udp_info);
    send(sock_fd_tcp, my_udp_info, strlen(my_udp_info), 0);

    // --- 4. Nhận thông tin UDP của peer từ Signaling Server ---
    ssize_t bytes_read = recv(sock_fd_tcp, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        printf("Received peer UDP info from server: %s\n", buffer);

        // Phân tích cú pháp thông tin peer
        char* peer_ip_str = strtok(buffer, ":");
        char* peer_port_str = strtok(NULL, ":");
        if (peer_ip_str && peer_port_str) {
            peer_addr.sin_family = AF_INET;
            inet_pton(AF_INET, peer_ip_str, &peer_addr.sin_addr);
            peer_addr.sin_port = htons(atoi(peer_port_str));
            printf("Peer found: %s:%d\n", inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port));
        } else {
            fprintf(stderr, "Invalid peer info received.\n");
            close(sock_fd_tcp);
            close(udp_sock_fd);
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "Failed to receive peer info from server.\n");
        close(sock_fd_tcp);
        close(udp_sock_fd);
        exit(EXIT_FAILURE);
    }

    close(sock_fd_tcp); // Đóng kết nối TCP với server sau khi đã trao đổi thông tin

    // --- 5. Bắt đầu lắng nghe UDP trong một luồng riêng ---
    pthread_t listener_tid;
    pthread_create(&listener_tid, NULL, udp_listener, NULL);
    pthread_detach(listener_tid);

    // --- 6. Thực hiện UDP Hole Punching ---
    printf("\nAttempting UDP Hole Punching...\n");
    for (int i = 0; i < MAX_P2P_ATTEMPTS && !p2p_established; ++i) {
        printf("Sending P2P ping to %s:%d (Attempt %d/%d)...\n",
               inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port), i + 1, MAX_P2P_ATTEMPTS);
        sendto(udp_sock_fd, "SYN_P2P", strlen("SYN_P2P"), 0, (struct sockaddr*)&peer_addr, peer_addr_len);
        sleep(P2P_PING_INTERVAL); // Chờ một chút trước khi gửi lại
    }

    if (!p2p_established) {
        printf("P2P connection not established directly after %d attempts. Might need TURN server or different NAT type.\n", MAX_P2P_ATTEMPTS);
        printf("Continuing to listen for P2P connection...\n");
    } else {
        printf("P2P connection established! You can now type messages to your peer.\n");
    }

    // --- 7. Giao tiếp P2P trực tiếp qua UDP ---
    char chat_message[BUFFER_SIZE];
    while (1) {
        printf("You: ");
        if (fgets(chat_message, sizeof(chat_message), stdin) != NULL) {
            chat_message[strcspn(chat_message, "\n")] = '\0'; // Xóa ký tự xuống dòng
            if (strlen(chat_message) > 0) {
                sendto(udp_sock_fd, chat_message, strlen(chat_message), 0, (struct sockaddr*)&peer_addr, peer_addr_len);
            }
        }
    }

    close(udp_sock_fd);
    return 0;
}