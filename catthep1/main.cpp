#include <iostream>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <numeric>

using namespace std;

const double MAX_TOTAL_LENGTH = 11.7 +1e-6;
const double MIN_DU_ALLOWED = 0.0;
const double MAX_DU_ALLOWED = MAX_TOTAL_LENGTH/2-1e-6;

struct Demand {
    double length;
    int quantity;
};

class CutPattern {
public:
    vector<int> indices;
    vector<int> quantities;
    double total = 0.0;
    double du = 0.0;

    void print(int id, const vector<Demand>& demands) const {
        cout << "[" << id << "] ";
        for (size_t i = 0; i < indices.size(); ++i) {
            cout << quantities[i] << " x " << fixed << setprecision(2)
                 << demands[indices[i]].length << "m, ";
        }
        cout << "=> Tổng: " << fixed << setprecision(3) << total
             << "m, Dư: " << du << "m\n";
    }
};

class SteelCutter {
private:
    vector<Demand> demands;
    vector<CutPattern> patterns;
    int solution_count = 0;
    int max_count = 0;

public:
    SteelCutter(const vector<Demand>& d) : demands(d) {
        sort_demands_desc();
        double min_length = MAX_TOTAL_LENGTH;
        for (const auto& demand : demands) {
            if (demand.length < min_length)
                min_length = demand.length;
        }
        max_count = static_cast<int>(MAX_TOTAL_LENGTH / min_length);
    }

    void run() {
        cout << "🔍 Tìm mẫu cắt hợp lệ với hao hụt từ "
             << MIN_DU_ALLOWED << " đến " << MAX_DU_ALLOWED
             << " (MAX_COUNT=" << max_count << ")\n";

        vector<int> counts(demands.size(), 0);
        search(0, counts, 0.0);

        cout << "✅ Tổng mẫu hợp lệ: " << patterns.size() << "\n";
        for (const auto& pattern : patterns) {
            pattern.print(++solution_count, demands);
        }

        optimize_cutting();
    }

private:
    void sort_demands_desc() {
        sort(demands.begin(), demands.end(), [](const Demand& a, const Demand& b) {
            return a.length > b.length;
        });
    }

    void search(int index, vector<int>& counts, double total) {
        if (total > MAX_TOTAL_LENGTH) return;

        if (index == static_cast<int>(demands.size())) {
            save_pattern(counts, total);
            return;
        }

        for (int i = 0; i <= demands[index].quantity; ++i) {
            counts[index] = i;
            search(index + 1, counts, total + i * demands[index].length);
        }
    }

    bool is_effective_pattern(const vector<int>& counts) {
        int large_count = 0, small_count = 0;
        for (size_t i = 0; i < demands.size(); ++i) {
            if (counts[i] == 0) continue;
            if (demands[i].length > MAX_TOTAL_LENGTH / 2) large_count++;
            else small_count++;
        }
        return (large_count == 1 && small_count <= 2) || (large_count == 0 && small_count <= 2);
    }

    void save_pattern(const vector<int>& counts, double total) {
        double du = MAX_TOTAL_LENGTH - total;
        if (du < MIN_DU_ALLOWED || du > MAX_DU_ALLOWED) return;
        if (!is_effective_pattern(counts)) return;

        CutPattern p;
        p.total = total;
        p.du = du;
        for (size_t i = 0; i < demands.size(); ++i) {
            if (counts[i] > 0) {
                p.indices.push_back(i);
                p.quantities.push_back(counts[i]);
            }
        }
        patterns.push_back(p);
    }

    double score_pattern(const CutPattern& p, const vector<int>& remaining) {
        double total_length = 0.0;
        bool fulfill_any = false;
        int long_count = 0, item_count = 0;

        for (size_t i = 0; i < p.indices.size(); ++i) {
            int idx = p.indices[i];
            int qty = p.quantities[i];
            total_length += qty * demands[idx].length;
            if (qty >= remaining[idx]) fulfill_any = true;
            if (demands[idx].length > MAX_TOTAL_LENGTH / 2) long_count++;
            item_count++;
        }

        double efficiency = total_length / MAX_TOTAL_LENGTH;
        double score = 0.0;
        score += 0.6 * efficiency;
        if (fulfill_any) score += 0.2;
        if (item_count <= 2) score += 0.1;
        if (long_count > 0) score += 0.1;

        return score;
    }

    void optimize_cutting() {
        vector<int> remaining;
        vector<int> cut_total(demands.size(), 0);
        vector<int> used_patterns(patterns.size(), 0);
        int total_bars = 0;
        double total_du = 0.0;

        for (const auto& d : demands) remaining.push_back(d.quantity);

        while (true) {
            int best_pattern = -1;
            double best_score = -1.0;

            for (size_t i = 0; i < patterns.size(); ++i) {
                const auto& p = patterns[i];
                bool possible = true;
                for (size_t j = 0; j < p.indices.size(); ++j) {
                    int idx = p.indices[j];
                    if (p.quantities[j] > remaining[idx]) {
                        possible = false;
                        break;
                    }
                }
                if (!possible) continue;

                double score = score_pattern(p, remaining);
                if (score > best_score) {
                    best_score = score;
                    best_pattern = static_cast<int>(i);
                }
            }

            if (best_pattern == -1) break;

            const auto& p = patterns[best_pattern];
            for (size_t j = 0; j < p.indices.size(); ++j) {
                int idx = p.indices[j];
                int qty = p.quantities[j];
                remaining[idx] -= qty;
                cut_total[idx] += qty;
            }

            used_patterns[best_pattern]++;
            total_bars++;
            total_du += p.du;

            if (all_of(remaining.begin(), remaining.end(), [](int x) { return x == 0; }))
                break;
        }

        print_cut_summary(cut_total, remaining, used_patterns, total_bars, total_du);
    }

    void print_cut_summary(const vector<int>& cut_total, const vector<int>& remaining,
                           const vector<int>& used_patterns, int total_bars, double total_du) {
        cout << "\n📦 Số lượng thép yêu cầu:\n";
        for (size_t i = 0; i < demands.size(); ++i) {
            cout << "  - " << fixed << setprecision(3) << demands[i].length
                 << "m: " << demands[i].quantity << " thanh\n";
        }

        cout << "\n📋 Chi tiết kế hoạch cắt:\n";
        for (size_t i = 0; i < patterns.size(); ++i) {
            if (used_patterns[i] == 0) continue;
            cout << "  - " << used_patterns[i] << " cây:";
            for (size_t j = 0; j < patterns[i].indices.size(); ++j) {
                int idx = patterns[i].indices[j];
                cout << " " << patterns[i].quantities[j] << " x "
                     << demands[idx].length << "m,";
            }
            cout << " => Tổng: " << patterns[i].total
                 << "m, Dư: " << patterns[i].du << "m\n";
        }

        cout << "\n📦 Số lượng thép (đã cắt/yêu cầu):\n";
        for (size_t i = 0; i < demands.size(); ++i) {
            cout << "  - " << demands[i].length << "m: "
                 << cut_total[i] << " / " << demands[i].quantity << " thanh\n";
        }

        cout << "\n✅ Tổng số thép sử dụng: " << total_bars << " cây 11.7m\n";
        cout << "🔹 Tỷ lệ hao hụt: "
             << fixed << setprecision(2)
             << (total_du / (total_bars * MAX_TOTAL_LENGTH)) * 100.0 << "%\n";

        int total_remaining = accumulate(remaining.begin(), remaining.end(), 0);
        if (total_remaining == 0) {
            cout << "\n🎉 Đã cắt hết tất cả yêu cầu.\n";
        } else {
            cout << "\n⚠️ Còn lại " << total_remaining << " thanh:\n";
            for (size_t i = 0; i < remaining.size(); ++i) {
                if (remaining[i] > 0) {
                    cout << "  - " << remaining[i] << " x " << demands[i].length << "m\n";
                }
            }
        }
    }
};

int main() {
    vector<Demand> demands = {
        {8.0, 330},
        {1.3, 500},
        {1.05, 280},
        {0.9, 10000},
        {5.9, 336},
        {11.0, 336},
        {8.1, 330},
        {1.4, 500},
        {1.15, 280},
        {0.91, 1000},
        {5.91, 336},
        {11.2, 336},
    };

    SteelCutter cutter(demands);
    cutter.run();

    return 0;
}
