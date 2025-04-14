#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <iomanip>  // std::setprecision
#include <cctype>   // std::toupper

#include <array>
#include <vector>
#include <algorithm> // std::min, std::max


#define MAX_POINTS 26  // Tối đa từ A đến Z

// Hàm trim ký tự xuống dòng
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

    std::unordered_map<char, std::pair<double, double>> coordinates;
    std::string line;
    int line_num = 0;
    char pointLabel = 'A';

    std::cout << "Đọc và lưu tọa độ các điểm từ file...\n";
    std::cout << "-------------------------------------\n";

    while (std::getline(file, line)) {
        line_num++;
        trim_newline(line);

        // Bỏ qua dòng rỗng
        if (line.empty()) continue;

        // Bỏ dòng tiêu đề
        if (line_num == 1 && (line.find('A') != std::string::npos || line.find('a') != std::string::npos)) {
            continue;
        }

        std::stringstream ss(line);
        std::string token;
        double x = 0.0, y = 0.0;

        // Đọc tọa độ A
        if (!std::getline(ss, token, ',')) {
            std::cerr << "Lỗi định dạng dòng " << line_num << std::endl;
            continue;
        }
        x = std::stod(token);

        // Đọc tọa độ B
        if (!std::getline(ss, token, ',')) {
            std::cerr << "Lỗi định dạng dòng " << line_num << std::endl;
            continue;
        }
        y = std::stod(token);

        if (pointLabel > 'Z') {
            std::cerr << "Vượt quá giới hạn 26 điểm A-Z\n";
            break;
        }

        coordinates[pointLabel] = {x, y};
        std::cout << "Điểm " << pointLabel << ": A = " << std::fixed << std::setprecision(2)
                  << x << ", B = " << y << std::endl;

        pointLabel++;
    }

    file.close();

    std::cout << "\nTổng số điểm đã lưu: " << coordinates.size() << std::endl;

    // (Tuỳ chọn) In lại danh sách đã lưu:
    std::cout << "\nDanh sách tất cả điểm:\n";
    for (const auto &pair : coordinates) {
        std::cout << pair.first << ": (" << pair.second.first << ", " << pair.second.second << ")\n";
    }
    //-------------------------

    

    std::vector<std::array<char, 4>> valid_slabs;
    
    std::vector<char> labels;
    for (const auto& p : coordinates) {
        labels.push_back(p.first);
    }
    
    // Duyệt tất cả tổ hợp 4 điểm
    for (size_t i = 0; i < labels.size(); ++i) {
        for (size_t j = i + 1; j < labels.size(); ++j) {
            for (size_t k = j + 1; k < labels.size(); ++k) {
                for (size_t l = k + 1; l < labels.size(); ++l) {
    
                    std::array<char, 4> slab = {labels[i], labels[j], labels[k], labels[l]};
    
                    // Tìm min/max của X và Y
                    double min_x = std::numeric_limits<double>::max();
                    double max_x = std::numeric_limits<double>::lowest();
                    double min_y = std::numeric_limits<double>::max();
                    double max_y = std::numeric_limits<double>::lowest();
    
                    for (char ch : slab) {
                        double x = coordinates[ch].first;
                        double y = coordinates[ch].second;
                        min_x = std::min(min_x, x);
                        max_x = std::max(max_x, x);
                        min_y = std::min(min_y, y);
                        max_y = std::max(max_y, y);
                    }
    
                    // Kiểm tra nếu có điểm nào nằm bên trong
                    bool has_inside_point = false;
                    for (const auto& p : coordinates) {
                        char ch = p.first;
                        if (std::find(slab.begin(), slab.end(), ch) != slab.end()) continue;
    
                        double x = p.second.first;
                        double y = p.second.second;
    
                        if (is_inside(x, y, min_x, max_x, min_y, max_y)) {
                            has_inside_point = true;
                            break;
                        }
                    }
    
                    if (!has_inside_point) {
                        valid_slabs.push_back(slab);
                    }
                }
            }
        }
    }
    
    // In ra danh sách ô sàn hợp lệ
    std::cout << "\nCác ô sàn hợp lệ (không có điểm giữa):\n";
    int slab_id = 1;
    for (const auto& slab : valid_slabs) {
        std::cout << "Ô sàn " << slab_id++ << ": ";
        for (char ch : slab) {
            auto coord = coordinates[ch];
            std::cout << ch << "(" << coord.first << "," << coord.second << ") ";
        }
        std::cout << "\n";
    }



    return 0;
}
