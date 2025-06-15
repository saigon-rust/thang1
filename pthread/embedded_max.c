#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define ARRAY_SIZE 100000
#define MAX_THREADS 8  // Giới hạn luồng tối đa để tiết kiệm bộ nhớ

static int array[ARRAY_SIZE];
static int local_max[MAX_THREADS];

typedef struct {
    int thread_index;
    int start;
    int end;
} ThreadArgs;

void* thread_find_max(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    int max = array[args->start];
    for (int i = args->start + 1; i < args->end; ++i) {
        if (array[i] > max)
            max = array[i];
    }
    local_max[args->thread_index] = max;
    return NULL;
}

int main() {
    // Khởi tạo mảng với dữ liệu giả định
    for (int i = 0; i < ARRAY_SIZE; ++i)
        array[i] = rand() % 100000;

    // Lấy số CPU cores khả dụng (fallback = 1 nếu không hỗ trợ)
    int cpu_cores = (int)sysconf(_SC_NPROCESSORS_ONLN);
    if (cpu_cores <= 0 || cpu_cores > MAX_THREADS)
        cpu_cores = 1;

    int num_threads = cpu_cores;

    pthread_t threads[MAX_THREADS];
    ThreadArgs args[MAX_THREADS];
    int chunk_size = ARRAY_SIZE / num_threads;

    for (int i = 0; i < num_threads; ++i) {
        args[i].thread_index = i;
        args[i].start = i * chunk_size;
        args[i].end = (i == num_threads - 1) ? ARRAY_SIZE : (i + 1) * chunk_size;
        pthread_create(&threads[i], NULL, thread_find_max, &args[i]);
    }

    for (int i = 0; i < num_threads; ++i)
        pthread_join(threads[i], NULL);

    // Tìm max toàn cục
    int global_max = local_max[0];
    for (int i = 1; i < num_threads; ++i) {
        if (local_max[i] > global_max)
            global_max = local_max[i];
    }

    // Chỉ in kết quả cuối cùng (tùy yêu cầu)
    printf("MAX = %d\n", global_max);
    return 0;
}
