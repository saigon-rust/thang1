#include <stdio.h>    // For printf, fprintf
#include <stdlib.h>   // For malloc, free, rand, srand, RAND_MAX
#include <pthread.h>  // For pthreads library
#include <time.h>     // For clock_gettime, CLOCK_MONOTONIC, time
#include <string.h>   // For memset (optional, but good practice for zeroing memory)

// Macro de kiem tra loi pthreads
#define HANDLE_PTHREAD_ERROR(err, msg) \
    if (err != 0) { \
        fprintf(stderr, "Loi pthread: %s - %s\n", msg, strerror(err)); \
        exit(EXIT_FAILURE); \
    }

// Struct de truyen tham so cho moi luong
typedef struct {
    double** A;        // Ma tran A
    double** B;        // Ma tran B
    double** C;        // Ma tran ket qua C
    size_t A_rows;     // So hang cua A
    size_t A_cols;     // So cot cua A
    size_t B_cols;     // So cot cua B
    size_t start_row;  // Hang bat dau ma luong nay se xu ly
    size_t end_row;    // Hang ket thuc (doc quyen) ma luong nay se xu ly
} thread_args_t;

// Ham cap phat dong mot ma tran
double** createMatrix(size_t rows, size_t cols) {
    double** matrix = (double**)malloc(rows * sizeof(double*));
    if (matrix == NULL) {
        fprintf(stderr, "Loi cap phat bo nho cho ma tran (hang)!\n");
        exit(EXIT_FAILURE);
    }
    for (size_t i = 0; i < rows; ++i) {
        matrix[i] = (double*)malloc(cols * sizeof(double));
        if (matrix[i] == NULL) {
            fprintf(stderr, "Loi cap phat bo nho cho ma tran (cot)!\n");
            // Giai phong cac hang da cap phat truoc do
            for (size_t k = 0; k < i; ++k) {
                free(matrix[k]);
            }
            free(matrix);
            exit(EXIT_FAILURE);
        }
        // Khoi tao tat ca phan tu ve 0 de dam bao tinh toan dung
        memset(matrix[i], 0, cols * sizeof(double));
    }
    return matrix;
}

// Ham giai phong bo nho cua ma tran
void freeMatrix(double** matrix, size_t rows) {
    if (matrix == NULL) return;
    for (size_t i = 0; i < rows; ++i) {
        free(matrix[i]);
    }
    free(matrix);
}

// Ham in ma tran ra console
void printMatrix(double** matrix, size_t rows, size_t cols, const char* name) {
    if (matrix == NULL) {
        printf("Ma tran %s rong hoac NULL.\n", name);
        return;
    }
    printf("Ma tran %s (%zu x %zu):\n", name, rows, cols);
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            printf("%8.2f ", matrix[i][j]); // Modified to 2 decimal places for printing
        }
        printf("\n");
    }
    printf("\n");
}

// Ham thuc hien phep nhan ma tran cho mot phan cua ma tran C
// Luong nay se tinh toan cac hang tu thread_args->start_row den thread_args->end_row - 1
void* multiplyMatrices_Thread(void* arg) {
    thread_args_t* args = (thread_args_t*)arg;

    // Lay cac ma tran va kich thuoc tu tham so
    double** A = args->A;
    double** B = args->B;
    double** C = args->C;
    size_t A_cols = args->A_cols;
    size_t B_cols = args->B_cols;
    size_t start_row = args->start_row;
    size_t end_row = args->end_row;

    // Thuc hien phep nhan ma tran voi thu tu vong lap I-K-J cho phan cong viec nay
    // I: hang hien tai cua A va C (start_row den end_row - 1)
    // K: cot cua A va hang cua B
    // J: cot cua B va C
    for (size_t i = start_row; i < end_row; ++i) {
        for (size_t k = 0; k < A_cols; ++k) {
            double A_ik = A[i][k]; // Lay gia tri A[i][k] mot lan de giam viec truy cap bo nho
            for (size_t j = 0; j < B_cols; ++j) {
                C[i][j] += A_ik * B[k][j];
            }
        }
    }

    pthread_exit(NULL); // Ket thuc luong
}

// Ham nhan hai ma tran A va B mot cach song song voi toi uu I-K-J
double** multiplyMatricesOptimized_Parallel(double** A, double** B,
                                            size_t A_rows, size_t A_cols,
                                            size_t B_rows, size_t B_cols,
                                            int num_threads) {
    // Kiem tra dieu kien de nhan ma tran: so cot cua A phai bang so hang cua B
    if (A_cols != B_rows) {
        fprintf(stderr, "Loi: Kich thuoc ma tran khong tuong thich de nhan! (%zu != %zu)\n", A_cols, B_rows);
        return NULL;
    }

    // Tao ma tran ket qua C
    double** C = createMatrix(A_rows, B_cols);

    // Kiem tra so luong luong. Neu la 0 hoac 1, chay don luong.
    if (num_threads <= 0) {
        num_threads = 1; // Mac dinh la 1 luong neu so luong luong khong hop le
    }
    if (num_threads > A_rows) {
        num_threads = A_rows; // Khong nen co nhieu luong hon so hang
    }

    pthread_t* threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    thread_args_t* thread_args = (thread_args_t*)malloc(num_threads * sizeof(thread_args_t));

    if (threads == NULL || thread_args == NULL) {
        fprintf(stderr, "Loi cap phat bo nho cho luong hoac tham so luong!\n");
        freeMatrix(C, A_rows);
        if (threads) free(threads);
        if (thread_args) free(thread_args);
        return NULL;
    }

    // Chia cong viec cho tung luong
    size_t rows_per_thread = A_rows / num_threads;
    size_t remaining_rows = A_rows % num_threads;
    size_t current_row = 0;
    int rc; // Return code for pthread functions

    for (int i = 0; i < num_threads; ++i) {
        thread_args[i].A = A;
        thread_args[i].B = B;
        thread_args[i].C = C;
        thread_args[i].A_rows = A_rows; // Kich thuoc toan bo ma tran A (khong dung trong vong lap cua luong)
        thread_args[i].A_cols = A_cols;
        thread_args[i].B_cols = B_cols;
        thread_args[i].start_row = current_row;
        thread_args[i].end_row = current_row + rows_per_thread + (i < remaining_rows ? 1 : 0);

        rc = pthread_create(&threads[i], NULL, multiplyMatrices_Thread, (void*)&thread_args[i]);
        HANDLE_PTHREAD_ERROR(rc, "tao luong");

        current_row = thread_args[i].end_row;
    }

    // Doi tat ca cac luong hoan thanh
    for (int i = 0; i < num_threads; ++i) {
        rc = pthread_join(threads[i], NULL);
        HANDLE_PTHREAD_ERROR(rc, "tham gia luong");
    }

    free(threads);
    free(thread_args);

    return C;
}

int main() {
    // Dinh nghia kich thuoc ma tran
    // Chu y: voi ma tran lon, viec in ra se rat cham
    const size_t M = 1000; // So hang cua A va C
    const size_t K = 800;  // So cot cua A va so hang cua B
    const size_t N = 1200; // So cot cua B va C

    const int NUM_THREADS = 4; // So luong luong muon su dung cho che do song song

    // Khoi tao bo sinh so ngau nhien
    srand(time(NULL));

    // --- Chay voi da luong (Parallel Run) ---
    printf("--- Bat dau do thoi gian voi da luong (%d luong) ---\n", NUM_THREADS);

    // 1. Tao va khoi tao ma tran A va B cho che do song song
    double** A_parallel = createMatrix(M, K);
    double** B_parallel = createMatrix(K, N);

    printf("Khoi tao ma tran A_parallel (%zu x %zu) va B_parallel (%zu x %zu) voi du lieu ngau nhien tu -1.00 den 1.00...\n", M, K, K, N);
    for (size_t i = 0; i < M; ++i) {
        for (size_t j = 0; j < K; ++j) {
            A_parallel[i][j] = ((double)rand() / RAND_MAX * 200 - 100) / 100.0;
        }
    }
    for (size_t i = 0; i < K; ++i) {
        for (size_t j = 0; j < N; ++j) {
            B_parallel[i][j] = ((double)rand() / RAND_MAX * 200 - 100) / 100.0;
        }
    }

    // 2. Thuc hien phep nhan ma tran song song va do thoi gian
    struct timespec start_parallel, end_parallel;
    double elapsed_time_parallel;

    printf("Bat dau nhan ma tran song song...\n");
    clock_gettime(CLOCK_MONOTONIC, &start_parallel);

    double** C_parallel = multiplyMatricesOptimized_Parallel(A_parallel, B_parallel, M, K, K, N, NUM_THREADS);

    clock_gettime(CLOCK_MONOTONIC, &end_parallel);

    elapsed_time_parallel = (end_parallel.tv_sec - start_parallel.tv_sec) + (end_parallel.tv_nsec - start_parallel.tv_nsec) / 1e9;
    printf("Da ket thuc phep nhan ma tran song song.\n");
    printf("Thoi gian thuc thi (da luong): %.4f giay\n\n", elapsed_time_parallel);

    // 3. Giai phong bo nho cua cac ma tran song song
    freeMatrix(A_parallel, M);
    freeMatrix(B_parallel, K);
    freeMatrix(C_parallel, M);


    // --- Chay voi don luong (Single-threaded Run) ---
    printf("--- Bat dau do thoi gian voi don luong (1 luong) ---\n");

    // 1. Tao va khoi tao ma tran A va B cho che do don luong
    // Tao ma tran moi de dam bao phep do la cong bang, khong bi anh huong boi bo nho cache tu lan truoc
    double** A_single = createMatrix(M, K);
    double** B_single = createMatrix(K, N);

    printf("Khoi tao ma tran A_single (%zu x %zu) va B_single (%zu x %zu) voi du lieu ngau nhien tu -1.00 den 1.00...\n", M, K, K, N);
    for (size_t i = 0; i < M; ++i) {
        for (size_t j = 0; j < K; ++j) {
            A_single[i][j] = ((double)rand() / RAND_MAX * 200 - 100) / 100.0;
        }
    }
    for (size_t i = 0; i < K; ++i) {
        for (size_t j = 0; j < N; ++j) {
            B_single[i][j] = ((double)rand() / RAND_MAX * 200 - 100) / 100.0;
        }
    }

    // 2. Thuc hien phep nhan ma tran don luong va do thoi gian
    struct timespec start_single, end_single;
    double elapsed_time_single;

    printf("Bat dau nhan ma tran don luong...\n");
    clock_gettime(CLOCK_MONOTONIC, &start_single);

    // Goi ham song song nhung chi voi 1 luong
    double** C_single = multiplyMatricesOptimized_Parallel(A_single, B_single, M, K, K, N, 1);

    clock_gettime(CLOCK_MONOTONIC, &end_single);

    elapsed_time_single = (end_single.tv_sec - start_single.tv_sec) + (end_single.tv_nsec - start_single.tv_nsec) / 1e9;
    printf("Da ket thuc phep nhan ma tran don luong.\n");
    printf("Thoi gian thuc thi (don luong): %.4f giay\n\n", elapsed_time_single);

    // 3. In ket qua (chỉ in cho ma trận nhỏ để dễ quan sát)
    if (M <= 10 && N <= 10) {
        printf("--- Ket qua chi tiet (cho ma tran nho) ---\n");
        printMatrix(A_single, M, K, "A_single");
        printMatrix(B_single, K, N, "B_single");
        printMatrix(C_single, M, N, "C_single (Ket qua don luong)");
    } else {
        printf("Ma tran qua lon de in ket qua chi tiet ra console.\n");
    }

    // 4. Giai phong bo nho cua cac ma tran don luong
    freeMatrix(A_single, M);
    freeMatrix(B_single, K);
    freeMatrix(C_single, M);

    return 0;
}

