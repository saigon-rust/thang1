#include <iostream>
#include <cmath>
#include <vector>
#include <iomanip>
using namespace std;

// Phản lực gối từ tải phân bố đều
double tinh_RA(double L, double q, double P = 0, double a = 0) {
    return (q * L) / 2 + (P * (L - a)) / L;
}

// Tải trọng bản sàn (bê tông + hoàn thiện + sử dụng)
double tinh_qdam(double Lx) {
    double qs = 25 * 0.12 + 2 + 2;
    return qs * Lx / 2;
}

// Tải trọng tường gạch
double tinh_qtuong() {
    return 18 * 0.23 * 3;
}

// Diện tích thép yêu cầu
double tinh_As(double N, double Rsc) {
    return (N * 1e3) / Rsc;
}

// Kiểm tra điều kiện mảnh cột
bool kiem_tra_manh_cot(double b, double h, double l0) {
    double I = (b * pow(h, 3)) / 12.0;
    double Ac = b * h;
    double i = sqrt(I / Ac);
    double lambda = l0 / i;
    return lambda <= 70.0;
}

// Đề xuất phương án bố trí thép
struct PhuongAnThep {
    int so_thanh;
    int duong_kinh;
    double As_cung_cap;
};

PhuongAnThep tim_thep(double As_yeu_cau) {
    vector<int> duong_kinh = {14, 16, 18, 20, 22, 25};
    for (int d : duong_kinh) {
        double As1 = M_PI * d * d / 4.0;
        int so_thanh = ceil(As_yeu_cau / As1);
        if (so_thanh < 4) so_thanh = 4;
        if (so_thanh % 2 != 0) ++so_thanh;
        double As_cung_cap = so_thanh * As1;
        return {so_thanh, d, As_cung_cap};
    }
    return {0, 0, 0};
}

// Tự động chọn tiết diện cột và xuất báo cáo
void chon_tiet_dien(double N, double Rb, double Rsc, double l0) {
    vector<int> kich_thuoc = {200, 250, 300, 350, 400};

    cout << "\n📊 Dang danh gia cac phuong an cot...\n";

    for (int b : kich_thuoc) {
        for (int h : kich_thuoc) {
            double Ac = b * h; // mm2
            double Rc = 0.9 * Rb * Ac; // kháng lực bê tông
            double N_bt = Rc / 1e3; // đổi về kN

            if (N <= N_bt * 1.2) { // có khả năng chịu nén
                if (kiem_tra_manh_cot(b, h, l0)) {
                    double As = tinh_As(N, Rsc);
                    PhuongAnThep ph = tim_thep(As);

                    cout << "\n✅ Phuong an hop le:\n";
                    cout << "   - Tiet dien cot: " << b << " x " << h << " mm\n";
                    cout << "   - As yeu cau: " << ceil(As) << " mm2\n";
                    cout << "   - Bo tri: " << ph.so_thanh << " thanh \u03d5" << ph.duong_kinh << " (As cung cap: " << ph.As_cung_cap << " mm2)\n";
                    cout << "   - Kiem tra manh cot: Dat\n";
                    return;
                }
            }
        }
    }
    cout << "\n⚠️ Khong tim duoc phuong an tiet dien phu hop!\n";
}

int main() {
    // B1. Tính tổng tải trọng lên cột D
    double qdam = tinh_qdam(4);
    double qtuong = tinh_qtuong();
    double RD = tinh_RA(4, qdam + qtuong) * 3 + tinh_RA(4, tinh_qdam(2)); // tổng 3 dầm + 1 ô 2m

    cout << fixed << setprecision(2);
    cout << "📌 Tong tai trong tac dung len cot D: " << RD << " kN\n";

    // B2. Đầu vào vật liệu
    double Rb = 11.5;  // MPa (B20)
    double Rsc = 300;  // MPa (thép CB400)
    double l0 = 3000;  // chiều cao tính toán cột (mm)

    // B3. Đề xuất tiết diện + cốt thép
    chon_tiet_dien(RD, Rb, Rsc, l0);

    return 0;
}
