#ifndef TOP_PRODUCTS_H
#define TOP_PRODUCTS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sales_stats.h"

typedef struct {
    int product_id;
    int total_units;
    int monthly_units[12]; // Số lượng theo từng tháng 1-12
} ProductSummary;

int compare_by_units_desc(const void* a, const void* b) {
    int diff = ((ProductSummary*)b)->total_units - ((ProductSummary*)a)->total_units;
    if (diff > 0) return 1;
    if (diff < 0) return -1;
    return 0;
}

typedef struct {
    int month;
    int units;
} MonthUnit;

int compare_month_units_desc(const void* a, const void* b) {
    return ((MonthUnit*)b)->units - ((MonthUnit*)a)->units;
}

int extract_month_number(const char* date) {
    int y, m, d;
    sscanf(date, "%d-%d-%d", &y, &m, &d);
    return m;
}

void summarize_top_products(SalesData* data, int count) {
    ProductSummary summaries[64] = {0};
    int summary_count = 0;

    for (int i = 0; i < count; i++) {
        int month = extract_month_number(data[i].date);
        int found = 0;

        for (int j = 0; j < summary_count; j++) {
            if (summaries[j].product_id == data[i].product_id) {
                summaries[j].total_units += data[i].units;
                summaries[j].monthly_units[month - 1] += data[i].units;
                found = 1;
                break;
            }
        }

        if (!found) {
            summaries[summary_count].product_id = data[i].product_id;
            summaries[summary_count].total_units = data[i].units;
            memset(summaries[summary_count].monthly_units, 0, sizeof(int) * 12);
            summaries[summary_count].monthly_units[month - 1] = data[i].units;
            summary_count++;
        }
    }

    qsort(summaries, summary_count, sizeof(ProductSummary), compare_by_units_desc);

    // In bảng
    printf("\n📦 Danh sách sản phẩm bán chạy (giảm dần theo tổng số lượng):\n");
    printf("-------------------------------------------------------------\n");
    printf("| %-6s| %-8s| Tháng bán chạy (giảm dần)            |\n", "Mã SP", "Tổng SL");
    printf("-------------------------------------------------------------\n");

    for (int i = 0; i < summary_count; i++) {
        // Tạo danh sách MonthUnit
        MonthUnit month_units[12];
        int month_count = 0;
        for (int m = 0; m < 12; m++) {
            if (summaries[i].monthly_units[m] > 0) {
                month_units[month_count].month = m + 1;
                month_units[month_count].units = summaries[i].monthly_units[m];
                month_count++;
            }
        }

        qsort(month_units, month_count, sizeof(MonthUnit), compare_month_units_desc);

        // In danh sách tháng
        printf("| %6d| %8d| ", summaries[i].product_id, summaries[i].total_units);
        for (int k = 0; k < month_count; k++) {
            printf("%d", month_units[k].month);
            if (k != month_count - 1) printf("-");
        }
        printf("\n");
    }

    printf("-------------------------------------------------------------\n");
}
//==============================
#include <time.h>

void top_products_last_days(SalesData* data, int count, int days) {
    if (days <= 0) {
        printf("⚠️ Số ngày phải lớn hơn 0.\n");
        return;
    }

    // Tìm ngày lớn nhất trong dữ liệu
    time_t max_date = 0;
    for (int i = 0; i < count; i++) {
        struct tm tm_data = {0};
        strptime(data[i].date, "%Y-%m-%d", &tm_data);
        time_t t = mktime(&tm_data);
        if (t > max_date) max_date = t;
    }

    time_t from_time = max_date - (time_t)(days * 24 * 60 * 60);

    ProductSummary summaries[64] = {0};
    int summary_count = 0;

    for (int i = 0; i < count; i++) {
        struct tm tm_data = {0};
        strptime(data[i].date, "%Y-%m-%d", &tm_data);
        time_t t = mktime(&tm_data);

        if (difftime(t, from_time) >= 0) {
            int month = tm_data.tm_mon;

            int found = 0;
            for (int j = 0; j < summary_count; j++) {
                if (summaries[j].product_id == data[i].product_id) {
                    summaries[j].total_units += data[i].units;
                    summaries[j].monthly_units[month] += data[i].units;
                    found = 1;
                    break;
                }
            }

            if (!found) {
                summaries[summary_count].product_id = data[i].product_id;
                summaries[summary_count].total_units = data[i].units;
                memset(summaries[summary_count].monthly_units, 0, sizeof(int) * 12);
                summaries[summary_count].monthly_units[month] = data[i].units;
                summary_count++;
            }
        }
    }

    if (summary_count == 0) {
        printf("\n❗ Không có sản phẩm nào bán trong %d ngày qua.\n", days);
        return;
    }

    qsort(summaries, summary_count, sizeof(ProductSummary), compare_by_units_desc);

    printf("\n📦 Top sản phẩm bán chạy trong %d ngày gần nhất:\n", days);
    printf("-------------------------------------------------------------\n");
    printf("| %-6s| %-8s| Tháng bán chạy (giảm dần)            |\n", "Mã SP", "Tổng SL");
    printf("-------------------------------------------------------------\n");

    for (int i = 0; i < summary_count; i++) {
        MonthUnit month_units[12];
        int month_count = 0;

        for (int m = 0; m < 12; m++) {
            if (summaries[i].monthly_units[m] > 0) {
                month_units[month_count].month = m + 1;
                month_units[month_count].units = summaries[i].monthly_units[m];
                month_count++;
            }
        }

        qsort(month_units, month_count, sizeof(MonthUnit), compare_month_units_desc);

        printf("| %6d| %8d| ", summaries[i].product_id, summaries[i].total_units);
        for (int k = 0; k < month_count; k++) {
            printf("%d", month_units[k].month);
            if (k != month_count - 1) printf("-");
        }
        printf("\n");
    }

    printf("-------------------------------------------------------------\n");
}

#endif // TOP_PRODUCTS_H
