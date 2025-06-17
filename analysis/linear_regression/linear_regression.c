#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "linear_regression.h"

float random_weight() {
    return ((float)rand() / RAND_MAX) * 0.02f - 0.01f; // [-0.01, 0.01]
}

LinearRegression* create_model(int input_size, int output_size, int epochs, float learning_rate) {
    LinearRegression* model = (LinearRegression*)malloc(sizeof(LinearRegression));
    model->input_size = input_size;
    model->output_size = output_size;
    model->epochs = epochs;
    model->learning_rate = learning_rate;

    // Cấp phát trọng số W
    model->W = (float**)malloc(input_size * sizeof(float*));
    for (int i = 0; i < input_size; i++) {
        model->W[i] = (float*)malloc(output_size * sizeof(float));
        for (int j = 0; j < output_size; j++) {
            model->W[i][j] = random_weight();
        }
    }

    // Cấp phát bias
    model->b = (float*)calloc(output_size, sizeof(float));

    return model;
}

void free_model(LinearRegression* model) {
    for (int i = 0; i < model->input_size; i++) {
        free(model->W[i]);
    }
    free(model->W);
    free(model->b);
    free(model);
}

float* linear(LinearRegression* model, float* x) {
    float* z = (float*)malloc(model->output_size * sizeof(float));
    for (int j = 0; j < model->output_size; j++) {
        z[j] = model->b[j];
        for (int i = 0; i < model->input_size; i++) {
            z[j] += x[i] * model->W[i][j];
        }
    }
    return z;
}

float* sigmoid(LinearRegression* model, float* x) {
    float* z = linear(model, x);
    for (int i = 0; i < model->output_size; i++) {
        z[i] = 1.0f / (1.0f + expf(-z[i]));
    }
    return z;
}

float* softmax(float* z, int size) {
    float* result = (float*)malloc(size * sizeof(float));
    float max = z[0];
    for (int i = 1; i < size; i++) if (z[i] > max) max = z[i];

    float sum = 0.0f;
    for (int i = 0; i < size; i++) {
        result[i] = expf(z[i] - max);
        sum += result[i];
    }

    for (int i = 0; i < size; i++) {
        result[i] /= sum;
    }
    return result;
}

void forward(LinearRegression* model, float* x, float* out) {
    float* z = linear(model, x);
    float* probs = softmax(z, model->output_size);
    memcpy(out, probs, model->output_size * sizeof(float));
    free(z);
    free(probs);
}

void forward_s(LinearRegression* model, float* x, float* out) {
    float* z = sigmoid(model, x);
    float* probs = softmax(z, model->output_size);
    memcpy(out, probs, model->output_size * sizeof(float));
    free(z);
    free(probs);
}

void backward(LinearRegression* model, float* x, float* y_true, float* output_probs) {
    float* dz = (float*)malloc(model->output_size * sizeof(float));
    for (int i = 0; i < model->output_size; i++) {
        dz[i] = output_probs[i] - y_true[i];
    }

    for (int i = 0; i < model->input_size; i++) {
        for (int j = 0; j < model->output_size; j++) {
            model->W[i][j] -= model->learning_rate * x[i] * dz[j];
        }
    }

    for (int j = 0; j < model->output_size; j++) {
        model->b[j] -= model->learning_rate * dz[j];
    }

    free(dz);
}

void train(LinearRegression* model, float** X_train, float** y_train, int n_samples) {
    float* output = (float*)malloc(model->output_size * sizeof(float));
    for (int epoch = 0; epoch < model->epochs; epoch++) {
        for (int i = 0; i < n_samples; i++) {
            forward(model, X_train[i], output);
            backward(model, X_train[i], y_train[i], output);
            forward_s(model, X_train[i], output);
            backward(model, X_train[i], y_train[i], output);
        }
        if ((epoch + 1) % 100 == 0) {
            printf("Epoch %d/%d done\n", epoch + 1, model->epochs);
        }
    }
    free(output);
}

void save_model(LinearRegression* model, const char* filename) {
    FILE* f = fopen(filename, "wb");
    if (!f) {
        perror("save_model");
        return;
    }

    fwrite(&model->input_size, sizeof(int), 1, f);
    fwrite(&model->output_size, sizeof(int), 1, f);
    for (int i = 0; i < model->input_size; i++) {
        fwrite(model->W[i], sizeof(float), model->output_size, f);
    }
    fwrite(model->b, sizeof(float), model->output_size, f);
    fclose(f);
}

LinearRegression* load_model(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        perror("load_model");
        return NULL;
    }

    int input_size, output_size;
    fread(&input_size, sizeof(int), 1, f);
    fread(&output_size, sizeof(int), 1, f);
    LinearRegression* model = create_model(input_size, output_size, 1, 0.01f); // dummy epoch/lr

    for (int i = 0; i < input_size; i++) {
        fread(model->W[i], sizeof(float), output_size, f);
    }
    fread(model->b, sizeof(float), output_size, f);
    fclose(f);
    return model;
}
