#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h> // Để đo thời gian
#include <sys/time.h> // Để đo thời gian chính xác hơn trên Linux/macOS

#define M 512
#define N 512
#define P 512
#define NUM_THREADS 8

// Ma trận toàn cục (hoặc cấp phát động trên heap)
float A[M][N];
float B[N][P];
float B_T[P][N]; // Ma trận B chuyển vị
float C[M][P];

typedef struct {
    int start_row;
    int end_row;
} ThreadArg;

// Hàm chuyển vị ma trận B
void transpose_B() {
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < P; ++j) {
            B_T[j][i] = B[i][j];
        }
    }
}

void* matrix_multiply_worker(void* arg) {
    ThreadArg* t = (ThreadArg*)arg;
    for (int i = t->start_row; i < t->end_row; ++i) {
        for (int j = 0; j < P; ++j) {
            float sum = 0.0f;
            for (int k = 0; k < N; ++k) {
                sum += A[i][k] * B_T[j][k]; // SỬ DỤNG B_T[j][k]
            }
            C[i][j] = sum;
        }
    }
    return NULL;
}

void matrix_multiply_threaded() {
    // 1. Chuyển vị ma trận B trước (có thể làm song song nếu B rất lớn)
    transpose_B(); // Trong ví dụ này làm tuần tự, nhưng có thể song song

    pthread_t threads[NUM_THREADS];
    ThreadArg args[NUM_THREADS];

    int rows_per_thread = M / NUM_THREADS;

    for (int i = 0; i < NUM_THREADS; ++i) {
        args[i].start_row = i * rows_per_thread;
        args[i].end_row = (i == NUM_THREADS - 1) ? M : (i + 1) * rows_per_thread;
        pthread_create(&threads[i], NULL, matrix_multiply_worker, &args[i]);
    }

    for (int i = 0; i < NUM_THREADS; ++i)
        pthread_join(threads[i], NULL);
}

int main() {
    // Khởi tạo A và B với dữ liệu ngẫu nhiên
    srand(time(NULL)); // Khởi tạo seed cho rand()
    for (int i = 0; i < M; ++i)
        for (int j = 0; j < N; ++j)
            A[i][j] = (float)(rand() % 1000) / 100.0f;

    for (int i = 0; i < N; ++i)
        for (int j = 0; j < P; ++j)
            B[i][j] = (float)(rand() % 1000) / 100.0f;

    struct timeval start, end;
    gettimeofday(&start, NULL);

    matrix_multiply_threaded();

    gettimeofday(&end, NULL);
    long seconds = end.tv_sec - start.tv_sec;
    long microseconds = end.tv_usec - start.tv_usec;
    double elapsed = seconds + microseconds * 1e-6;

    printf("Thoi gian thuc hien: %.4f giay\n", elapsed);
    printf("C[0][0] = %.2f\n", C[0][0]);
    printf("C[M-1][P-1] = %.2f\n", C[M-1][P-1]);

    return 0;
}