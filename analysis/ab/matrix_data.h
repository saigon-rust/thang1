#ifndef MATRIX_DATA_H
#define MATRIX_DATA_H

#include "sales_data.h"

#define MATRIX_COLS (3 + BASE_COLUMNS + NUM_CAMPAIGNS)

void matrix_data() {
    int matrix_data[SALES_DATA_COUNT][MATRIX_COLS];
    print_stats_header(NUM_CAMPAIGNS);

    for (int row = 0; row < SALES_DATA_COUNT; row++) {
        SalesData sale = sales_data[row];

        // Ghi 3 cột đầu: ProductID, Units, Revenue
        matrix_data[row][0] = sale.product_id;
        matrix_data[row][1] = sale.units;
        matrix_data[row][2] = sale.revenue;

        // Cập nhật chỉ số thời gian
        set_stats_row_base(row, sale.date);

        // Copy cột thời gian từ stats
        for (int col = 0; col < BASE_COLUMNS; col++) {
            matrix_data[row][3 + col] = stats[row][col];
        }

        // Ghi các chiến dịch
        for (int j = 0; j < NUM_CAMPAIGNS; j++) {
            if (is_date_in_campaign(sale.date, &campaigns[j])) {
                matrix_data[row][3 + BASE_COLUMNS + j] = 1;
            }
        }

        // In dòng
        for (int col = 0; col < MATRIX_COLS; col++) {
            printf("%d%s", matrix_data[row][col], (col < MATRIX_COLS - 1) ? "," : "\n");
        }
    }
}

#endif //MATRIX_DATA_H