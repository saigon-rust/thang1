#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "cJSON.h" // Bao gồm thư viện cJSON

#define PORT 8080
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 2048    // Tăng kích thước bộ đệm để chứa JSON lớn hơn
#define REQUEST_INTERVAL 10
#define JSON_FILE "request.json"

// --- Hàm đọc JSON từ file (giữ nguyên) ---
char* read_json_file(const char* filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening JSON file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char *)malloc(length + 1);
    if (buffer == NULL) {
        perror("Error allocating buffer for JSON file");
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, length, file);
    buffer[length] = '\0';

    fclose(file);
    return buffer;
}

// --- Đã sửa đổi: Hàm Encode() để chuyển token thành ma trận 2 chiều ---
// Hàm này lấy chuỗi JSON gốc, sửa đổi phần 'token' của nó thành dạng ma trận 2 chiều,
// và trả về chuỗi JSON mới đã được sửa đổi.
char* Encode(const char* original_json_string) {
    cJSON *root = NULL;
    char *modified_json_string = NULL;

    // Phân tích cú pháp chuỗi JSON gốc
    root = cJSON_Parse(original_json_string);
    if (root == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Encode Error: Failed to parse original JSON: %s\n", error_ptr);
        }
        return NULL;
    }

    // Lấy đối tượng 'token'
    cJSON *token_obj = cJSON_GetObjectItemCaseSensitive(root, "token");
    if (!cJSON_IsObject(token_obj)) {
        fprintf(stderr, "Encode Error: 'token' object not found or not an object.\n");
        cJSON_Delete(root);
        return NULL;
    }

    // Lấy đối tượng '2' bên trong 'token'
    cJSON *inner_2_obj = cJSON_GetObjectItemCaseSensitive(token_obj, "2");
    if (!cJSON_IsObject(inner_2_obj)) {
        fprintf(stderr, "Encode Error: 'token'->'2' object not found or not an object.\n");
        cJSON_Delete(root);
        return NULL;
    }

    // --- Tạo cấu trúc ma trận 2 chiều mới ---
    cJSON *new_matrix_array = cJSON_CreateArray(); // Đây sẽ là giá trị 'token' mới

    // Lấy giá trị của "1" bên trong "2"
    cJSON *val_1_in_2 = cJSON_GetObjectItemCaseSensitive(inner_2_obj, "1");
    if (cJSON_IsNumber(val_1_in_2)) {
        // Nếu bạn muốn bao gồm cả "1" trong ma trận, chúng ta cần thêm một dòng cho nó.
        // Tuy nhiên, theo mẫu ma trận 2 chiều bạn gửi, nó không có dòng nào cho "1" trực tiếp.
        // Tôi sẽ bỏ qua nó để phù hợp với mẫu [2,KEY,CHILD_KEY,VALUE]
        // Nếu bạn muốn bao gồm 123, hãy cho biết cách bạn muốn format nó.
    }

    // Xử lý đối tượng con "12"
    cJSON *val_12_obj = cJSON_GetObjectItemCaseSensitive(inner_2_obj, "12");
    if (cJSON_IsObject(val_12_obj)) {
        // Tạo các mảng con cho từng giá trị bên trong "12"
        
        // Giá trị cho "12" -> "1"
        cJSON *arr_1_in_12 = cJSON_CreateArray();
        cJSON_AddItemToArray(arr_1_in_12, cJSON_CreateNumber(2));   // Khóa cha '2'
        cJSON_AddItemToArray(arr_1_in_12, cJSON_CreateNumber(12));  // Khóa hiện tại '12'
        cJSON_AddItemToArray(arr_1_in_12, cJSON_CreateNumber(1));   // Khóa con '1'
        cJSON_AddItemToArray(arr_1_in_12, cJSON_CreateNumber(cJSON_GetObjectItemCaseSensitive(val_12_obj, "1")->valueint)); // Giá trị
        cJSON_AddItemToArray(new_matrix_array, arr_1_in_12); // Thêm vào mảng ma trận chính

        // Giá trị cho "12" -> "2"
        cJSON *arr_2_in_12 = cJSON_CreateArray();
        cJSON_AddItemToArray(arr_2_in_12, cJSON_CreateNumber(2));   // Khóa cha '2'
        cJSON_AddItemToArray(arr_2_in_12, cJSON_CreateNumber(12));  // Khóa hiện tại '12'
        cJSON_AddItemToArray(arr_2_in_12, cJSON_CreateNumber(2));   // Khóa con '2'
        cJSON_AddItemToArray(arr_2_in_12, cJSON_CreateNumber(cJSON_GetObjectItemCaseSensitive(val_12_obj, "2")->valueint)); // Giá trị
        cJSON_AddItemToArray(new_matrix_array, arr_2_in_12);

        // Giá trị cho "12" -> "5"
        cJSON *arr_5_in_12 = cJSON_CreateArray();
        cJSON_AddItemToArray(arr_5_in_12, cJSON_CreateNumber(2));   // Khóa cha '2'
        cJSON_AddItemToArray(arr_5_in_12, cJSON_CreateNumber(12));  // Khóa hiện tại '12'
        cJSON_AddItemToArray(arr_5_in_12, cJSON_CreateNumber(5));   // Khóa con '5'
        cJSON_AddItemToArray(arr_5_in_12, cJSON_CreateNumber(cJSON_GetObjectItemCaseSensitive(val_12_obj, "5")->valueint)); // Giá trị
        cJSON_AddItemToArray(new_matrix_array, arr_5_in_12);
    }

    // Xử lý đối tượng con "123"
    cJSON *val_123_obj = cJSON_GetObjectItemCaseSensitive(inner_2_obj, "123");
    if (cJSON_IsObject(val_123_obj)) {
        // Giá trị cho "123" -> "1"
        cJSON *arr_1_in_123 = cJSON_CreateArray();
        cJSON_AddItemToArray(arr_1_in_123, cJSON_CreateNumber(2));   // Khóa cha '2'
        cJSON_AddItemToArray(arr_1_in_123, cJSON_CreateNumber(123)); // Khóa hiện tại '123'
        cJSON_AddItemToArray(arr_1_in_123, cJSON_CreateNumber(1));   // Khóa con '1'
        cJSON_AddItemToArray(arr_1_in_123, cJSON_CreateNumber(cJSON_GetObjectItemCaseSensitive(val_123_obj, "1")->valueint)); // Giá trị
        cJSON_AddItemToArray(new_matrix_array, arr_1_in_123);
    }

    // Xử lý đối tượng con "112"
    cJSON *val_112_obj = cJSON_GetObjectItemCaseSensitive(inner_2_obj, "112");
    if (cJSON_IsObject(val_112_obj)) {
        // Giá trị cho "112" -> "1"
        cJSON *arr_1_in_112 = cJSON_CreateArray();
        cJSON_AddItemToArray(arr_1_in_112, cJSON_CreateNumber(2));   // Khóa cha '2'
        cJSON_AddItemToArray(arr_1_in_112, cJSON_CreateNumber(112)); // Khóa hiện tại '112'
        cJSON_AddItemToArray(arr_1_in_112, cJSON_CreateNumber(1));   // Khóa con '1'
        cJSON_AddItemToArray(arr_1_in_112, cJSON_CreateNumber(cJSON_GetObjectItemCaseSensitive(val_112_obj, "1")->valueint)); // Giá trị
        cJSON_AddItemToArray(new_matrix_array, arr_1_in_112);
    }

    // Thay thế đối tượng 'token' hiện có bằng mảng ma trận 2 chiều mới
    cJSON_DeleteItemFromObject(root, "token"); // Xóa đối tượng token cũ
    cJSON_AddItemToObject(root, "token", new_matrix_array); // Thêm mảng mới làm giá trị cho 'token'

    // Chuyển đổi cấu trúc cJSON đã sửa đổi trở lại thành chuỗi
    modified_json_string = cJSON_Print(root);
    if (modified_json_string == NULL) {
        fprintf(stderr, "Encode Error: Failed to print modified JSON.\n");
    }

    // Dọn dẹp đối tượng cJSON
    cJSON_Delete(root);

    return modified_json_string; // Nhớ giải phóng chuỗi này trong hàm main!
}

int main() {
    while (1) {
        int sock = 0;
        struct sockaddr_in serv_addr;
        char buffer_recv[BUFFER_SIZE] = {0};
        char *json_original_message = NULL;
        char *json_modified_message = NULL; // Đây sẽ chứa kết quả từ Encode()

        // 1. Tạo socket
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Socket creation error");
            sleep(REQUEST_INTERVAL);
            continue;
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(PORT);

        if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
            perror("Invalid address/ Address not supported");
            close(sock);
            sleep(REQUEST_INTERVAL);
            continue;
        }

        // 2. Kết nối đến server
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            perror("Connection Failed");
            close(sock);
            sleep(REQUEST_INTERVAL);
            continue;
        }
        printf("Connected to server %s:%d\n", SERVER_IP, PORT);

        // Đọc JSON gốc từ file
        json_original_message = read_json_file(JSON_FILE);
        if (json_original_message == NULL) {
            close(sock);
            sleep(REQUEST_INTERVAL);
            continue;
        }

        printf("Client: Original JSON read from file:\n%s\n", json_original_message);

        // --- Sử dụng hàm Encode mới ---
        json_modified_message = Encode(json_original_message);
        json_modified_message = Encode_(json_original_message);
        if (json_modified_message == NULL) {
            fprintf(stderr, "Client: Failed to encode JSON message. Aborting send.\n");
            free(json_original_message); // Giải phóng tin nhắn gốc
            close(sock);
            sleep(REQUEST_INTERVAL);
            continue;
        }

        printf("Client: Modified JSON after Encode():\n%s\n", json_modified_message);

        // 3. Gửi dữ liệu JSON đã sửa đổi cho server
        printf("Client: Sending modified JSON data...\n");
        if (strlen(json_modified_message) >= BUFFER_SIZE) {
            fprintf(stderr, "Warning: Modified JSON message might be truncated (size %zu > BUFFER_SIZE %d)\n", strlen(json_modified_message), BUFFER_SIZE);
        }
        send(sock, json_modified_message, strlen(json_modified_message), 0);
        printf("Client: Modified JSON data sent.\n");

        // 4. Nhận dữ liệu từ server
        memset(buffer_recv, 0, BUFFER_SIZE);
        ssize_t valread = recv(sock, buffer_recv, BUFFER_SIZE - 1, 0);
        if (valread < 0) {
            perror("recv failed");
        } else if (valread == 0) {
            printf("Server closed connection.\n");
        } else {
            buffer_recv[valread] = '\0';
            printf("Server: %s\n", buffer_recv);
        }

        // 5. Đóng socket và giải phóng bộ nhớ
        free(json_original_message);   // Giải phóng bộ nhớ cho chuỗi JSON gốc
        free(json_modified_message);   // Giải phóng bộ nhớ cho chuỗi JSON đã sửa đổi
        close(sock);
        printf("Connection closed. Waiting %d seconds for next request...\n", REQUEST_INTERVAL);
        sleep(REQUEST_INTERVAL);
    }

    return 0;
}