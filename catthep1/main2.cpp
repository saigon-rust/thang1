#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>

const double MAX_TOTAL_LENGTH = 11.7;
const double MIN_DU_ALLOWED = 0;
const double MAX_DU_ALLOWED = MAX_TOTAL_LENGTH/2;

struct Demand {
    double length;
    int quantity;
};

class CutPattern {
public:
    std::vector<int> pattern_indices;
    std::vector<int> pattern_quantities;
    double total = 0;
    double du = 0;
    int score = 0;

    void addItem(int index, int quantity) {
        pattern_indices.push_back(index);
        pattern_quantities.push_back(quantity);
    }

    void evaluateScore(const std::vector<Demand>& demands) {
        score = 0;
        int largeCount = 0;
        int smallCount = 0;
        int totalBars = 0;

        for (size_t i = 0; i < pattern_indices.size(); ++i) {
            double len = demands[pattern_indices[i]].length;
            int qty = pattern_quantities[i];
            totalBars += qty;

            if (len >= MAX_TOTAL_LENGTH/2) largeCount += qty;
            else smallCount += qty;
        }

        if (largeCount == 1 && smallCount == 1) score += totalBars*4;
        if (largeCount == 1 && smallCount == 2) score += totalBars*2;
        if (largeCount == 0 && smallCount == 1) score += totalBars*3;
        if (largeCount == 0 && smallCount == 2) score += totalBars;
        if (largeCount == 0 && smallCount == 3) score += totalBars*0.5;
        if (du >= 0.2) score -= 5;
        if (du < 0.2) score += 5;

    }

    void print(const std::vector<Demand>& demands, int pattern_number) const {
        std::cout << "[" << pattern_number << "] ";
        for (size_t i = 0; i < pattern_indices.size(); ++i) {
            std::cout << pattern_quantities[i] << " x " << std::fixed << std::setprecision(2)
                      << demands[pattern_indices[i]].length << "m, ";
        }
        std::cout << "=> Tổng: " << std::fixed << std::setprecision(3)
                  << total << "m, Dư: " << du << "m, Điểm: " << score << "\n";
    }
};

class Cutter {
private:
    std::vector<Demand> demands;
    std::vector<CutPattern> valid_patterns;
    int max_count = 1;

public:
    Cutter(const std::vector<Demand>& d) : demands(d) {
        std::sort(demands.begin(), demands.end(),
                  [](const Demand& a, const Demand& b) {
                      return a.length > b.length;
                  });

        double min_length = MAX_TOTAL_LENGTH;
        for (const auto& d : demands) {
            if (d.length > 0 && d.length < min_length) {
                min_length = d.length;
            }
        }
        if (min_length > 0) {
            max_count = static_cast<int>(MAX_TOTAL_LENGTH / min_length);
        }
    }

    void findPatterns() {
        std::vector<int> counts(demands.size(), 0);
        search(0, counts, 0.0);

        // Chấm điểm và sắp xếp mẫu theo điểm
        for (auto& p : valid_patterns) {
            p.evaluateScore(demands);
        }

        std::sort(valid_patterns.begin(), valid_patterns.end(),
                  [](const CutPattern& a, const CutPattern& b) {
                      return a.score > b.score;
                  });

        int idx = 1;
        for (const auto& pattern : valid_patterns) {
            pattern.print(demands, idx++);
        }
    }

private:
    void search(int index, std::vector<int>& counts, double total) {
        if (total > MAX_TOTAL_LENGTH) return;

        if (index == static_cast<int>(demands.size())) {
            double du = MAX_TOTAL_LENGTH - total;
            if (du >= MIN_DU_ALLOWED && du <= MAX_DU_ALLOWED) {
                CutPattern pattern;
                for (size_t i = 0; i < counts.size(); ++i) {
                    if (counts[i] > 0) {
                        pattern.addItem(i, counts[i]);
                    }
                }
                pattern.total = total;
                pattern.du = du;
                valid_patterns.push_back(pattern);
            }
            return;
        }

        for (int i = 0; i <= max_count; ++i) {
            counts[index] = i;
            search(index + 1, counts, total + i * demands[index].length);
        }
    }
};

int main() {
    std::vector<Demand> demands = {
        {8, 336},
        {1.3, 281},
        {1.05, 281},
        {0.9, 336},
        {6.95, 336},
    };

    Cutter cutter(demands);
    std::cout << "Tìm tổ hợp tối ưu (ưu tiên theo điểm số):\n";
    cutter.findPatterns();

    return 0;
}
