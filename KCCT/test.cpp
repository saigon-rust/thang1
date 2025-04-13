#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <iomanip>

struct PhuongAnThep {
    int duongKinh;
    int soThanhTrenMet;
    double khoangCach;
    double AsCungCap;

    void hien_thi() const {
        std::cout << std::fixed << std::setprecision(1);
        std::cout << "\n🔩 Đề xuất cấu hình thép sàn:\n";
        std::cout << "   • Đường kính: ∅" << duongKinh << " mm\n";
        std::cout << "   • Khoảng cách đặt thanh: " << khoangCach << " mm\n";
        std::cout << "   • Diện tích As cung cấp: ≈ " << AsCungCap << " mm²/m\n";
    }
};

PhuongAnThep goi_y_duong_kinh_thep(double As_yeu_cau) {
    std::vector<int> danhSachDuongKinh = {10, 12, 14, 16, 18, 20};
    for (int d : danhSachDuongKinh) {
        double A1 = (M_PI * d * d) / 4.0;
        int n = std::ceil(As_yeu_cau / A1);
        double s = 1000.0 / n;
        double AsCap = n * A1;

        if (s >= 80 && s <= 250) {
            return {d, n, s, AsCap};
        }
    }

    std::cout << "\n⚠️  Không tìm thấy phương án phù hợp với As = " << As_yeu_cau << " mm²/m.\n";
    return {0, 0, 0, 0};
}

class San {
private:
    double Lx, Ly;

public:
    San(double Lx_in, double Ly_in) : Lx(Lx_in), Ly(Ly_in) {}

    double tinh_chieu_day_san() {
        int loaiSan;
        std::cout << "\nChọn loại sàn:\n"
                     "1. Sàn kê 4 cạnh (2 phương)\n"
                     "2. Sàn kê 2 cạnh (1 phương)\n"
                     "3. Sàn công xôn\n"
                     "Nhập lựa chọn (1-3): ";
        std::cin >> loaiSan;

        double L = std::min(Lx, Ly);
        double h = 0.12; // m - chiều dày tối thiểu

        switch (loaiSan) {
            case 1: h = L / 35.0; break;
            case 2: {
                std::cout << "Chọn phương chịu lực chính (1 = Lx, 2 = Ly): ";
                int chon; std::cin >> chon;
                L = (chon == 1) ? Lx : Ly;
                h = L / 30.0;
                break;
            }
            case 3: {
                std::cout << "Chọn phương công xôn (1 = Lx, 2 = Ly): ";
                int chon; std::cin >> chon;
                L = (chon == 1) ? Lx : Ly;
                h = L / 10.0;
                break;
            }
            default:
                std::cout << "⛔️ Lựa chọn không hợp lệ. Mặc định sàn kê 4 cạnh.\n";
                h = L / 35.0;
        }

        if (h < 0.12) h = 0.12;
        h = std::ceil(h * 1000) / 1000.0;
        std::cout << "➡️  Chiều dày sàn đề xuất: " << h << " m (" << h * 1000 << " mm)\n";
        return h;
    }

    double tinh_tai_trong_san(double h_san) {
        int loaiCT;
        const double g2 = 2.0; // tải hoàn thiện, trần
        const double lbt = 25.0; // kN/m3
        double g1 = lbt * h_san;
        double qsd = 2.0;

        std::cout << "\nChọn loại công trình:\n"
                     "1. Mái không người (1.0 kN/m²)\n"
                     "2. Nhà ở (2.0 kN/m²)\n"
                     "3. Văn phòng (3.0 kN/m²)\n"
                     "4. Hành lang công cộng (5.0 kN/m²)\n"
                     "Nhập lựa chọn (1-4): ";
        std::cin >> loaiCT;

        switch (loaiCT) {
            case 1: qsd = 1.0; break;
            case 2: qsd = 2.0; break;
            case 3: qsd = 3.0; break;
            case 4: qsd = 5.0; break;
            default:
                std::cout << "Lựa chọn không hợp lệ. Mặc định nhà ở.\n";
        }

        double q = g1 + g2 + qsd;
        std::cout << "\n✅ Tổng tải trọng sàn: q = " << q << " kN/m²\n";
        return q;
    }

    void tinh_tai_len_dam(double q) {
        double Q = q * Lx * Ly;
        double q_ngan = q * Lx / 2.0;
        double q_dai = q * Ly / 2.0;

        std::cout << "\n🔸 Tổng tải sàn: Q = " << Q << " kN\n";
        std::cout << "🔸 Tải lên dầm NGẮN: " << q_ngan << " kN/m\n";
        std::cout << "🔸 Tải lên dầm DÀI:  " << q_dai << " kN/m\n";
    }

    void tinh_thep_san(double h_san, double q) {
        double tyLe = Lx / Ly;
        double alpha_nhip = 0.045;

        if (tyLe >= 0.95) alpha_nhip = 0.045;
        else if (tyLe >= 0.75) alpha_nhip = 0.050;
        else alpha_nhip = 0.060;

        double M = alpha_nhip * q * Lx * Lx;
        std::cout << "→ Mô men uốn giữa nhịp: " << M << " kNm\n";

        double R = 210; // MPa
        double h0 = h_san * 1000 - 20; // mm
        double As = (M * 1e6) / (R * h0); // mm²/m

        std::cout << "➡️  Diện tích cốt thép yêu cầu As: " << std::ceil(As) << " mm²/m\n";
        PhuongAnThep phuongAn = goi_y_duong_kinh_thep(As);
        phuongAn.hien_thi();
    }
};

int main() {
    double Lx, Ly;
    std::cout << "Nhập chiều dài sàn Lx (m): "; std::cin >> Lx;
    std::cout << "Nhập chiều rộng sàn Ly (m): "; std::cin >> Ly;

    San san(Lx, Ly);

    double h_san = san.tinh_chieu_day_san();
    double q = san.tinh_tai_trong_san(h_san);
    san.tinh_tai_len_dam(q);
    san.tinh_thep_san(h_san, q);

    return 0;
}