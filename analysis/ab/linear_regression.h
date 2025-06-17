//linear_regression.h
#ifndef LINEAR_REGRESSION_H
#define LINEAR_REGRESSION_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define MAX_FEATURES  64      // số chiều đầu vào (thay đổi theo input)
#define MAX_SAMPLES   100     // số dòng dữ liệu tối đa
#define EPOCHS        1000
#define LEARNING_RATE 0.01

typedef struct {
    int num_samples;
    int num_features;
    double X[MAX_SAMPLES][MAX_FEATURES]; // dữ liệu đầu vào (không gồm bias)
    double y[MAX_SAMPLES];               // nhãn (doanh thu)
    double weights[MAX_FEATURES];        // trọng số
    double bias;
} LinearModel;

// Hàm khởi tạo
void init_model(LinearModel* model, int num_samples, int num_features) {
    model->num_samples = num_samples;
    model->num_features = num_features;
    for (int i = 0; i < num_features; i++) {
        model->weights[i] = 0.0;
    }
    model->bias = 0.0;
}

// forward: dự đoán đầu ra cho mẫu thứ i
double forward(LinearModel* model, int i) {
    double result = model->bias;
    for (int j = 0; j < model->num_features; j++) {
        result += model->X[i][j] * model->weights[j];
    }
    return result;
}

// backward: cập nhật trọng số dựa trên gradient
void backward(LinearModel* model) {
    double dw[MAX_FEATURES] = {0};
    double db = 0.0;

    for (int i = 0; i < model->num_samples; i++) {
        double y_pred = forward(model, i);
        double error = y_pred - model->y[i];

        for (int j = 0; j < model->num_features; j++) {
            dw[j] += error * model->X[i][j];
        }
        db += error;
    }

    // Gradient descent
    for (int j = 0; j < model->num_features; j++) {
        model->weights[j] -= LEARNING_RATE * dw[j] / model->num_samples;
    }
    model->bias -= LEARNING_RATE * db / model->num_samples;
}

// Hàm huấn luyện
void train(LinearModel* model) {
    for (int epoch = 0; epoch < EPOCHS; epoch++) {
        backward(model);

        // Theo dõi loss sau mỗi 100 epochs
        if (epoch % 100 == 0) {
            double loss = 0.0;
            for (int i = 0; i < model->num_samples; i++) {
                double y_pred = forward(model, i);
                double error = y_pred - model->y[i];
                loss += error * error;
            }
            printf("Epoch %d: Loss = %.2f\n", epoch, loss / model->num_samples);
        }
    }
}

// Dự đoán đầu ra từ 1 vector đặc trưng mới
double predict(LinearModel* model, const double* input_features) {
    double result = model->bias;
    for (int i = 0; i < model->num_features; i++) {
        result += input_features[i] * model->weights[i];
    }
    return result;
}

// In trọng số
void print_weights(LinearModel* model) {
    printf("\nModel Weights:\n");
    for (int i = 0; i < model->num_features; i++) {
        printf("  w[%d] = %.4f\n", i, model->weights[i]);
    }
    printf("  bias = %.4f\n\n", model->bias);
}

#endif // LINEAR_REGRESSION_H
