#ifndef SEASONALITY_MODEL_H
#define SEASONALITY_MODEL_H

#include "sales_data.h"
#include <math.h>

#define MAX_TIME_INDEX 64

typedef struct {
    int count;
    int total_units;
    int total_revenue;
} TimeSlotStats;

typedef struct {
    int id;
    char name[64];
    int count_in;
    int revenue_in;
    int count_out;
    int revenue_out;
    double uplift_ratio; // (in - out) / out
} CampaignEffectiveness;

CampaignEffectiveness effectiveness[CAMPAIGNS_COLUMNS];
TimeSlotStats weekday_stats[DAYS_IN_WEEK] = {0};
TimeSlotStats month_stats[MONTHS_IN_YEAR] = {0};
TimeSlotStats quarter_stats[QUARTERS_IN_YEAR] = {0};

//==== Tính thống kê theo mùa ====
void analyze_seasonality() {
    for (int i = 0; i < SALES_DATA_COUNT; i++) {
        const SalesData* sale = &sales_data[i];

        int w, d, m, q;
        get_time_indices(sale->date, &w, &d, &m, &q);
        if (w < 0 || m < 0 || q < 0) continue;

        weekday_stats[w].count++;
        weekday_stats[w].total_units += sale->units;
        weekday_stats[w].total_revenue += sale->revenue;

        month_stats[m].count++;
        month_stats[m].total_units += sale->units;
        month_stats[m].total_revenue += sale->revenue;

        quarter_stats[q].count++;
        quarter_stats[q].total_units += sale->units;
        quarter_stats[q].total_revenue += sale->revenue;
    }
}

//==== In thống kê trung bình ====
void print_seasonality_report() {
    printf("\n📊 Seasonality Report\n");
    printf("-----------------------------------------\n");

    printf("Theo ngày trong tuần:\n");
    const char* days[] = {"CN", "T2", "T3", "T4", "T5", "T6", "T7"};
    for (int i = 0; i < DAYS_IN_WEEK; i++) {
        if (weekday_stats[i].count == 0) continue;
        double avg = (double)weekday_stats[i].total_revenue / weekday_stats[i].count;
        printf("  %s: %.2f đ\n", days[i], avg);
    }

    printf("\nTheo tháng trong năm:\n");
    for (int i = 0; i < MONTHS_IN_YEAR; i++) {
        if (month_stats[i].count == 0) continue;
        double avg = (double)month_stats[i].total_revenue / month_stats[i].count;
        printf("  Tháng %2d: %.2f đ\n", i + 1, avg);
    }

    printf("\nTheo quý:\n");
    for (int i = 0; i < QUARTERS_IN_YEAR; i++) {
        if (quarter_stats[i].count == 0) continue;
        double avg = (double)quarter_stats[i].total_revenue / quarter_stats[i].count;
        printf("  Quý %d: %.2f đ\n", i + 1, avg);
    }
}

//==== Gợi ý thời điểm tốt để mở chiến dịch ====
void suggest_campaign_periods() {
    int best_month = 0, best_quarter = 0;
    double max_month_avg = 0.0, max_quarter_avg = 0.0;

    for (int i = 0; i < MONTHS_IN_YEAR; i++) {
        if (month_stats[i].count == 0) continue;
        double avg = (double)month_stats[i].total_revenue / month_stats[i].count;
        if (avg > max_month_avg) {
            max_month_avg = avg;
            best_month = i;
        }
    }

    for (int i = 0; i < QUARTERS_IN_YEAR; i++) {
        if (quarter_stats[i].count == 0) continue;
        double avg = (double)quarter_stats[i].total_revenue / quarter_stats[i].count;
        if (avg > max_quarter_avg) {
            max_quarter_avg = avg;
            best_quarter = i;
        }
    }

    printf("\n🎯 Gợi ý thời điểm mở chiến dịch:\n");
    printf("  👉 Tháng tốt nhất: Tháng %d (trung bình: %.2f đ)\n", best_month + 1, max_month_avg);
    printf("  👉 Quý tốt nhất: Quý %d (trung bình: %.2f đ)\n", best_quarter + 1, max_quarter_avg);
}
//=====================================
void analyze_campaign_effectiveness() {
    for (int i = 0; i < NUM_CAMPAIGNS; i++) {
        const SaleRecord* camp = &campaigns[i];
        effectiveness[i].id = camp->campaign_id;
        strncpy(effectiveness[i].name, camp->campaign, sizeof(effectiveness[i].name));
        effectiveness[i].count_in = effectiveness[i].revenue_in = 0;
        effectiveness[i].count_out = effectiveness[i].revenue_out = 0;
    }

    for (int i = 0; i < SALES_DATA_COUNT; i++) {
        const SalesData* sale = &sales_data[i];
        int in_any = 0;

        for (int j = 0; j < NUM_CAMPAIGNS; j++) {
            if (is_date_in_campaign(sale->date, &campaigns[j])) {
                effectiveness[j].count_in++;
                effectiveness[j].revenue_in += sale->revenue;
                in_any = 1;
            } else {
                effectiveness[j].count_out++;
                effectiveness[j].revenue_out += sale->revenue;
            }
        }
    }

    // Tính tỷ lệ tăng trưởng cho từng chiến dịch
    for (int i = 0; i < NUM_CAMPAIGNS; i++) {
        int in = effectiveness[i].revenue_in;
        int out = effectiveness[i].revenue_out;

        if (out > 0)
            effectiveness[i].uplift_ratio = ((double)(in - out)) / out * 100;
        else
            effectiveness[i].uplift_ratio = 0;
    }
}

void print_campaign_effectiveness_report() {
    printf("\n📈 Đánh giá hiệu quả chiến dịch:\n");
    printf("---------------------------------------------------------------------\n");
    printf("| ID | Tên chiến dịch         | Doanh thu | %% Tăng | Đơn trong | Đơn ngoài |\n");
    printf("---------------------------------------------------------------------\n");
    for (int i = 0; i < NUM_CAMPAIGNS; i++) {
        CampaignEffectiveness* e = &effectiveness[i];
        printf("| %2d | %-22s | %9d | %+6.2f%% | %9d | %10d |\n",
            e->id, e->name, e->revenue_in, e->uplift_ratio, e->count_in, e->count_out);
    }
    printf("---------------------------------------------------------------------\n");

    // Tìm chiến dịch tốt nhất
    int best_id = -1;
    double max_uplift = -9999;
    for (int i = 0; i < NUM_CAMPAIGNS; i++) {
        if (effectiveness[i].uplift_ratio > max_uplift) {
            max_uplift = effectiveness[i].uplift_ratio;
            best_id = i;
        }
    }

    if (best_id >= 0) {
        printf("🏆 Chiến dịch hiệu quả nhất: \"%s\" (Tăng %.2f%% doanh thu)\n",
               effectiveness[best_id].name, effectiveness[best_id].uplift_ratio);
    }
}
#endif //SEASONALITY_MODEL_H

// analyze_seasonality();
// print_seasonality_report();
// suggest_campaign_periods();
// analyze_campaign_effectiveness();
// print_campaign_effectiveness_report();