#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h> // Dùng để xử lý nhiều client đồng thời

#define SERVER_PORT 8888
#define MAX_CLIENTS 2 // Chỉ hỗ trợ 2 client cho ví dụ P2P này
#define BUFFER_SIZE 256

// Cấu trúc để lưu thông tin về một client
typedef struct {
    struct sockaddr_in addr; // Địa chỉ TCP/UDP của client
    socklen_t addr_len;      // Độ dài địa chỉ
    int sock_fd;             // File descriptor của socket TCP kết nối với server
} ClientInfo;

ClientInfo clients[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t clients_ready_cond = PTHREAD_COND_INITIALIZER;

void* handle_client(void* arg) {
    int client_fd = *(int*)arg;
    struct sockaddr_in client_addr_tcp;
    socklen_t client_addr_len_tcp = sizeof(client_addr_tcp);
    char buffer[BUFFER_SIZE];
    int read_bytes;

    getpeername(client_fd, (struct sockaddr*)&client_addr_tcp, &client_addr_len_tcp);

    printf("Client connected: %s:%d (TCP)\n", inet_ntoa(client_addr_tcp.sin_addr), ntohs(client_addr_tcp.sin_port));

    // Thêm client vào mảng
    pthread_mutex_lock(&client_mutex);
    if (client_count < MAX_CLIENTS) {
        clients[client_count].sock_fd = client_fd;
        clients[client_count].addr = client_addr_tcp; // Lưu tạm địa chỉ TCP
        clients[client_count].addr_len = client_addr_len_tcp;
        client_count++;
        printf("Client count: %d\n", client_count);

        if (client_count == MAX_CLIENTS) {
            printf("Two clients connected. Signaling addresses...\n");
            pthread_cond_broadcast(&clients_ready_cond); // Báo hiệu 2 client đã sẵn sàng
        }
    } else {
        printf("Max clients reached. Refusing new connection.\n");
        close(client_fd);
        pthread_mutex_unlock(&client_mutex);
        return NULL;
    }
    pthread_mutex_unlock(&client_mutex);

    // Chờ cho đến khi có đủ 2 client
    pthread_mutex_lock(&client_mutex);
    while (client_count < MAX_CLIENTS) {
        pthread_cond_wait(&clients_ready_cond, &client_mutex);
    }
    pthread_mutex_unlock(&client_mutex);

    // Trao đổi thông tin địa chỉ UDP của các client
    // Server sẽ nhận địa chỉ UDP client gửi lên (sau khi đã cố gắng STUN hoặc biết IP công cộng)
    read_bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (read_bytes > 0) {
        buffer[read_bytes] = '\0';
        printf("Received UDP info from client %s:%d: %s\n", inet_ntoa(client_addr_tcp.sin_addr), ntohs(client_addr_tcp.sin_port), buffer);

        // Lưu thông tin UDP của client
        // Format: "UDP_IP:UDP_PORT"
        char* token_ip = strtok(buffer, ":");
        char* token_port = strtok(NULL, ":");
        if (token_ip && token_port) {
            pthread_mutex_lock(&client_mutex);
            for (int i = 0; i < MAX_CLIENTS; ++i) {
                if (clients[i].sock_fd == client_fd) {
                    inet_pton(AF_INET, token_ip, &(clients[i].addr.sin_addr)); // Lưu IP
                    clients[i].addr.sin_port = htons(atoi(token_port)); // Lưu Port
                    printf("Updated client %d UDP address to %s:%d\n", i + 1, inet_ntoa(clients[i].addr.sin_addr), ntohs(clients[i].addr.sin_port));
                    break;
                }
            }
            pthread_mutex_unlock(&client_mutex);
        }
    }

    // Chờ tất cả client gửi thông tin UDP
    pthread_mutex_lock(&client_mutex);
    // Đây là một cách đơn giản, một hệ thống thực tế sẽ cần logic kiểm tra cẩn thận hơn
    // để đảm bảo cả hai client đã gửi thông tin UDP của mình
    pthread_mutex_unlock(&client_mutex);


    // Trao đổi địa chỉ UDP giữa 2 client
    pthread_mutex_lock(&client_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].sock_fd == client_fd) {
            // Đây là client hiện tại, gửi cho nó địa chỉ của client còn lại
            int other_client_idx = (i == 0) ? 1 : 0;
            char peer_info[BUFFER_SIZE];
            snprintf(peer_info, sizeof(peer_info), "%s:%d",
                     inet_ntoa(clients[other_client_idx].addr.sin_addr),
                     ntohs(clients[other_client_idx].addr.sin_port));

            printf("Signaling client %s:%d (TCP) with peer info: %s\n",
                   inet_ntoa(client_addr_tcp.sin_addr), ntohs(client_addr_tcp.sin_port), peer_info);
            send(client_fd, peer_info, strlen(peer_info), 0);
            break;
        }
    }
    pthread_mutex_unlock(&client_mutex);

    printf("Client %s:%d (TCP) connection closed.\n", inet_ntoa(client_addr_tcp.sin_addr), ntohs(client_addr_tcp.sin_port));
    close(client_fd); // Đóng kết nối TCP với server sau khi trao đổi thông tin
    return NULL;
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in server_addr;

    // Tạo socket TCP
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Thiết lập tùy chọn tái sử dụng địa chỉ/port
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Lắng nghe trên tất cả các interface
    server_addr.sin_port = htons(SERVER_PORT);

    // Bind socket
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Lắng nghe kết nối
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Signaling Server listening on port %d\n", SERVER_PORT);

    while (1) {
        socklen_t client_addr_len = sizeof(struct sockaddr_in);
        new_socket = accept(server_fd, (struct sockaddr*)&clients[client_count].addr, &client_addr_len);
        if (new_socket < 0) {
            perror("Accept failed");
            continue;
        }

        pthread_t tid;
        int* client_fd_ptr = (int*)malloc(sizeof(int));
        *client_fd_ptr = new_socket;
        pthread_create(&tid, NULL, handle_client, (void*)client_fd_ptr);
        pthread_detach(tid); // Tách luồng để nó tự giải phóng tài nguyên khi kết thúc
    }

    close(server_fd);
    return 0;
}