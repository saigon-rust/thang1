//gcc -O3 -Wall -o io_uring_echo_server io_uring_echo_server.c -luring
//./io_uring_echo_server

#define _GNU_SOURCE
#include <liburing.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define PORT 8080
#define BUF_SIZE 4096
#define RING_SIZE 2048

enum {
    ACCEPT_REQ,
    READ_REQ,
    WRITE_REQ,
};

typedef struct {
    int fd;
    int type;
    char *buf;
} request_t;

static char bufs[RING_SIZE][BUF_SIZE];
static request_t requests[RING_SIZE];

static void submit_accept(struct io_uring *ring, int listen_fd, int flags) {
    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    io_uring_prep_accept(sqe, listen_fd, (struct sockaddr *)&client_addr, &client_len, 0);
    
    // Sử dụng request_t cố định
    request_t *req = &requests[0];
    req->fd = listen_fd;
    req->type = ACCEPT_REQ;
    io_uring_sqe_set_data(sqe, req);
}

static void submit_read_write(struct io_uring *ring, int fd, int slot, int type, int buf_idx, int count) {
    struct io_uring_sqe *sqe = io_uring_get_sqe(ring);
    request_t *req = &requests[slot];
    
    req->fd = fd;
    req->type = type;
    req->buf = bufs[buf_idx];

    if (type == READ_REQ) {
        io_uring_prep_read(sqe, fd, req->buf, BUF_SIZE, 0);
    } else { // WRITE_REQ
        io_uring_prep_write(sqe, fd, req->buf, count, 0);
    }
    
    io_uring_sqe_set_data(sqe, req);
}

int main() {
    struct io_uring ring;
    io_uring_queue_init(RING_SIZE, &ring, IORING_SETUP_SQPOLL);

    for (int i = 0; i < RING_SIZE; i++) {
        requests[i].fd = -1;
    }

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) { perror("socket"); exit(1); }

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = htonl(INADDR_ANY),
        .sin_port = htons(PORT)
    };
    if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind"); exit(1);
    }
    if (listen(listen_fd, SOMAXCONN) < 0) {
        perror("listen"); exit(1);
    }
    
    printf("io_uring echo server listening on port1 %d\n", PORT);
    
    submit_accept(&ring, listen_fd, 0);
    
    while (1) {
        struct io_uring_cqe *cqe;
        int ret;
        
        // Gửi các yêu cầu đã được xếp hàng đợi
        ret = io_uring_submit(&ring);
        
        // Chờ sự kiện hoàn thành nếu không có yêu cầu nào được xử lý
        ret = io_uring_wait_cqe(&ring, &cqe);
        if (ret < 0) {
            perror("io_uring_wait_cqe");
            break;
        }

        request_t *req = (request_t *)io_uring_cqe_get_data(cqe);
        int res = cqe->res;
        int slot = req - requests;

        if (req->type == ACCEPT_REQ) {
            if (res >= 0) {
                int client_fd = res;
                printf("New connection accepted: fd=%d\n", client_fd);
                submit_read_write(&ring, client_fd, 1, READ_REQ, 1, 0);
            }
            submit_accept(&ring, listen_fd, 0);
        } else if (req->type == READ_REQ) {
            if (res <= 0) {
                close(req->fd);
                printf("Connection closed: fd=%d\n", req->fd);
                req->fd = -1;
            } else {
                submit_read_write(&ring, req->fd, slot, WRITE_REQ, slot, res);
            }
        } else if (req->type == WRITE_REQ) {
            if (res <= 0) {
                close(req->fd);
                printf("Connection closed during write: fd=%d\n", req->fd);
                req->fd = -1;
            } else {
                submit_read_write(&ring, req->fd, slot, READ_REQ, slot, 0);
            }
        }
        
        io_uring_cqe_seen(&ring, cqe);
    }

    io_uring_queue_exit(&ring);
    close(listen_fd);
    return 0;
}