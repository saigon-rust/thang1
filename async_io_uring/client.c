// client.c
// Compile: gcc -o client client.c
// Run: ./client <IP_SERVER> <PORT>
// Ví dụ: ./client 127.0.0.1 8080

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUF_SIZE 4096

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IP_SERVER> <PORT>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *server_ip = argv[1];
    int port = atoi(argv[2]);

    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUF_SIZE] = {0};
    const char *message = "Hello from client!";

    // Tạo socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Chuyển đổi địa chỉ IP từ chuỗi sang định dạng nhị phân
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Kết nối tới server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        exit(EXIT_FAILURE);
    }

    // Gửi dữ liệu tới server
    send(sock, message, strlen(message), 0);
    printf("Message sent to server: %s\n", message);

    // Đọc dữ liệu echo lại từ server
    int valread = read(sock, buffer, BUF_SIZE);
    if (valread > 0) {
        printf("Message received from server: %s\n", buffer);
    } else if (valread == 0) {
        printf("Server closed the connection\n");
    } else {
        perror("Read failed");
    }

    // Đóng socket
    close(sock);

    return 0;
}