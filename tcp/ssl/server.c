#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/aes.h>
#include <openssl/err.h>
#include <pthread.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

#define PORT 12345
#define PRIVATE_KEY_FILE "private.pem"
#define PUBLIC_KEY_FILE "public.pem"
#define MAX_PACKET_SIZE 8192
#define THREAD_POOL_SIZE 8
#define MAX_QUEUE_SIZE 128

// Queue để lưu socket client chờ xử lý
typedef struct {
    int clients[MAX_QUEUE_SIZE];
    int front, rear;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} client_queue_t;

// Khai báo biến toàn cục
client_queue_t client_queue;
RSA *rsa_private = NULL;

// Hàm sinh cặp khóa RSA (giữ nguyên từ bạn)
void generate_keys() {
    RSA *rsa = RSA_new();
    BIGNUM *bne = BN_new();
    BN_set_word(bne, RSA_F4);
    if (!RSA_generate_key_ex(rsa, 2048, bne, NULL)) {
        fprintf(stderr, "Error generating RSA key.\n");
        exit(1);
    }
    FILE *fp = fopen(PRIVATE_KEY_FILE, "wb");
    if (!fp) {
        perror("fopen private key");
        exit(1);
    }
    PEM_write_RSAPrivateKey(fp, rsa, NULL, NULL, 0, NULL, NULL);
    fclose(fp);
    fp = fopen(PUBLIC_KEY_FILE, "wb");
    if (!fp) {
        perror("fopen public key");
        exit(1);
    }
    PEM_write_RSA_PUBKEY(fp, rsa);
    fclose(fp);
    RSA_free(rsa);
    BN_free(bne);
}

// Load private key (giữ nguyên)
RSA *load_private_key() {
    FILE *fp = fopen(PRIVATE_KEY_FILE, "rb");
    if (!fp) {
        perror("fopen private key");
        return NULL;
    }
    RSA *rsa = PEM_read_RSAPrivateKey(fp, NULL, NULL, NULL);
    fclose(fp);
    return rsa;
}

// Đảm bảo nhận đủ dữ liệu len byte
int recv_all(int sock, unsigned char *buf, int len) {
    int total = 0;
    while (total < len) {
        int n = recv(sock, buf + total, len - total, 0);
        if (n <= 0) return n;
        total += n;
    }
    return total;
}

// Khởi tạo queue
void queue_init(client_queue_t *q) {
    q->front = 0;
    q->rear = 0;
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->cond, NULL);
}

// Đưa socket vào queue
void queue_push(client_queue_t *q, int client_fd) {
    pthread_mutex_lock(&q->mutex);
    // Nếu queue đầy, đợi (hoặc có thể bỏ qua client)
    while ((q->rear + 1) % MAX_QUEUE_SIZE == q->front) {
        pthread_cond_wait(&q->cond, &q->mutex);
    }
    q->clients[q->rear] = client_fd;
    q->rear = (q->rear + 1) % MAX_QUEUE_SIZE;
    pthread_cond_signal(&q->cond);
    pthread_mutex_unlock(&q->mutex);
}

// Lấy socket ra khỏi queue
int queue_pop(client_queue_t *q) {
    pthread_mutex_lock(&q->mutex);
    while (q->front == q->rear) { // queue rỗng
        pthread_cond_wait(&q->cond, &q->mutex);
    }
    int client_fd = q->clients[q->front];
    q->front = (q->front + 1) % MAX_QUEUE_SIZE;
    pthread_cond_signal(&q->cond);
    pthread_mutex_unlock(&q->mutex);
    return client_fd;
}

// Xử lý client trên thread pool
void *worker_thread(void *arg) {
    (void)arg;
    unsigned char aes_key[32] = {0};
    unsigned char aes_iv[16] = {0};
    int rsa_len = RSA_size(rsa_private);

    while (1) {
        int client_fd = queue_pop(&client_queue);

        // Gửi public key cho client
        FILE *fp = fopen(PUBLIC_KEY_FILE, "rb");
        if (!fp) {
            fprintf(stderr, "Failed to open public key file.\n");
            close(client_fd);
            continue;
        }
        fseek(fp, 0, SEEK_END);
        long pub_len = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        char *pub_key = malloc(pub_len + 1);
        if (!pub_key) {
            fclose(fp);
            close(client_fd);
            fprintf(stderr, "Memory allocation failed.\n");
            continue;
        }
        fread(pub_key, 1, pub_len, fp);
        fclose(fp);
        pub_key[pub_len] = '\0';

        if (send(client_fd, pub_key, pub_len, 0) != pub_len) {
            fprintf(stderr, "Failed to send public key.\n");
            free(pub_key);
            close(client_fd);
            continue;
        }
        free(pub_key);

        printf("[Thread %ld] Client connected.\n", pthread_self());

        while (1) {
            uint32_t len_net;
            int n = recv_all(client_fd, (unsigned char *)&len_net, 4);
            if (n <= 0) break;

            uint32_t len = ntohl(len_net);
            if (len == 0 || len > MAX_PACKET_SIZE) {
                fprintf(stderr, "[Thread %ld] Packet size invalid: %u\n", pthread_self(), len);
                break;
            }

            unsigned char *packet = malloc(len);
            if (!packet) {
                fprintf(stderr, "[Thread %ld] Memory alloc failed.\n", pthread_self());
                break;
            }

            n = recv_all(client_fd, packet, len);
            if (n <= 0) {
                free(packet);
                break;
            }

            if (len == (uint32_t)rsa_len) {
                // Nhận AES key + IV mã hóa RSA
                unsigned char decrypted[512];
                int dec_len = RSA_private_decrypt(len, packet, decrypted, rsa_private, RSA_PKCS1_OAEP_PADDING);
                if (dec_len == 48) {
                    memcpy(aes_key, decrypted, 32);
                    memcpy(aes_iv, decrypted + 32, 16);
                    printf("[Thread %ld] 🔑 AES key updated.\n", pthread_self());
                } else {
                    fprintf(stderr, "[Thread %ld] ❌ AES key decryption failed.\n", pthread_self());
                }
            } else {
                if (aes_key[0] == 0) {
                    fprintf(stderr, "[Thread %ld] ❌ AES key not set.\n", pthread_self());
                    free(packet);
                    continue;
                }

                AES_KEY dec_key;
                AES_set_decrypt_key(aes_key, 256, &dec_key);

                unsigned char iv_copy[16];
                memcpy(iv_copy, aes_iv, 16);  // IV reset mỗi message

                unsigned char *plain = malloc(len);
                if (!plain) {
                    fprintf(stderr, "[Thread %ld] Memory alloc failed.\n", pthread_self());
                    free(packet);
                    break;
                }

                AES_cbc_encrypt(packet, plain, len, &dec_key, iv_copy, AES_DECRYPT);

                int padding = plain[len - 1];
                if (padding > 0 && padding <= 16) {
                    len -= padding;
                }

                plain[len] = '\0';

                printf("[Thread %ld] 📩 Message: %s\n", pthread_self(), plain);

                free(plain);
            }
            free(packet);
        }

        close(client_fd);
        printf("[Thread %ld] Client disconnected.\n", pthread_self());

        // Reset AES key/IV để bảo mật tốt hơn khi client reconnect
        memset(aes_key, 0, sizeof(aes_key));
        memset(aes_iv, 0, sizeof(aes_iv));
    }

    return NULL;
}

int main() {
    // Tắt SIGPIPE để tránh crash khi gửi dữ liệu socket đã đóng
    signal(SIGPIPE, SIG_IGN);

    if (access(PRIVATE_KEY_FILE, F_OK) != 0 || access(PUBLIC_KEY_FILE, F_OK) != 0) {
        printf("Generating RSA key pair...\n");
        generate_keys();
    }

    rsa_private = load_private_key();
    if (!rsa_private) {
        fprintf(stderr, "❌ Failed to load private key\n");
        return 1;
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        RSA_free(rsa_private);
        return 1;
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        RSA_free(rsa_private);
        return 1;
    }

    if (listen(server_fd, 128) < 0) {
        perror("listen");
        close(server_fd);
        RSA_free(rsa_private);
        return 1;
    }

    printf("🚀 Server listening on port %d with thread pool size %d...\n", PORT, THREAD_POOL_SIZE);

    queue_init(&client_queue);

    pthread_t threads[THREAD_POOL_SIZE];
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        if (pthread_create(&threads[i], NULL, worker_thread, NULL) != 0) {
            perror("pthread_create");
            close(server_fd);
            RSA_free(rsa_private);
            return 1;
        }
    }

    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            if (errno == EINTR) continue;
            perror("accept");
            continue;
        }
        queue_push(&client_queue, client_fd);
    }

    // Không tới được đây nhưng nếu cần có thể giải phóng tài nguyên
    RSA_free(rsa_private);
    close(server_fd);

    return 0;
}
//gcc server.c -o server -lssl -lcrypto -lpthread

