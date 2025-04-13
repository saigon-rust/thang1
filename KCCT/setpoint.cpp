#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <array>
#include <set>
#include <algorithm>
#include <iomanip>
#include <limits>

#define MAX_POINTS 26

struct Point {
    double x, y;
};

void trim_newline(std::string &str) {
    while (!str.empty() && (str.back() == '\n' || str.back() == '\r')) {
        str.pop_back();
    }
}

bool is_inside(double x, double y,
               double min_x, double max_x,
               double min_y, double max_y) {
    return x > min_x && x < max_x && y > min_y && y < max_y;
}

int main() {
    const std::string filename = "a.csv";
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Không thể mở file: " << filename << std::endl;
        return 1;
    }

    std::unordered_map<char, Point> coordinates;
    std::string line;
    int line_num = 0;
    char pointLabel = 'A';

    std::cout << ">>> Đang đọc dữ liệu từ file CSV...\n";

    while (std::getline(file, line)) {
        line_num++;
        trim_newline(line);
        if (line.empty()) continue;

        // Bỏ qua tiêu đề
        if (line_num == 1 && (line.find('X') != std::string::npos || line.find('x') != std::string::npos)) {
            continue;
        }

        std::stringstream ss(line);
        std::string token;
        double x = 0.0, y = 0.0;

        if (!std::getline(ss, token, ',')) continue;
        x = std::stod(token);

        if (!std::getline(ss, token, ',')) continue;
        y = std::stod(token);

        if (pointLabel > 'Z') {
            std::cerr << "Số lượng điểm vượt quá giới hạn (26 điểm từ A-Z).\n";
            break;
        }

        coordinates[pointLabel] = {x, y};
        ++pointLabel;
    }
    file.close();

    std::vector<char> labels;
    for (const auto& p : coordinates) {
        labels.push_back(p.first);
    }

    std::sort(labels.begin(), labels.end());

    // ===== Xác định dầm hợp lý (ngang và dọc) =====
    std::set<std::pair<char, char>> beams;

    for (size_t i = 0; i < labels.size(); ++i) {
        for (size_t j = i + 1; j < labels.size(); ++j) {
            auto p1 = coordinates[labels[i]];
            auto p2 = coordinates[labels[j]];

            // Dầm chỉ hợp lệ nếu cùng X hoặc cùng Y
            if (p1.x == p2.x || p1.y == p2.y) {
                beams.insert({labels[i], labels[j]});
            }
        }
    }

    // ===== Xác định ô sàn hợp lệ (hình chữ nhật, không có điểm bên trong) =====
    std::vector<std::array<char, 4>> valid_slabs;

    for (size_t i = 0; i < labels.size(); ++i) {
        for (size_t j = i + 1; j < labels.size(); ++j) {
            for (size_t k = j + 1; k < labels.size(); ++k) {
                for (size_t l = k + 1; l < labels.size(); ++l) {
                    std::array<char, 4> slab = {labels[i], labels[j], labels[k], labels[l]};
                    std::vector<Point> points;
                    for (char ch : slab) {
                        points.push_back(coordinates[ch]);
                    }

                    std::set<double> xs, ys;
                    for (const auto& pt : points) {
                        xs.insert(pt.x);
                        ys.insert(pt.y);
                    }

                    // Chỉ tiếp tục nếu tạo được hình chữ nhật (2 giá trị X, 2 giá trị Y)
                    if (xs.size() == 2 && ys.size() == 2) {
                        double min_x = *xs.begin();
                        double max_x = *xs.rbegin();
                        double min_y = *ys.begin();
                        double max_y = *ys.rbegin();

                        bool has_inside = false;
                        for (const auto& p : coordinates) {
                            char ch = p.first;
                            if (std::find(slab.begin(), slab.end(), ch) != slab.end()) continue;

                            if (is_inside(p.second.x, p.second.y, min_x, max_x, min_y, max_y)) {
                                has_inside = true;
                                break;
                            }
                        }

                        if (!has_inside) {
                            valid_slabs.push_back(slab);
                        }
                    }
                }
            }
        }
    }

    // ===== IN KẾT QUẢ =====
    std::cout << "\n====== BÁO CÁO PHƯƠNG ÁN DẦM - SÀN =====\n";
    std::cout << "Tổng số điểm: " << coordinates.size() << "\n";
    std::cout << "Tổng số dầm hợp lý: " << beams.size() << "\n";
    std::cout << "Tổng số ô sàn hợp lệ: " << valid_slabs.size() << "\n";

    std::cout << "\n-- Danh sách DẦM (theo phương X/Y) --\n";
    int beam_id = 1;
    for (const auto& beam : beams) {
        const auto& A = coordinates[beam.first];
        const auto& B = coordinates[beam.second];
        std::cout << "Dầm " << beam_id++ << ": "
                  << beam.first << "(" << A.x << "," << A.y << ") <-> "
                  << beam.second << "(" << B.x << "," << B.y << ")\n";
    }

    std::cout << "\n-- Danh sách Ô SÀN (4 điểm hình chữ nhật không chứa điểm trong) --\n";
    int slab_id = 1;
    for (const auto& slab : valid_slabs) {
        std::cout << "Ô sàn " << slab_id++ << ": ";
        for (char ch : slab) {
            const auto& pt = coordinates[ch];
            std::cout << ch << "(" << pt.x << "," << pt.y << ") ";
        }
        std::cout << "\n";
    }

    std::cout << "\n=======================================\n";

    return 0;
}
