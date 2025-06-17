#include "sales_stats.h"

int main() {
    SaleRecord campaigns[] = {
        {1, "2024-01-10", "2024-01-12", "Khuyến mãi Tết"},
        {2, "2024-03-05", "2024-03-07", "Sinh nhật thương hiệu"},
        {3, "2024-06-01", "2024-06-03", "Giữa năm xả hàng"}
    };

    int num_campaigns = sizeof(campaigns) / sizeof(SaleRecord);
    int row = 0;

    print_stats_header(num_campaigns);

    for (int i = 0; i < num_campaigns; i++) {
        SaleRecord campaign = campaigns[i];

        struct tm tm_start = {0}, tm_end = {0};
        strptime(campaign.date_start, "%Y-%m-%d", &tm_start);
        strptime(campaign.date_end, "%Y-%m-%d", &tm_end);

        time_t t_start = mktime(&tm_start);
        time_t t_end   = mktime(&tm_end);

        for (time_t t = t_start; t <= t_end; t += 86400) {
            struct tm* now = localtime(&t);
            char date[11];
            strftime(date, sizeof(date), "%Y-%m-%d", now);

            // Kiểm tra xem đã ghi dòng này chưa
            int found = 0;
            for (int r = 0; r < row; r++) {
                int match = 0;
                struct tm tmp = {0};
                get_time_indices(date, &tmp.tm_wday, &tmp.tm_mday, &tmp.tm_mon, &tmp.tm_year);
                if (stats[r][tmp.tm_wday] &&
                    stats[r][DAYS_IN_WEEK + (tmp.tm_mday - 1)] &&
                    stats[r][DAYS_IN_WEEK + DAYS_IN_MONTH + tmp.tm_mon]) {
                    match = 1;
                }
                if (match) {
                    set_campaign_column(r, i);
                    found = 1;
                    break;
                }
            }

            if (!found) {
                set_stats_row_base(row, date);
                set_campaign_column(row, i);
                print_stats_row(row, BASE_COLUMNS + num_campaigns);
                row++;
            }
        }
    }

    return 0;
}
