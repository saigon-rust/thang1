#include <iostream>
#include <algorithm>

#include <iomanip>
#include <vector>
#include <cmath>

class PhuongAnThep {
private:
    int duongKinh;           // ∅ thép
    int soThanhTrenMet;      // số thanh / mét
    double khoangCach;       // mm
    double AsCungCap;        // mm²/m

public:
    PhuongAnThep() : duongKinh(0), soThanhTrenMet(0), khoangCach(0), AsCungCap(0) {}

    bool goiY(double As_yeu_cau) {
        std::vector<int> danhSachDuongKinh = {10, 12, 14, 16, 18, 20};
        for (int d : danhSachDuongKinh) {
            double A1 = (M_PI * d * d) / 4.0;
            int n = std::ceil(As_yeu_cau / A1);
            double s = 1000.0 / n;
            double AsCap = n * A1;

            if (s >= 80 && s <= 250) {
                duongKinh = d;
                soThanhTrenMet = n;
                khoangCach = s;
                AsCungCap = AsCap;
                return true;
            }
        }
        return false;
    }

    void hienThi() const {
        if (duongKinh == 0) {
            std::cout << "\n⚠️ Không có phương án thép phù hợp!\n";
            return;
        }

        std::cout << std::fixed << std::setprecision(1);
        std::cout << "\n🔩 Đề xuất cấu hình thép sàn:\n";
        std::cout << "   • Đường kính: ∅" << duongKinh << " mm\n";
        std::cout << "   • Khoảng cách đặt thanh: " << khoangCach << " mm\n";
        std::cout << "   • Diện tích As cung cấp: ≈ " << AsCungCap << " mm²/m\n";
    }
};

class San {
    public:
        double Lx, Ly;   // Kích thước sàn (m)
        double q_san;       // Tải trọng sàn (kN/m2)

        double h_san;
        double q_ngan;
        double q_dai;
        San(double Lx_in, double Ly_in, double q_in = 7)
            :Lx(Lx_in), Ly(Ly_in), q_san(q_in),
            h_san(std::max(0.12, std::min(Lx_in, Ly_in) / 35)),
            q_ngan(q_in * Lx_in / 2),
            q_dai(q_in * Ly_in / 2) {}

        void tinh_hsan_1phuong(double L_1phuong) {
            h_san = std::max(0.12, L_1phuong / 30);
        }

        void tinh_hsan_congxon(double L_congxon) {
            h_san = std::max(0.12, L_congxon / 10);
        }

        void tinh_thepsan() {
            double tyLe = Lx / Ly;
            double alpha_nhip = 0.045;
    
            if (tyLe >= 0.95) alpha_nhip = 0.045;
            else if (tyLe >= 0.75) alpha_nhip = 0.050;
            else alpha_nhip = 0.060;
    
            double M = alpha_nhip * q_san * Lx * Lx;
            std::cout << "→ Mô men uốn giữa nhịp: " << M << " kNm\n";
    
            double Rs = 210; // MPa
            double h0 = h_san * 1000 - 20; // mm
            double As = (M * 1e6) / (Rs * h0); // mm²/m
    
            std::cout << "➡️  Diện tích cốt thép yêu cầu As: " << std::ceil(As) << " mm²/m\n";
            PhuongAnThep phuongan;
            phuongan.goiY(As);
            phuongan.hienThi();
        }
    };

class Dam {
    public:
        double L_dam;
        double h_dam;
        double R_s;
        double R_b;
        double Rs;
        double Rb;

        Dam(double Rs_in = 300, double Rb_in = 17)
            :Rs(Rs_in / 1.15), Rb(Rb_in / 1.5) {}
    };
class AsCalculator {
    public:
        double Rs;   // Cường độ chịu kéo của thép (MPa)
        double M;    // Mô men uốn (kNm)
        double h0;   // Chiều cao làm việc (mm)
        double j;    // Hệ số đòn bẩy (thường ≈ 0.9)
        double As;   // Kết quả diện tích cốt thép (mm^2)

        AsCalculator(double M_in, double Rs_in, double h0_in, double j_in = 0.9)
            : M(M_in), Rs(Rs_in), h0(h0_in), j(j_in)
        {
            As = (M) / (Rs * j * h0);  // M đơn vị kNm → N.mm
        }

        void printValues() const {
            std::cout << "\n--- Tính diện tích thép As (gần đúng) ---\n";
            std::cout << "==> As ≈ " << As << " mm^2\n";
        }
    };

int main() {
    San san(6, 4);
    std::cout << "🔸 Tải lên dầm DÀI:  " << san.q_ngan << " kN/m\n";
    san.tinh_thepsan();
    return 0;
}