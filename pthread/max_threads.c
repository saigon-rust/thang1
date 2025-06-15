#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define ARRAY_SIZE 1000000
int array[ARRAY_SIZE];
int* local_max; // mỗi luồng lưu max cục bộ

typedef struct {
    int thread_index;
    int start;
    int end;
} ThreadData;

void* find_max(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int max = array[data->start];
    for (int i = data->start + 1; i < data->end; ++i) {
        if (array[i] > max)
            max = array[i];
    }
    local_max[data->thread_index] = max;
    pthread_exit(NULL);
}

int main() {
    // Khởi tạo mảng với số ngẫu nhiên
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        array[i] = rand() % 1000000;
    }

    // Lấy số luồng tối đa hiện có (số lõi CPU)
    int num_threads = sysconf(_SC_NPROCESSORS_ONLN);
    printf("🧠 Number of CPU cores: %d\n", num_threads);

    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];
    local_max = malloc(sizeof(int) * num_threads);

    int chunk_size = ARRAY_SIZE / num_threads;

    // Tạo các luồng
    for (int i = 0; i < num_threads; ++i) {
        thread_data[i].thread_index = i;
        thread_data[i].start = i * chunk_size;
        thread_data[i].end = (i == num_threads - 1) ? ARRAY_SIZE : (i + 1) * chunk_size;
        pthread_create(&threads[i], NULL, find_max, &thread_data[i]);
    }

    // Chờ các luồng kết thúc
    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], NULL);
    }

    // Tìm max toàn cục
    int global_max = local_max[0];
    for (int i = 1; i < num_threads; ++i) {
        if (local_max[i] > global_max)
            global_max = local_max[i];
    }

    printf("✅ Max value in array: %d\n", global_max);

    free(local_max);
    return 0;
}
