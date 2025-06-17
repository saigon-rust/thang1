#ifndef ANALYZE_REVENUE_H
#define ANALYZE_REVENUE_H

#include "sales_stats.h"

// Tổng hợp doanh thu theo ngày trong tháng (1–31)
void analyze_revenue_by_day_of_month(SalesData* data, int count) {
    int sum[DAYS_IN_MONTH] = {0};
    for (int i = 0; i < count; i++) {
        struct tm t = {0};
        if (strptime(data[i].date, "%Y-%m-%d", &t)) {
            int day = t.tm_mday - 1;
            if (day >= 0 && day < DAYS_IN_MONTH) {
                sum[day] += data[i].revenue;
            }
        }
    }

    printf("📊 Doanh thu theo ngày trong tháng:\n");
    for (int i = 0; i < DAYS_IN_MONTH; i++) {
        if (sum[i] > 0) {
            printf("  Ngày %2d: %d\n", i + 1, sum[i]);
        }
    }
    printf("\n");
}

// Tổng hợp doanh thu theo thứ trong tuần (0=Chủ Nhật ... 6=Thứ Bảy)
void analyze_revenue_by_weekday(SalesData* data, int count) {
    int sum[DAYS_IN_WEEK] = {0};
    for (int i = 0; i < count; i++) {
        struct tm t = {0};
        if (strptime(data[i].date, "%Y-%m-%d", &t)) {
            mktime(&t);  // để có tm_wday
            sum[t.tm_wday] += data[i].revenue;
        }
    }

    const char* wday_names[] = {"Chủ Nhật", "Thứ 2", "Thứ 3", "Thứ 4", "Thứ 5", "Thứ 6", "Thứ 7"};
    printf("📊 Doanh thu theo thứ trong tuần:\n");
    for (int i = 0; i < DAYS_IN_WEEK; i++) {
        printf("  %-9s: %d\n", wday_names[i], sum[i]);
    }
    printf("\n");
}

// Tổng hợp doanh thu theo tháng trong năm (1–12)
void analyze_revenue_by_month(SalesData* data, int count) {
    int sum[MONTHS_IN_YEAR] = {0};
    for (int i = 0; i < count; i++) {
        struct tm t = {0};
        if (strptime(data[i].date, "%Y-%m-%d", &t)) {
            int month = t.tm_mon;
            sum[month] += data[i].revenue;
        }
    }

    printf("📊 Doanh thu theo tháng:\n");
    for (int i = 0; i < MONTHS_IN_YEAR; i++) {
        if (sum[i] > 0) {
            printf("  Tháng %2d: %d\n", i + 1, sum[i]);
        }
    }
    printf("\n");
}

// Tổng hợp doanh thu theo quý trong năm (Q1–Q4)
void analyze_revenue_by_quarter(SalesData* data, int count) {
    int sum[QUARTERS_IN_YEAR] = {0};
    for (int i = 0; i < count; i++) {
        struct tm t = {0};
        if (strptime(data[i].date, "%Y-%m-%d", &t)) {
            int quarter = t.tm_mon / 3;
            sum[quarter] += data[i].revenue;
        }
    }

    printf("📊 Doanh thu theo quý:\n");
    for (int i = 0; i < QUARTERS_IN_YEAR; i++) {
        printf("  Q%d: %d\n", i + 1, sum[i]);
    }
    printf("\n");
}

// Hàm tổng hợp doanh thu theo từng năm có trong dữ liệu
void analyze_revenue_by_year(SalesData* data, int count) {
    // Giả sử dữ liệu không quá 100 năm
    int year_min = 9999, year_max = 0;
    int year_sum[100] = {0};  // Lưu tổng doanh thu theo chỉ số offset từ year_min

    for (int i = 0; i < count; i++) {
        struct tm t = {0};
        if (strptime(data[i].date, "%Y-%m-%d", &t)) {
            int year = t.tm_year + 1900;
            if (year < year_min) year_min = year;
            if (year > year_max) year_max = year;
        }
    }

    // Nếu không có năm hợp lệ
    if (year_max < year_min) {
        printf("⚠️  Không có dữ liệu năm hợp lệ.\n");
        return;
    }

    // Tổng hợp doanh thu
    for (int i = 0; i < count; i++) {
        struct tm t = {0};
        if (strptime(data[i].date, "%Y-%m-%d", &t)) {
            int year = t.tm_year + 1900;
            int index = year - year_min;
            year_sum[index] += data[i].revenue;
        }
    }

    // In kết quả
    printf("📅 Doanh thu theo năm:\n");
    for (int y = year_min; y <= year_max; y++) {
        int idx = y - year_min;
        if (year_sum[idx] > 0) {
            printf("  Năm %d: %d\n", y, year_sum[idx]);
        }
    }
    printf("\n");
}

#endif //ANALYZE_REVENUE_H