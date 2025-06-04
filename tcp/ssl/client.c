#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <sys/socket.h>
#include <time.h>
#include <stdint.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345
#define MAX_PACKET_SIZE 8192

// Load RSA public key từ chuỗi PEM (string)
RSA *load_public_key_from_string(const char *pem_str) {
    BIO *bio = BIO_new_mem_buf(pem_str, -1);
    if (!bio) return NULL;

    RSA *rsa = PEM_read_bio_RSA_PUBKEY(bio, NULL, NULL, NULL);
    BIO_free(bio);
    return rsa;
}

// Gửi packet: 4 byte chiều dài (big-endian) + payload
int send_packet(int sockfd, const unsigned char *data, uint32_t len) {
    uint32_t len_be = htonl(len);
    if (send(sockfd, &len_be, 4, 0) != 4) return -1;
    if (send(sockfd, data, len, 0) != (int)len) return -1;
    return 0;
}

// Tạo AES-256 key và AES IV 128-bit ngẫu nhiên
void generate_aes_key(unsigned char *key, unsigned char *iv) {
    if (RAND_bytes(key, 32) != 1 || RAND_bytes(iv, 16) != 1) {
        fprintf(stderr, "Error generating random AES key or IV\n");
        exit(1);
    }
}

// Mã hóa AES-CBC với PKCS#7 padding
// Trả về buffer malloc, cipher_len_out chứa kích thước cipher
unsigned char *aes_encrypt(const unsigned char *plaintext, int plaintext_len,
                           const unsigned char *key, const unsigned char *iv,
                           int *cipher_len_out) {
    int block_size = AES_BLOCK_SIZE;
    int padding = block_size - (plaintext_len % block_size);
    int cipher_len = plaintext_len + padding;

    // Thêm padding PKCS#7
    unsigned char *padded = malloc(cipher_len);
    memcpy(padded, plaintext, plaintext_len);
    memset(padded + plaintext_len, padding, padding);

    unsigned char *cipher = malloc(cipher_len);
    AES_KEY enc_key;
    AES_set_encrypt_key(key, 256, &enc_key);

    // AES CBC mã hóa, chú ý iv không bị thay đổi
    unsigned char iv_copy[16];
    memcpy(iv_copy, iv, 16);

    AES_cbc_encrypt(padded, cipher, cipher_len, &enc_key, iv_copy, AES_ENCRYPT);

    free(padded);
    *cipher_len_out = cipher_len;
    return cipher;
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[4096];
    unsigned char aes_key[32], aes_iv[16];
    time_t last_key_time = 0;
    RSA *rsa = NULL;

    // Tạo socket TCP
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    // Thiết lập địa chỉ server
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid server IP address\n");
        close(sockfd);
        return 1;
    }

    // Kết nối đến server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sockfd);
        return 1;
    }

    // Nhận public key PEM từ server (giả định server gửi public key ngay sau kết nối)
    int len = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
    if (len <= 0) {
        fprintf(stderr, "Failed to receive public key\n");
        close(sockfd);
        return 1;
    }
    buffer[len] = '\0';

    // Parse public key từ chuỗi PEM nhận được
    rsa = load_public_key_from_string(buffer);
    if (!rsa) {
        fprintf(stderr, "Failed to parse public key\n");
        close(sockfd);
        return 1;
    }

    printf("Public key received.\n");

    // Vòng lặp gửi tin nhắn và xoay AES key mỗi 10s
    while (1) {
        time_t now = time(NULL);

        // Nếu chưa có key hoặc đã quá 10s, tạo AES key mới và gửi lên server
        if (last_key_time == 0 || now - last_key_time >= 10) {
            generate_aes_key(aes_key, aes_iv);
            printf("Generated new AES key and IV.\n");

            // Kết hợp key + iv để mã hóa RSA
            unsigned char key_iv[48];
            memcpy(key_iv, aes_key, 32);
            memcpy(key_iv + 32, aes_iv, 16);

            unsigned char encrypted[256];
            int encrypted_len = RSA_public_encrypt(48, key_iv, encrypted, rsa, RSA_PKCS1_OAEP_PADDING);
            if (encrypted_len < 0) {
                fprintf(stderr, "RSA encryption failed.\n");
                break;
            }

            if (send_packet(sockfd, encrypted, encrypted_len) < 0) {
                fprintf(stderr, "Failed to send encrypted AES key.\n");
                break;
            }

            last_key_time = now;
        }

        // Nhập message từ stdin
        printf("Enter message: ");
        fflush(stdout);
        if (!fgets(buffer, sizeof(buffer), stdin)) break;

        int msg_len = strlen(buffer);
        if (msg_len > 0 && buffer[msg_len - 1] == '\n') buffer[--msg_len] = '\0';

        // Mã hóa AES message vừa nhập
        int cipher_len;
        unsigned char *cipher = aes_encrypt((unsigned char *)buffer, msg_len, aes_key, aes_iv, &cipher_len);
        if (!cipher) {
            fprintf(stderr, "Encryption failed.\n");
            break;
        }

        // Gửi message đã mã hóa
        if (send_packet(sockfd, cipher, cipher_len) < 0) {
            fprintf(stderr, "Failed to send encrypted message.\n");
            free(cipher);
            break;
        }

        printf("Encrypted message sent.\n");
        free(cipher);
    }

    RSA_free(rsa);
    close(sockfd);
    return 0;
}
