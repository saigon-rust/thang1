#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define MAX_TOTAL_LENGTH 11.7

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

};

int numDemands = sizeof(demands) / sizeof(demands[0]);

int compareDemands(const void *a, const void *b) {
    Demand *demandA = (Demand *)a;
    Demand *demandB = (Demand *)b;

    if (demandA->length < demandB->length) {
        return -1; // a nhỏ hơn b, nên a đứng sau
    } else if (demandA->length > demandB->length) {
        return 1; // a lớn hơn b, nên a đứng trước
    } else {
        return 0; // bằng nhau
    }
}

typedef struct {
    int *pattern_indices;   // Lưu index của loại chiều dài và số lượng (cấp phát động)
    int *pattern_quantities; // Lưu số lượng tương ứng (cấp phát động)
    int num_items;          // Số lượng loại chiều dài trong pattern này
    double total;
    double du;
} CutPattern;

int n_demand_types;         // số loại chiều dài yêu cầu thực tế
int MAX_COUNT;              // mỗi loại yêu cầu lấy tối đa bao nhiêu lần (tự động tính)

int solution_count = 0;     // Biến đếm số phương án cắt

void print_pattern(CutPattern *p, Demand *demands) {
    printf("[%d] ", ++solution_count);
    for (int i = 0; i < p->num_items; i++) {
        int demand_index = p->pattern_indices[i];
        int quantity = p->pattern_quantities[i];
        printf("%d x %.2fm, ", quantity, demands[demand_index].length);
    }
    printf("=> Tổng: %.3fm, Dư: %.3fm\n", p->total, p->du);
}

void search(int index, int counts[], double total, CutPattern *current_pattern) {
    if (total > MAX_TOTAL_LENGTH) return;

    if (index == n_demand_types) {
        double du = MAX_TOTAL_LENGTH - total;
        if (du >= min_du_allowed && du <= max_du_allowed) {
            CutPattern *pattern = (CutPattern *)malloc(sizeof(CutPattern));
            if (pattern != NULL) {
                pattern->num_items = 0;
                pattern->pattern_indices = (int *)malloc(n_demand_types * sizeof(int));
                pattern->pattern_quantities = (int *)malloc(n_demand_types * sizeof(int));
                if (pattern->pattern_indices == NULL || pattern->pattern_quantities == NULL) {
                    perror("Lỗi cấp phát bộ nhớ");
                    free(pattern->pattern_indices);
                    free(pattern->pattern_quantities);
                    free(pattern);
                    exit(EXIT_FAILURE);
                }

                for (int i = 0; i < n_demand_types; i++) {
                    if (counts[i] > 0) {
                        pattern->pattern_indices[pattern->num_items] = i;
                        pattern->pattern_quantities[pattern->num_items] = counts[i];
                        pattern->num_items++;
                    }
                }
                pattern->total = total;
                pattern->du = du;
                print_pattern(pattern, demands);
                free(pattern->pattern_indices);
                free(pattern->pattern_quantities);
                free(pattern);
            }
        }
        return;
    }

    for (int i = 0; i <= MAX_COUNT; i++) {
        counts[index] = i;
        search(index + 1, counts, total + i * demands[index].length, current_pattern);
    }
}

void sort_demands_desc() {
    for (int i = 0; i < numDemands - 1; i++) {
        for (int j = i + 1; j < numDemands; j++) {
            if (demands[i].length < demands[j].length) {
                Demand temp = demands[i];
                demands[i] = demands[j];
                demands[j] = temp;
            }
        }
    }
}
int main() {
    sort_demands_desc()
    n_demand_types = sizeof(demands) / sizeof(demands[0]);
    qsort(demands, numDemands, sizeof(Demand), compareDemands);

    double min_length = MAX_TOTAL_LENGTH;
    for (int i = 0; i < n_demand_types; i++) {
        if (demands[i].length > 0 && demands[i].length < min_length) {
            min_length = demands[i].length;
        }
    }

    if (min_length > 0) {
        MAX_COUNT = (int)(MAX_TOTAL_LENGTH / min_length);
    } else {
        MAX_COUNT = 1; // Avoid division by zero if all lengths are 0 (shouldn't happen with the given data)
    }


    printf("Tìm tổ hợp với tổng <= %.3f, dư từ %.3f đến %.3f (MAX_COUNT=%d)\n",
           MAX_TOTAL_LENGTH, min_du_allowed, max_du_allowed, MAX_COUNT);

    int counts[n_demand_types];
    for (int i = 0; i < n_demand_types; i++) {
        counts[i] = 0;
    }
    CutPattern current_pattern;
    search(0, counts, 0.0, &current_pattern);

    return 0;
}