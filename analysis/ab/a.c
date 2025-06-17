#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_LINE 512
#define MAX_CAMPAIGN_LEN 64
#define DAYS_IN_WEEK 7
#define MONTHS_IN_YEAR 12

typedef struct {
    char date[11];         // Định dạng: YYYY-MM-DD
    char campaign[MAX_CAMPAIGN_LEN];
    int total;
} SaleRecord;

// Mảng doanh thu theo thứ trong tuần
int weekday_revenue[DAYS_IN_WEEK] = {0}; // Từ chủ nhật (0) đến thứ bảy (6)
int month_revenue[MONTHS_IN_YEAR] = {0}; // Từ tháng 1 (0) đến tháng 12 (11)

// Phân tích ngày trong tuần
int get_weekday(const char* date_str) {
    struct tm timeinfo = {0};
    strptime(date_str, "%Y-%m-%d", &timeinfo);
    mktime(&timeinfo);
    return timeinfo.tm_wday; // 0 = Chủ Nhật, 1 = Thứ 2, ..., 6 = Thứ 7
}

// Phân tích tháng trong năm
int get_month(const char* date_str) {
    struct tm timeinfo = {0};
    strptime(date_str, "%Y-%m-%d", &timeinfo);
    mktime(&timeinfo);
    return timeinfo.tm_mon; // 0 = Tháng 1, ..., 11 = Tháng 12
}

// Đọc file CSV và phân tích
void analyze_sales(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Không thể mở file");
        return;
    }

    char line[MAX_LINE];
    fgets(line, sizeof(line), file); // Bỏ qua dòng header

    while (fgets(line, sizeof(line), file)) {
        SaleRecord record;
        char* token;
        int column = 0;
        int value;
        char* ptr = strtok(line, ",");

        // Lấy dữ liệu từng cột
        while (ptr) {
            switch (column) {
                case 6: // date
                    strncpy(record.date, ptr, sizeof(record.date));
                    break;
                case 7: // campaign
                    strncpy(record.campaign, ptr, sizeof(record.campaign));
                    break;
                case 8: // total
                    record.total = atoi(ptr);
                    break;
            }
            ptr = strtok(NULL, ",");
            column++;
        }

        // Phân tích theo ngày trong tuần
        int weekday = get_weekday(record.date);
        weekday_revenue[weekday] += record.total;

        // Phân tích theo tháng
        int month = get_month(record.date);
        month_revenue[month] += record.total;
    }

    fclose(file);
}

// In kết quả phân tích
void print_results() {
    const char* weekdays[] = {"Chủ Nhật", "Thứ Hai", "Thứ Ba", "Thứ Tư", "Thứ Năm", "Thứ Sáu", "Thứ Bảy"};
    printf("📅 Doanh thu theo ngày trong tuần:\n");
    for (int i = 0; i < DAYS_IN_WEEK; i++) {
        printf("  - %s: %d VND\n", weekdays[i], weekday_revenue[i]);
    }

    printf("\n📆 Doanh thu theo tháng:\n");
    for (int i = 0; i < MONTHS_IN_YEAR; i++) {
        if (month_revenue[i] > 0)
            printf("  - Tháng %d: %d VND\n", i + 1, month_revenue[i]);
    }
}

int main() {
    analyze_sales("sales.csv");
    print_results();
    return 0;
}
