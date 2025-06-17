#define _XOPEN_SOURCE 700  // Đảm bảo hỗ trợ hàm strptime
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Định nghĩa các hằng số cho phân tích thống kê
#define DAYS_IN_WEEK   7
#define DAYS_IN_MONTH 31
#define MONTHS_IN_YEAR 12
#define QUARTERS_IN_YEAR 4

#define D1 0x01;
#define D2 0x02;
#define D3 0x03;
#define D4 0x04;
#define D5 0x05;
#define D6 0x06;
#define D7 0x07;
#define D8 0x08;
#define D9 0x09;
#define D10 0x10;
#define D11 0x11;
#define D12 0x12;
#define D13 0x13;
#define D14 0x14;
#define D15 0x15;
#define D16 0x16;
#define D17 0x17;
#define D18 0x18;
#define D19 0x19;
#define D20 0x20;
#define D21 0x21;
#define D22 0x22;
#define D23 0x23;
#define D24 0x24;
#define D25 0x25;
#define D26 0x26;
#define D27 0x27;
#define D28 0x28;
#define D29 0x29;
#define D30 0x30;
#define D31 0x31;

#define MON 0x32;
#define TUE 0x33;
#define WED 0x34;
#define THU 0x35;
#define FRI 0x36;
#define SAT 0x37;
#define SUN 0x38;

#define JAN 0x41;
#define FEB 0x42;
#define MAR 0x43;
#define APR 0x44;
#define MAY 0x45;
#define JUN 0x46;
#define JUL 0x47;
#define AUG 0x48;
#define SEP 0x49;
#define OCT 0x50;
#define NOV 0x51;
#define DEC 0x52;

#define Q1 0x61;
#define Q2 0x62;
#define Q3 0x63;
#define Q4 0x64;

typedef enum {DAY_1, DAY_2, DAY_3, DAY_4, DAY_5, DAY_6, DAY_7, DAY_8, DAY_9, DAY_10,
    DAY_11, DAY_12, DAY_13, DAY_14, DAY_15, DAY_16, DAY_17, DAY_18, DAY_19, DAY_20,
    DAY_21, DAY_22, DAY_23, DAY_24, DAY_25, DAY_26, DAY_27, DAY_28, DAY_29, DAY_30, DAY_31
} Days_in_month;

typedef enum {SUNDAY, MONDAY, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY, SATURDAY
} Days_in_week;

typedef enum {JANUARY, FEBRUARY, MARCH, APRIL, MAY, JUNE, JULY, AUGUST, SEPTEMBER, OCTOBER, NOVEMBER, DECEMBER
} Months_in_year;

typedef enum {Q1, Q2, Q3, Q4
} Quarters_in_year;

typedef struct {
    int campaign_id
    char date_start[11];          // Định dạng "DD/MM/YYYY"
    char date_end[11];
    char campaign[64];      // Tên chiến dịch bán hàng
} SaleRecord;
SaleRecord sale_record = {1, "2024-01-10","2024-01-20","khuyến mãi lớn"},{2, "2024-01-10","2024-01-15","khuyến mãi nhỏ"};

int STATS_COLS = DAYS_IN_WEEK+ DAYS_IN_MONTH+ MONTHS_IN_YEAR+ QUARTERS_IN_YEAR;

typedef struct {
    Days_in_week weekday;
    Days_in_month day;
    Months_in_year month;
    Quarters_in_year quarter;
} Stats;

typedef stats[STATS_ROWS][STATS_COLS];

void set_stats(const char* date_str){
    for (int r = 0; r < STATS_ROWS; r++) {
        for (int c = 0; c < STATS_COLS; c++) {
            stats[r][c] = (Stats){};
        }
    }
    stats[r] = (Stats) {weekday, day, month, quarter}
    return stats
}

stats = (Stats) {}

// Biến thống kê toàn cục
int weekday_stats[DAYS_IN_WEEK] = {0};
int daily_stats[DAYS_IN_MONTH] = {0};
int monthly_stats[MONTHS_IN_YEAR] = {0};
int quarterly_stats[QUARTERS_IN_YEAR] = {0};