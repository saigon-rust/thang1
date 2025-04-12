#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <iomanip>  // std::setprecision
#include <cctype>   // std::toupper

#define MAX_POINTS 26  // Tối đa từ A đến Z

// Hàm trim ký tự xuống dòng
void trim_newline(std::string &str) {
    while (!str.empty() && (str.back() == '\n' || str.back() == '\r')) {
        str.pop_back();
    }
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

    return 0;
}
