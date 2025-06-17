//forecast_calculate_average.h
#ifndef FORECAST_H
#define FORECAST_H

#include <limits.h>

double calculate_average_daily_revenue(SalesData* data, int count) {
    if (count == 0) return 0.0;

    int total_revenue = 0;
    time_t min_date = LONG_MAX, max_date = 0;

    for (int i = 0; i < count; i++) {
        struct tm t = {0};
        if (strptime(data[i].date, "%Y-%m-%d", &t)) {
            time_t time_val = mktime(&t);
            if (time_val < min_date) min_date = time_val;
            if (time_val > max_date) max_date = time_val;
            total_revenue += data[i].revenue;
        }
    }

    double days_range = difftime(max_date, min_date) / (60 * 60 * 24);
    if (days_range <= 0) days_range = 1; // Tránh chia 0

    return total_revenue / days_range;
}

void forecast_next_week(SalesData* data, int count) {
    double avg = calculate_average_daily_revenue(data, count);
    printf("📈 Dự báo doanh thu 1 tuần tới: %.2f\n", avg * 7);
}

void forecast_next_month(SalesData* data, int count) {
    double avg = calculate_average_daily_revenue(data, count);
    printf("📈 Dự báo doanh thu 1 tháng tới: %.2f\n", avg * 30);
}

void forecast_next_quarter(SalesData* data, int count) {
    double avg = calculate_average_daily_revenue(data, count);
    printf("📈 Dự báo doanh thu 1 quý tới (3 tháng): %.2f\n", avg * 90);
}

void forecast_next_year(SalesData* data, int count) {
    double avg = calculate_average_daily_revenue(data, count);
    printf("📈 Dự báo doanh thu 1 năm tới: %.2f\n", avg * 365);
}

void forecast_next_n_years(SalesData* data, int count, int years) {
    if (years <= 0) return;
    double avg = calculate_average_daily_revenue(data, count);
    printf("📈 Dự báo doanh thu %d năm tới: %.2f\n", years, avg * 365 * years);
}

#endif //FORECAST_H

// forecast_next_week(sales_data, SALES_DATA_COUNT);
// forecast_next_month(sales_data, SALES_DATA_COUNT);
// forecast_next_quarter(sales_data, SALES_DATA_COUNT);
// forecast_next_year(sales_data, SALES_DATA_COUNT);
// forecast_next_n_years(sales_data, SALES_DATA_COUNT, 5);