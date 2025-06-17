#include <stdio.h>
#include <stdlib.h>
#include "linear_regression.h"

#define INPUT_SIZE 3
#define OUTPUT_SIZE 3
#define N_SAMPLES 3

float** allocate_2d(int rows, int cols) {
    float** mat = malloc(rows * sizeof(float*));
    for (int i = 0; i < rows; i++) {
        mat[i] = calloc(cols, sizeof(float));
    }
    return mat;
}

int main() {
    float** X_train = allocate_2d(N_SAMPLES, INPUT_SIZE);
    float** y_train = allocate_2d(N_SAMPLES, OUTPUT_SIZE);

    // Giả lập one-hot encoding cho 3 câu hỏi
    X_train[0][0] = 1; // Câu 1: bạn tên gì?
    X_train[1][1] = 1; // Câu 2: bạn ở đâu?
    X_train[2][2] = 1; // Câu 3: bạn bao nhiêu tuổi?

    y_train[0][0] = 1; // Trả lời: tôi không tên
    y_train[1][1] = 1; // Trả lời: ở Vũng Tàu
    y_train[2][2] = 1; // Trả lời: 1991

    // Tạo và huấn luyện mô hình
    LinearRegression* model = create_model(INPUT_SIZE, OUTPUT_SIZE, 1000, 0.1f);
    train(model, X_train, y_train, N_SAMPLES);
    save_model(model, "model.dat");

    printf("✅ Mô hình đã được lưu vào file: model.dat\n");

    // Giải phóng
    free_model(model);
    for (int i = 0; i < N_SAMPLES; i++) {
        free(X_train[i]);
        free(y_train[i]);
    }
    free(X_train);
    free(y_train);
    return 0;
}
/*
gcc training.c linear_regression.c -o training -lm
./training
*/