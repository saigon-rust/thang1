#include <iostream>
#include <cmath>
#include <vector>
#include <iomanip>
using namespace std;

// ======== HÀM PHỤ ========

// Tải trọng bản sàn (bê tông + hoàn thiện + sử dụng)
double tinh_qdam(double Lx) {
    double qs = 25 * 0.12 + 2 + 2;
    return qs * Lx / 2;
}

// Mô men uốn cực đại của dầm chịu tải đều: M = qL^2 / 8
double tinh_Mmax(double q, double L) {
    return q * pow(L, 2) / 8.0;
}

// Diện tích thép dầm yêu cầu (As): As = M / (Rs * z)
double tinh_As_dam(double M, double Rs, double h) {
    double z = 0.9 * h; // khoảng cách tính toán đòn tay
    return (M * 1e6) / (Rs * z); // M: kNm → Nmm
}

// Hàm đề xuất bố trí cốt thép dầm
struct PhuongAnThep {
    int so_thanh;
    int duong_kinh;
    double As_cung_cap;
};

PhuongAnThep goi_y_thep_dam(double As_yeu_cau) {
    vector<int> duong_kinh = {14, 16, 18, 20, 22, 25, 28};
    for (int d : duong_kinh) {
        double As1 = M_PI * d * d / 4.0;
        int so_thanh = ceil(As_yeu_cau / As1);
        if (so_thanh < 2) so_thanh = 2;
        if (so_thanh % 2 != 0) ++so_thanh;
        double As_cung_cap = so_thanh * As1;
        return {so_thanh, d, As_cung_cap};
    }
    return {0, 0, 0};
}

// Hàm đề xuất tiết diện dầm
void chon_tiet_dien_dam(double q, double L, double Rs) {
    cout << "\n🔍 Dang danh gia cac phuong an dam...\n";
    double Mmax = tinh_Mmax(q, L);
    vector<int> bw = {200, 250, 300}; // bề rộng dầm
    vector<int> h = {400, 450, 500, 550}; // chiều cao dầm

    for (int b : bw) {
        for (int H : h) {
            double h_tinh = H - 30; // chiều cao hữu hiệu, trừ lớp bảo vệ
            double As = tinh_As_dam(Mmax, Rs, h_tinh);
            PhuongAnThep ph = goi_y_thep_dam(As);

            if (ph.so_thanh > 0) {
                cout << fixed << setprecision(1);
                cout << "\n✅ Phuong an dam hop le:\n";
                cout << "   - Tiet dien dam: " << b << " x " << H << " mm\n";
                cout << "   - Mmax = " << Mmax << " kNm\n";
                cout << "   - As yeu cau: " << ceil(As) << " mm2\n";
                cout << "   - Bo tri: " << ph.so_thanh << " thanh \u03d5" << ph.duong_kinh
                     << " (As cung cap: " << ph.As_cung_cap << " mm2)\n";
                return;
            }
        }
    }
    cout << "\n⚠️ Khong tim duoc phuong an tiet dien dam phu hop!\n";
}

// ======== MAIN ========
int main() {
    double L = 4.0;         // chiều dài nhịp dầm (m)
    double q = tinh_qdam(L); // tải trọng phân bố (kN/m)
    double Rs = 280;        // cường độ thiết kế của thép chịu kéo (MPa)

    cout << fixed << setprecision(2);
    cout << "📌 Tai trong phan bo tren dam: q = " << q << " kN/m\n";

    chon_tiet_dien_dam(q, L, Rs);

    return 0;
}
