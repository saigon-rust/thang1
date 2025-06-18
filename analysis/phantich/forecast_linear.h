#ifndef FORECAST_LINEAR_H
#define FORECAST_LINEAR_H

#include "sales_data.h"
#include "linear_regression.h"

void forecast_linear() {
    int MATRIX_COLS = 3 + BASE_COLUMNS + NUM_CAMPAIGNS;
    int matrix_data[SALES_DATA_COUNT][MATRIX_COLS];
    double X[SALES_DATA_COUNT][MATRIX_COLS - 3]; // đặc trưng
    double y[SALES_DATA_COUNT]; // doanh thu

    // Xây dựng matrix_data từ sales_data và stats
    for (int row = 0; row < SALES_DATA_COUNT; row++) {
        SalesData sale = sales_data[row];
        matrix_data[row][0] = sale.product_id;
        matrix_data[row][1] = sale.units;
        matrix_data[row][2] = sale.revenue;

        set_stats_row_base(row, sale.date);

        for (int col = 0; col < BASE_COLUMNS; col++) {
            matrix_data[row][3 + col] = stats[row][col];
        }

        for (int j = 0; j < NUM_CAMPAIGNS; j++) {
            if (is_date_in_campaign(sale.date, &campaigns[j])) {
                matrix_data[row][3 + BASE_COLUMNS + j] = 1;
            } else {
                matrix_data[row][3 + BASE_COLUMNS + j] = 0;
            }
        }
    }

    // Chuyển dữ liệu sang X, y cho huấn luyện
    for (int i = 0; i < SALES_DATA_COUNT; i++) {
        y[i] = (double)matrix_data[i][2];
        for (int j = 3; j < MATRIX_COLS; j++) {
            X[i][j - 3] = (double)matrix_data[i][j];
        }
    }

    // Khởi tạo mô hình
    LinearModel model;
    init_model(&model, SALES_DATA_COUNT, MATRIX_COLS - 3);

    for (int i = 0; i < SALES_DATA_COUNT; i++) {
        for (int j = 0; j < MATRIX_COLS - 3; j++) {
            model.X[i][j] = X[i][j];
        }
        model.y[i] = y[i];
    }

    // Huấn luyện mô hình
    train(&model);

    // Dự đoán thử
    double test[MATRIX_COLS - 3];
    for (int i = 0; i < MATRIX_COLS - 3; i++) test[i] = 0.0;

    // In trọng số
    print_weights(&model);

    double result = predict(&model, test);
    printf("📈 Dự đoán doanh thu cho đầu vào giả định: %.2f\n", result);


    //-----------------
        // ====== Dự đoán doanh thu N ngày tới với chiến dịch ID ======
    int n_days, campaign_id;
    printf("Nhập số ngày muốn dự đoán và ID chiến dịch (VD: 30 2, hoặc 30 0 nếu không có): ");
    if (scanf("%d %d", &n_days, &campaign_id) != 2 || n_days <= 0) {
        fprintf(stderr, "⚠️  Dữ liệu đầu vào không hợp lệ.\n");
        exit(EXIT_FAILURE);
    }

    if (campaign_id < 0 || campaign_id > NUM_CAMPAIGNS) {
        fprintf(stderr, "⚠️  ID chiến dịch không hợp lệ.\n");
        exit(EXIT_FAILURE);
    }

    time_t now = time(NULL);
    struct tm today = *localtime(&now);

    double total_revenue = 0.0;

    printf("\n📊 Dự đoán doanh thu trong %d ngày tới:\n", n_days);
    printf("Ngày         | Doanh thu (dự đoán)\n");
    printf("-----------------------------------\n");

    for (int i = 0; i < n_days; i++) {
        struct tm future = today;
        future.tm_mday += i;
        mktime(&future); // normalize

        char date_str[11];
        strftime(date_str, sizeof(date_str), "%Y-%m-%d", &future);

        // Tạo feature vector
        double test[MATRIX_COLS - 3];
        for (int j = 0; j < MATRIX_COLS - 3; j++) test[j] = 0.0;

        int w, d, m, q;
        get_time_indices(date_str, &w, &d, &m, &q);
        if (w >= 0) test[w] = 1.0;
        if (d >= 0) test[DAYS_IN_WEEK + d] = 1.0;
        if (m >= 0) test[DAYS_IN_WEEK + DAYS_IN_MONTH + m] = 1.0;
        if (q >= 0) test[DAYS_IN_WEEK + DAYS_IN_MONTH + MONTHS_IN_YEAR + q] = 1.0;

        if (campaign_id > 0) {
            if (is_date_in_campaign(date_str, &campaigns[campaign_id - 1])) {
                test[BASE_COLUMNS + campaign_id - 1] = 1.0;
            }
        }

        double predicted = predict(&model, test);
        total_revenue += predicted;
        printf("%s | %.2f\n", date_str, predicted);
    }

    printf("-----------------------------------\n");
    printf("📈 Tổng doanh thu dự đoán: %.2f\n", total_revenue);
}
#endif //FORECAST_LINEAR_H
//forecast_linear();
