#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8888
#define BUF_SIZE 4096

void handle_request(const char *json, char *response) {
    char method[16], route[128];
    int databaseId = -1;

    // Trích xuất method
    char *m_pos = strstr(json, "\"method\"");
    if (m_pos) sscanf(m_pos, "\"method\": \"%15[^\"]\"", method);

    // Trích xuất route
    char *r_pos = strstr(json, "\"route\"");
    if (r_pos) sscanf(r_pos, "\"route\": \"%127[^\"]\"", route);

    // Trích xuất DatabaseId nếu có
    char *p_pos = strstr(json, "\"DatabaseId\"");
    if (p_pos) sscanf(p_pos, "\"DatabaseId\": %d", &databaseId);

    // Tùy logic
    if (strcmp(method, "GET") == 0 && strcmp(route, "/table/list") == 0) {
        snprintf(response, BUF_SIZE,
            "{ \"status\": \"ok\", \"tables\": [\"users\", \"orders\", \"products\"] }");
    } else if (strcmp(method, "POST") == 0 && strstr(route, "/table/") && strstr(route, "/rows")) {
        snprintf(response, BUF_SIZE,
            "{ \"status\": \"ok\", \"rows\": [ { \"id\": 1, \"name\": \"Alice\" }, { \"id\": 2, \"name\": \"Bob\" } ] }");
    } else {
        snprintf(response, BUF_SIZE, "{ \"status\": \"error\", \"message\": \"Unsupported route\" }");
    }
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUF_SIZE];
    socklen_t client_len = sizeof(client_addr);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_fd, 5);

    printf("[INFO] Server running on port %d\n", PORT);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        int recv_len = recv(client_fd, buffer, BUF_SIZE - 1, 0);
        if (recv_len <= 0) {
            close(client_fd);
            continue;
        }

        buffer[recv_len] = '\0';
        printf("[RECV] %s\n", buffer);

        char response[BUF_SIZE];
        handle_request(buffer, response);

        send(client_fd, response, strlen(response), 0);
        close(client_fd);
    }

    close(server_fd);
    return 0;
}
