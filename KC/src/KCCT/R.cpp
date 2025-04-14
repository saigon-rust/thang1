#include <iostream>
#include <string>
#include <unordered_map>

class R_ {
public:
    const char* vatLieu;
    double f;
    double l;
    double R;

    R_(const char* vatLieu_in, double f_in, double l_in)
        : vatLieu(vatLieu_in), f(f_in), l(l_in), R(f_in / l_in) {}

    void printValues() const {
        std::cout << "Vat lieu: " << vatLieu << std::endl;
        std::cout << "f = " << f << std::endl;
        std::cout << "l = " << l << std::endl;
        std::cout << "R = " << R << std::endl;
    }
};

double R_value(const std::string& vatLieuInput) {
    const std::unordered_map<std::string, std::pair<double, double>> materialData = {
        {"M200", {11.5, 1.5}},
        {"M250", {14.5, 1.5}},
        {"CB300", {300.0, 1.15}},
        {"CB400", {400.0, 1.15}}
    };

    if (materialData.count(vatLieuInput)) {
        const auto& data = materialData.at(vatLieuInput);
        R_ myObject(vatLieuInput.c_str(), data.first, data.second);
        myObject.printValues();
        return myObject.R;
    } else {
        std::cout << "Vat lieu '" << vatLieuInput << "' khong duoc ho tro." << std::endl;
        return -1.0; // giá trị báo lỗi
    }
}

int main() {
    std::string vatLieuInput;
    std::cout << "Nhap vao vat lieu: ";
    std::cin >> vatLieuInput;

    double R = R_value(vatLieuInput);

    return 0;
}
