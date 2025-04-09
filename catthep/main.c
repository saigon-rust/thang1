#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_LENGTHS 10
#define MAX_PATTERNS 10000
#define MAX_PLAN 1000
#define TARGET 11.7
#define MAX_LOAI_TREN_MOT_CAY 2

typedef struct {
    float length;
    int quantity;
} Demand;

typedef struct {
    int pattern[MAX_LENGTHS];  // số lượng mỗi loại chiều dài
    float total;
    float du;
} CutPattern;

typedef struct {
    CutPattern pattern;
    int count;
} CutPlan;

// Dữ liệu đầu vào mẫu (giả lập từ Google Sheet)
Demand demands[] = {
    {1.302, 281},
    {1.05, 281},
    {0.9, 336},
    {0.95, 336}
};
int num_demands = sizeof(demands) / sizeof(Demand);

// Mảng lưu mẫu cắt
CutPattern patterns[MAX_PATTERNS];
int num_patterns = 0;

CutPlan plan[MAX_PLAN];
int num_plan = 0;

// Hàm sinh các mẫu cắt
void generate_patterns(float *lengths, int len_count, int max_qty, float max_du) {
    int max_loop = (int) pow(max_qty + 1, len_count);

    for (int i = 0; i < max_loop; i++) {
        int qtys[MAX_LENGTHS] = {0};
        int tmp = i;
        float total = 0;

        for (int j = 0; j < len_count; j++) {
            qtys[j] = tmp % (max_qty + 1);
            total += qtys[j] * lengths[j];
            tmp /= (max_qty + 1);
        }

        if (total > 0 && total <= TARGET && TARGET - total <= max_du) {
            CutPattern p;
            for (int j = 0; j < len_count; j++) {
                p.pattern[j] = qtys[j];
            }
            p.total = total;
            p.du = TARGET - total;
            patterns[num_patterns++] = p;
        }
    }
}

// So sánh mẫu cắt theo độ dư (ít hơn ưu tiên trước)
int compare_du(const void *a, const void *b) {
    float du1 = ((CutPattern*)a)->du;
    float du2 = ((CutPattern*)b)->du;
    return (du1 > du2) - (du1 < du2);
}

// In một mẫu cắt
void print_pattern(CutPattern *p, float *lengths, int len_count) {
    for (int i = 0; i < len_count; i++) {
        if (p->pattern[i] > 0) {
            printf("%d x %.2fm, ", p->pattern[i], lengths[i]);
        }
    }
    printf("=> Tổng: %.2fm, Dư: %.2fm\n", p->total, p->du);
}

// Lập kế hoạch cắt
void plan_cut(float *lengths, int *quantities, int len_count) {
    int remaining[MAX_LENGTHS];
    memcpy(remaining, quantities, sizeof(int) * len_count);

    while (1) {
        int best_index = -1;
        int best_score = -1;

        for (int i = 0; i < num_patterns; i++) {
            int non_zero = 0;
            int match_idx = -1;
            int match_qty = 0;
            int valid = 1;

            for (int j = 0; j < len_count; j++) {
                if (patterns[i].pattern[j] > 0) {
                    non_zero++;
                    if (remaining[j] == 0) {
                        valid = 0;
                        break;
                    }
                    match_idx = j;
                    match_qty = patterns[i].pattern[j];
                }
            }

            if (!valid || non_zero > MAX_LOAI_TREN_MOT_CAY) continue;

            int score = 0;
            if (match_idx >= 0) {
                score += match_qty * 10;
                if (non_zero == 1) score += 20;
                if (match_qty >= remaining[match_idx]) score += 30;
                score -= (int)(patterns[i].du * 100);
            }

            if (score > best_score) {
                best_score = score;
                best_index = i;
            }
        }

        if (best_index == -1 || best_score == 0) break;

        // Áp dụng mẫu tốt nhất
        for (int j = 0; j < len_count; j++) {
            remaining[j] -= patterns[best_index].pattern[j];
            if (remaining[j] < 0) remaining[j] = 0;
        }

        // Kiểm tra trùng lặp mẫu
        int found = 0;
        for (int k = 0; k < num_plan; k++) {
            int same = 1;
            for (int j = 0; j < len_count; j++) {
                if (plan[k].pattern.pattern[j] != patterns[best_index].pattern[j]) {
                    same = 0;
                    break;
                }
            }
            if (same) {
                plan[k].count++;
                found = 1;
                break;
            }
        }

        if (!found) {
            plan[num_plan].pattern = patterns[best_index];
            plan[num_plan].count = 1;
            num_plan++;
        }
    }
}

int main() {
    float lengths[MAX_LENGTHS];
    int quantities[MAX_LENGTHS];

    for (int i = 0; i < num_demands; i++) {
        lengths[i] = demands[i].length;
        quantities[i] = demands[i].quantity;
    }

    printf("\n🔹 Danh sách chiều dài cần cắt:\n");
    for (int i = 0; i < num_demands; i++) {
        printf("- %.2fm: %d cây\n", lengths[i], quantities[i]);
    }

    int max_qty = (int)(TARGET / lengths[num_demands - 1]) + 2;

    generate_patterns(lengths, num_demands, max_qty, 0.5);
    qsort(patterns, num_patterns, sizeof(CutPattern), compare_du);

    printf("\n📌 Một số phương án cắt khả thi:\n");
    for (int i = 0; i < 10 && i < num_patterns; i++) {
        printf("%02d. ", i + 1);
        print_pattern(&patterns[i], lengths, num_demands);
    }

    plan_cut(lengths, quantities, num_demands);

    printf("\n✅ Kế hoạch cắt:\n");
    float tong_du = 0;
    int tong_cay = 0;

    for (int i = 0; i < num_plan; i++) {
        tong_cay += plan[i].count;
        tong_du += plan[i].pattern.du * plan[i].count;
        printf("- %d cây: ", plan[i].count);
        print_pattern(&plan[i].pattern, lengths, num_demands);
    }

    printf("\n♻️ Tổng hao hụt: %.2fm\n", tong_du);
    printf("📈 Tỉ lệ hao hụt trung bình: %.2fm/cây (~%.1f%%)\n",
           tong_du / tong_cay, (tong_du / tong_cay) / TARGET * 100);

    return 0;
}
