#ifndef FRAUD_MODEL_H
#define FRAUD_MODEL_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define MAX_FEATURES 32
#define MAX_SAMPLES  1000
#define EPOCHS       1000
#define LEARNING_RATE 0.01

typedef struct {
    int num_samples;
    int num_features;
    double X[MAX_SAMPLES][MAX_FEATURES];  // ma trận đầu vào
    int    y[MAX_SAMPLES];                // nhãn: 0 (bình thường), 1 (gian lận)
    double weights[MAX_FEATURES];         // trọng số
    double bias;
} FraudModel;

// Hàm sigmoid
double sigmoid(double z) {
    return 1.0 / (1.0 + exp(-z));
}

// Hàm khởi tạo
void init_fraud_model(FraudModel* model, int num_samples, int num_features) {
    model->num_samples = num_samples;
    model->num_features = num_features;
    for (int i = 0; i < num_features; i++) {
        model->weights[i] = 0.0;
    }
    model->bias = 0.0;
}

// forward: tính xác suất dự đoán cho dòng i
double forward_fraud(FraudModel* model, int i) {
    double z = model->bias;
    for (int j = 0; j < model->num_features; j++) {
        z += model->X[i][j] * model->weights[j];
    }
    return sigmoid(z);
}

// backward: cập nhật trọng số qua gradient descent
void backward_fraud(FraudModel* model) {
    double dw[MAX_FEATURES] = {0};
    double db = 0.0;

    for (int i = 0; i < model->num_samples; i++) {
        double y_pred = forward_fraud(model, i);
        double error = y_pred - model->y[i];

        for (int j = 0; j < model->num_features; j++) {
            dw[j] += error * model->X[i][j];
        }
        db += error;
    }

    for (int j = 0; j < model->num_features; j++) {
        model->weights[j] -= LEARNING_RATE * dw[j] / model->num_samples;
    }
    model->bias -= LEARNING_RATE * db / model->num_samples;
}

// train: huấn luyện mô hình
void train_fraud(FraudModel* model) {
    for (int epoch = 0; epoch < EPOCHS; epoch++) {
        backward_fraud(model);

        if (epoch % 100 == 0) {
            double loss = 0.0;
            for (int i = 0; i < model->num_samples; i++) {
                double y_pred = forward_fraud(model, i);
                double label = model->y[i];
                loss += -label * log(y_pred + 1e-9) - (1 - label) * log(1 - y_pred + 1e-9);
            }
            printf("Epoch %d: Loss = %.4f\n", epoch, loss / model->num_samples);
        }
    }
}

// predict: trả về nhãn (0 hoặc 1) từ 1 dòng input mới
int predict_fraud(FraudModel* model, const double* input) {
    double z = model->bias;
    for (int i = 0; i < model->num_features; i++) {
        z += input[i] * model->weights[i];
    }
    return sigmoid(z) >= 0.5 ? 1 : 0;
}

// xác suất phân loại (dùng cho kiểm tra)
double predict_prob(FraudModel* model, const double* input) {
    double z = model->bias;
    for (int i = 0; i < model->num_features; i++) {
        z += input[i] * model->weights[i];
    }
    return sigmoid(z);
}

// in trọng số
void print_fraud_weights(FraudModel* model) {
    printf("\n🎯 Trọng số mô hình:\n");
    for (int i = 0; i < model->num_features; i++) {
        printf("  w[%d] = %.4f\n", i, model->weights[i]);
    }
    printf("  bias = %.4f\n\n", model->bias);
}

#endif // FRAUD_MODEL_H
