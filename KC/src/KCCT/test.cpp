#include <iostream>
#include <cmath>
#include <unordered_map>
#include <string>

enum class LoaiDam {
    DonGian,
    LienTuc,
    CongXon,
    Nang
};

std::unordered_map<LoaiDam, std::pair<double, double>> tyLeDam = {
    {LoaiDam::DonGian, {1.0 / 10, 1.0 / 10}},
    {LoaiDam::LienTuc, {1.0 / 15, 1.0 / 12}},
    {LoaiDam::CongXon, {1.0 / 7,  1.0 / 5}},
    {LoaiDam::Nang,    {1.0 / 8,  1.0 / 8}}  // chỉ 1 giá trị, nhưng vẫn dùng pair
};

// Hàm làm tròn lên bội số 50 (mm)
int lam_tron_len_50(double h) {
    return static_cast<int>(std::ceil(h * 1000 / 50.0)) * 50; // h (m) -> mm -> làm tròn
}

// Hàm tính h_dam theo L (m), loại dầm, và làm tròn lên bội 50
int tinh_h_dam(double L, LoaiDam loai) {
    auto range = tyLeDam[loai];
    double h_min = range.first * L;
    double h_max = range.second * L;

    double h_tb = (h_min + h_max) / 2.0;
    return lam_tron_len_50(h_tb); // mm
}

// Hàm tiện lợi để in tên loại dầm
std::string ten_dam(LoaiDam loai) {
    switch (loai) {
        case LoaiDam::DonGian: return "Dầm đơn giản 1 nhịp";
        case LoaiDam::LienTuc: return "Dầm liên tục ≥ 2 nhịp";
        case LoaiDam::CongXon: return "Dầm công xôn (console)";
        case LoaiDam::Nang:    return "Dầm chịu lực nặng";
        default: return "Không xác định";
    }
}

// Ví dụ sử dụng
int main() {
    double L = 6; // chiều dài nhịp dầm (m)

    for (auto loai : {LoaiDam::DonGian, LoaiDam::LienTuc, LoaiDam::CongXon, LoaiDam::Nang}) {
        int h = tinh_h_dam(L, loai);
        std::cout << "🔹 " << ten_dam(loai) << " (L = " << L << " m): h ≥ " << h << " mm\n";
    }

    return 0;
}
