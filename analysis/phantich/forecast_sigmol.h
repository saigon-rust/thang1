#ifndef FORECAST_SIGMOL_H
#define FORECAST_SIGMOL_H

#include "logistic_regression.h"

void forecast_sigmol() {
    FraudModel model;
    int num_samples = 4;
    int num_features = 3;

    // === Dữ liệu mẫu ===
    double sample_X[4][3] = {
        {100.0, 0.0, 0.1},  // bình thường
        {900.0, 1.0, 0.9},  // gian lận
        {850.0, 0.0, 0.85}, // gian lận
        {50.0,  1.0, 0.0}   // bình thường
    };
    int sample_y[4] = {0, 1, 1, 0};

    init_fraud_model(&model, num_samples, num_features);

    // copy dữ liệu vào model
    for (int i = 0; i < num_samples; i++) {
        for (int j = 0; j < num_features; j++) {
            model.X[i][j] = sample_X[i][j];
        }
        model.y[i] = sample_y[i];
    }

    // train
    train_fraud(&model);
    print_fraud_weights(&model);

    // test thử input mới
    double new_input[3] = {600.0, 1.0, 0.7};
    double prob = predict_prob(&model, new_input);
    int label = predict_fraud(&model, new_input);

    printf("Xác suất gian lận: %.2f => Dự đoán: %s\n", prob, label ? "GIAN LẬN" : "BÌNH THƯỜNG");
    return 0;
}
#endif //FORECAST_SIGMOL_H

//forecast_sigmol();
//gcc main.c -o main -lm

