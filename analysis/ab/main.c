#include "sales_data.h"

int main() {
    print_stats_header(NUM_CAMPAIGNS);
    
    for (int row = 0; row < SALES_DATA_COUNT; row++) {
        SalesData sale = sales_data[row];

        // Ghi 3 cột đầu: ProductID, Units, Revenue
        matrix_data[row][0] = sale.product_id;
        matrix_data[row][1] = sale.units;
        matrix_data[row][2] = sale.revenue;

        // Ghi các cột thời gian (gọi hàm như stats[])
        set_stats_row_base(row, sale.date);
        for (int col = 0; col < BASE_COLUMNS; col++) {
            matrix_data[row][3 + col] = stats[row][col];  // offset 3
        }

        // Ghi chiến dịch
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

    return 0;
}
