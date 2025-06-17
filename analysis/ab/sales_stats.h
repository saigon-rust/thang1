#ifndef SALES_STATS_H
#define SALES_STATS_H

#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DAYS_IN_WEEK       7
#define DAYS_IN_MONTH     31
#define MONTHS_IN_YEAR    12
#define QUARTERS_IN_YEAR   4
#define MAX_CAMPAIGNS    100

#define BASE_COLUMNS  (DAYS_IN_WEEK + DAYS_IN_MONTH + MONTHS_IN_YEAR + QUARTERS_IN_YEAR)
#define STATS_ROWS     1000

typedef struct {
    int product_id;
    char date[11];  // yyyy-mm-dd
    int units;
    int revenue;
} SalesData;

typedef struct {
    int campaign_id;
    char date_start[11];
    char date_end[11];
    char campaign[64];
} SaleRecord;

int stats[STATS_ROWS][BASE_COLUMNS + MAX_CAMPAIGNS] = {0};

// ==== Hàm hỗ trợ thời gian ====

int is_date_in_campaign(const char* date_str, const SaleRecord* record) {
    return strcmp(date_str, record->date_start) >= 0 &&
           strcmp(date_str, record->date_end) <= 0;
}

void get_time_indices(const char* date_str, int* weekday_idx, int* day_idx, int* month_idx, int* quarter_idx) {
    struct tm timeinfo = {0};
    if (strptime(date_str, "%Y-%m-%d", &timeinfo) == NULL) {
        fprintf(stderr, "⚠️  Invalid date: %s\n", date_str);
        *weekday_idx = *day_idx = *month_idx = *quarter_idx = -1;
        return;
    }

    mktime(&timeinfo);
    *weekday_idx = timeinfo.tm_wday;
    *day_idx     = timeinfo.tm_mday - 1;
    *month_idx   = timeinfo.tm_mon;
    *quarter_idx = timeinfo.tm_mon / 3;
}

void set_stats_row_base(int row, const char* date_str) {
    int w, d, m, q;
    get_time_indices(date_str, &w, &d, &m, &q);
    if (w < 0 || d < 0 || m < 0 || q < 0 || row >= STATS_ROWS) return;

    stats[row][w]                         = 1;
    stats[row][DAYS_IN_WEEK + d]         = 1;
    stats[row][DAYS_IN_WEEK + DAYS_IN_MONTH + m] = 1;
    stats[row][DAYS_IN_WEEK + DAYS_IN_MONTH + MONTHS_IN_YEAR + q] = 1;
}

void print_stats_header(int num_campaigns) {
    printf("ProductID,Units,Revenue,");
    for (int i = 0; i < DAYS_IN_WEEK; i++)        printf("WDAY_%d,", i);
    for (int i = 0; i < DAYS_IN_MONTH; i++)       printf("DAY_%02d,", i+1);
    for (int i = 0; i < MONTHS_IN_YEAR; i++)      printf("MONTH_%02d,", i+1);
    for (int i = 0; i < QUARTERS_IN_YEAR; i++)    printf("Q%d,", i+1);
    for (int i = 0; i < num_campaigns; i++)       printf("IN_CAMPAIGN_%d%c", i+1, i == num_campaigns - 1 ? '\n' : ',');
}

#endif // SALES_STATS_H
