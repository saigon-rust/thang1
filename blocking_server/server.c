// server.c
// Compile: gcc -o server server.c
// Run: ./server

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <errno.h>

#define PORT 8080
#define BUF_SIZE 4096

// Hàm xử lý kết nối từ client
void handle_client(int client_fd) {
    char buffer[BUF_SIZE];
    ssize_t bytes_read;

    // Vòng lặp đọc và echo lại dữ liệu
    while ((bytes_read = read(client_fd, buffer, sizeof(buffer))) > 0) {

        // Xử lý từng byte trong buffer
        for (ssize_t i = 0; i < bytes_read; i++) {
            printf("%02X ", buffer[i]);  // in byte theo hex
        }

        // Gửi lại chính xác dữ liệu đã nhận
        if (write(client_fd, buffer, bytes_read) != bytes_read) {
            perror("write");
            break;
        }
    }
    
    // Xử lý khi kết nối đóng hoặc có lỗi
    if (bytes_read == 0) {
        printf("Client closed the connection.\n");
    } else if (bytes_read < 0) {
        perror("read");
    }

    close(client_fd);
    printf("Connection with fd=%d closed.\n", client_fd);
    exit(0); // Kết thúc tiến trình con
}

int main() {
    int listen_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // 1. Tạo socket
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    printf("Socket created (listen_fd=%d).\n", listen_fd);

    // Cấu hình địa chỉ server
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    // 2. Gán địa chỉ cho socket
    if (bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    printf("Socket bound to port %d.\n", PORT);

    // 3. Lắng nghe kết nối
    if (listen(listen_fd, SOMAXCONN) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    printf("Server listening on port %d...\n", PORT);

    while (1) {
        // Xử lý các tiến trình con đã kết thúc để tránh "zombie process"
        while (waitpid(-1, NULL, WNOHANG) > 0);

        // 4. Chấp nhận kết nối mới
        client_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }
        printf("Accepted new connection from %s:%d (client_fd=%d).\n",
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), client_fd);

        // 5. Tạo một tiến trình con để xử lý client
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            close(client_fd);
        } else if (pid == 0) { // Đây là tiến trình con
            close(listen_fd); // Tiến trình con không cần socket lắng nghe
            handle_client(client_fd);
            // Chương trình con sẽ kết thúc tại đây
        } else { // Đây là tiến trình cha
            close(client_fd); // Tiến trình cha không cần giữ socket của client
        }
    }

    // Đóng socket lắng nghe (không bao giờ đạt được trong vòng lặp vô hạn)
    close(listen_fd);
    return 0;
}

/*
read / write	Đơn giản, trực tiếp, khá nhanh cho I/O tuần tự, không flag phức tạp.

recv / send	Hỗ trợ flags để điều khiển, phù hợp khi cần non-blocking hoặc peek data.

Dùng sendfile() để gửi file trực tiếp từ kernel buffer sang socket mà không copy vào user-space.
Với nhận, có thể dùng splice() hoặc vmsplice().

Dùng readv/writev hoặc recvmsg/sendmsg để đọc/gửi nhiều buffer trong 1 lần syscall.
Tránh gọi read/write nhỏ giọt nhiều lần.
*/