#include <stdio.h>
#include <stdlib.h>
#include "linear_regression.h"

#define INPUT_SIZE 3
#define OUTPUT_SIZE 3

const char* answers[] = {
    "tôi không tên",
    "tôi ở Vũng Tàu",
    "1991"
};

int main() {
    LinearRegression* model = load_model("model.dat");
    if (!model) {
        fprintf(stderr, "❌ Không thể tải mô hình từ file.\n");
        return 1;
    }

    while (1) {
        int choice;
        printf("\nChọn câu hỏi:\n");
        printf("1. bạn tên gì\n");
        printf("2. bạn ở đâu\n");
        printf("3. bạn bao nhiêu tuổi\n");
        printf("0. Thoát\n");
        printf("Nhập số: ");
        scanf("%d", &choice);

        if (choice == 0) break;
        if (choice < 1 || choice > 3) {
            printf("Vui lòng chọn lại.\n");
            continue;
        }

        float input[INPUT_SIZE] = {0};
        input[choice - 1] = 1;

        float output[OUTPUT_SIZE];
        forward(model, input, output);

        int max_index = 0;
        for (int i = 1; i < OUTPUT_SIZE; i++) {
            if (output[i] > output[max_index])
                max_index = i;
        }

        printf("→ Dự đoán: %s (%.2f)\n", answers[max_index], output[max_index]);
    }

    free_model(model);
    return 0;
}
/*
gcc -o test test.c linear_regression.c -lm
./test
*/