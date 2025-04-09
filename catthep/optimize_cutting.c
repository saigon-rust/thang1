#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_TOTAL_LENGTH 11.7
#define MAX_PATTERNS 10000
#define MAX_TYPES 10

double min_du_allowed = 0;
double max_du_allowed = 0.5;

typedef struct {
    float length;
    int quantity;
} Demand;

Demand demands[] = {
    {8, 336},
    {1.302, 281},
    {1.05, 281},
    {0.9, 336},
    {6.95, 336},
    {11.21, 336},
    {11.3, 100},
};

int numDemands = sizeof(demands) / sizeof(demands[0]);

typedef struct {
    int pattern_indices[MAX_TYPES];
    int pattern_quantities[MAX_TYPES];
    int num_items;
    double total;
    double du;
} CutPattern;

CutPattern patterns[MAX_PATTERNS];
int pattern_count = 0;
int MAX_COUNT = 0;

int solution_count = 0;

void print_pattern(CutPattern *p) {
    printf("[%d] ", ++solution_count);
    for (int i = 0; i < p->num_items; i++) {
        int idx = p->pattern_indices[i];
        int qty = p->pattern_quantities[i];
        printf("%d x %.2fm, ", qty, demands[idx].length);
    }
    printf("=> Tổng: %.3fm, Dư: %.3fm\n", p->total, p->du);
}

void save_pattern(int counts[], double total) {
    double du = MAX_TOTAL_LENGTH - total;
    if (du < min_du_allowed || du > max_du_allowed) return;

    if (pattern_count >= MAX_PATTERNS) return;

    CutPattern *p = &patterns[pattern_count];
    p->num_items = 0;
    p->total = total;
    p->du = du;

    for (int i = 0; i < numDemands; i++) {
        if (counts[i] > 0) {
            p->pattern_indices[p->num_items] = i;
            p->pattern_quantities[p->num_items] = counts[i];
            p->num_items++;
        }
    }
    pattern_count++;
}

void search(int index, int counts[], double total) {
    if (total > MAX_TOTAL_LENGTH) return;

    if (index == numDemands) {
        save_pattern(counts, total);
        return;
    }

    for (int i = 0; i <= MAX_COUNT; i++) {
        counts[index] = i;
        search(index + 1, counts, total + i * demands[index].length);
    }
}

int is_enough(int required[], int used[], int size) {
    for (int i = 0; i < size; i++) {
        if (used[i] < required[i]) return 0;
    }
    return 1;
}

void optimize_cutting() {
    int remaining[MAX_TYPES];
    for (int i = 0; i < numDemands; i++) {
        remaining[i] = demands[i].quantity;
    }

    int used_patterns[MAX_PATTERNS] = {0};
    int total_bars = 0;
    double total_du = 0.0;

    while (1) {
        int best_pattern = -1;
        int best_covered = 0;

        for (int i = 0; i < pattern_count; i++) {
            int cover = 0;
            for (int j = 0; j < patterns[i].num_items; j++) {
                int idx = patterns[i].pattern_indices[j];
                int qty = patterns[i].pattern_quantities[j];
                if (remaining[idx] > 0) {
                    cover += (remaining[idx] < qty) ? remaining[idx] : qty;
                }
            }
            if (cover > best_covered) {
                best_covered = cover;
                best_pattern = i;
            }
        }

        if (best_pattern == -1) break;

        CutPattern *p = &patterns[best_pattern];
        for (int j = 0; j < p->num_items; j++) {
            int idx = p->pattern_indices[j];
            int qty = p->pattern_quantities[j];
            remaining[idx] -= (remaining[idx] < qty) ? remaining[idx] : qty;
        }

        used_patterns[best_pattern]++;
        total_bars++;
        total_du += p->du;
    }

    
    printf("\n📋 Chi tiết kế hoạch cắt:\n");
    for (int i = 0; i < pattern_count; i++) {
        if (used_patterns[i] > 0) {
            printf("   - %d cây:", used_patterns[i]);
            for (int j = 0; j < patterns[i].num_items; j++) {
                int idx = patterns[i].pattern_indices[j];
                printf(" %d x %.2fm,", patterns[i].pattern_quantities[j], demands[idx].length);
            }
            printf(" => Tổng: %.3fm, Dư: %.3fm\n", patterns[i].total, patterns[i].du);
        }
    }
    printf("\n✅ Tổng số cây thép sử dụng: %d\n", total_bars);
    printf("🔹 Tỷ lệ hao hụt: %.2f%%\n", (total_du/total_bars/11.7) * 100.0);

    // Kiểm tra phần còn lại
    int total_remaining = 0;
    for (int i = 0; i < numDemands; i++) {
        total_remaining += remaining[i];
    }

    if (total_remaining == 0) {
        printf("\n🎉 Đã cắt hết tất cả yêu cầu.\n");
    } else {
        printf("\n⚠️ Còn lại %d thanh không thể cắt với các mẫu đã sinh:\n", total_remaining);
        for (int i = 0; i < numDemands; i++) {
            if (remaining[i] > 0) {
                printf("  - %d x %.2fm\n", remaining[i], demands[i].length);
            }
        }
    }
}

int main() {
    double min_length = MAX_TOTAL_LENGTH;
    for (int i = 0; i < numDemands; i++) {
        if (demands[i].length < min_length)
            min_length = demands[i].length;
    }

    MAX_COUNT = (int)(MAX_TOTAL_LENGTH / min_length);

    printf("🔍 Tìm mẫu cắt hợp lệ với hao hụt từ %.3f đến %.3f (MAX_COUNT=%d)\n",
           min_du_allowed, max_du_allowed, MAX_COUNT);

    int counts[MAX_TYPES] = {0};
    search(0, counts, 0.0);

    printf("✅ Tổng mẫu hợp lệ: %d\n", pattern_count);
    for (int i = 0; i < pattern_count; i++) {
        print_pattern(&patterns[i]);
    }

    optimize_cutting();

    return 0;
}
