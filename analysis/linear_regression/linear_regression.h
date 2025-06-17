#ifndef LINEAR_REGRESSION_H
#define LINEAR_REGRESSION_H

#include <stdlib.h>

typedef struct {
    int input_size;       // Số chiều đầu vào
    int output_size;      // Số lớp đầu ra
    int epochs;           // Số vòng lặp huấn luyện
    float learning_rate;  // Tốc độ học
    float** W;            // Trọng số (input_size x output_size)
    float* b;             // Độ dời (bias) (output_size)
} LinearRegression;

// Tạo và giải phóng model
LinearRegression* create_model(int input_size, int output_size, int epochs, float learning_rate);
void free_model(LinearRegression* model);

// Các hàm tuyến tính và kích hoạt
float* linear(LinearRegression* model, float* x);                        // x: [input_size] → z: [output_size]
float* sigmoid(LinearRegression* model, float* x);                       // sigmoid(xW + b)
float* softmax(float* z, int size);                                      // softmax đầu ra

// Dự đoán đầu ra (forward)
void forward(LinearRegression* model, float* x, float* out);            // linear + softmax
void forward_s(LinearRegression* model, float* x, float* out);          // sigmoid + softmax

// Tính gradient và cập nhật trọng số (backpropagation)
void backward(LinearRegression* model, float* x, float* y_true, float* output_probs);

// Huấn luyện model
void train(LinearRegression* model, float** X_train, float** y_train, int n_samples);

// Lưu và tải mô hình
void save_model(LinearRegression* model, const char* filename);         // Ghi xuống file nhị phân .dat
LinearRegression* load_model(const char* filename);                     // Đọc file mô hình đã lưu

#endif // LINEAR_REGRESSION_H
