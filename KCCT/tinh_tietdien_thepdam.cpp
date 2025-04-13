#include <iostream>
#include <cmath>
#include <vector>
#include <iomanip>

struct SteelBar {
    int phi;           // Đường kính (mm)
    double area;       // Diện tích (mm2)
};

class Dam {
private:
    double L;       // Nhịp dầm (m)
    double b;       // Bề rộng dầm (mm)
    double q;       // Tải phân bố đều (kN/m)
    double P;       // Tải tập trung giữa nhịp (kN)
    double Rb;      // Cường độ bê tông (MPa)
    double Rs;      // Cường độ chịu kéo của thép (MPa)
    double alpha;   // Hệ số tính mô men cho phép (0.8-1.0)
    double h0;      // Chiều cao hữu hiệu (mm)

public:
    Dam(double L_, double b_, double q_, double P_, double Rb_ = 11.5, double Rs_ = 280, double alpha_ = 0.8)
        : L(L_), b(b_), q(q_), P(P_), Rb(Rb_), Rs(Rs_), alpha(alpha_), h0(0) {}

    double tinh_Mmax() {
        double Mq = (q * L * L) / 8.0;
        double Mp = (P * L) / 4.0;
        return Mq + Mp; // kNm
    }

    double tinh_h0() {
        double Mmax = tinh_Mmax(); // kNm
        double h0_temp = sqrt((Mmax * 1e6) / (Rb * b * alpha)); // mm
        h0 = std::ceil(h0_temp / 50.0) * 50;
        return h0;
    }

    double tinh_As() {
        double Mmax = tinh_Mmax(); // kNm
        double h0_use = (h0 > 0) ? h0 : tinh_h0(); // đảm bảo h0 có giá trị
        return (Mmax * 1e6) / (Rs * h0_use); // mm2
    }

    double tinh_Mcp() {
        double h0_use = (h0 > 0) ? h0 : tinh_h0();
        return (Rb * b * h0_use * h0_use * alpha) / 1e6; // kNm
    }

    void goi_y_thep(double As_required) {
        std::vector<SteelBar> bars = {
            {10, 78.5}, {12, 113}, {14, 154}, {16, 201},
            {18, 254}, {20, 314}, {22, 380}, {25, 491}
        };

        std::cout << "\n--- Gợi ý chọn thép cho As yêu cầu: " << As_required << " mm^2 ---\n";
        std::cout << std::left << std::setw(8) << "Φ" 
                  << std::setw(12) << "Số lượng" 
                  << std::setw(15) << "As thực tế" << "\n";

        for (const auto& bar : bars) {
            int n = std::ceil(As_required / bar.area);
            double As_actual = n * bar.area;
            std::cout << "Φ" << std::setw(7) << bar.phi 
                      << std::setw(12) << n 
                      << std::setw(15) << As_actual << "\n";
        }
    }

    void hien_thi() {
        double Mmax = tinh_Mmax();
        double h0_value = tinh_h0();
        double As = tinh_As();
        double Mcp = tinh_Mcp();

        std::cout << "\n=== TÍNH TOÁN DẦM ===\n";
        std::cout << "M_max = " << Mmax << " kNm\n";
        std::cout << "h0 (chiều cao hữu hiệu) = " << h0_value << " mm\n";
        std::cout << "As yêu cầu = " << As << " mm^2\n";
        std::cout << "Mcp (mô men cho phép) = " << Mcp << " kNm\n";
        std::cout << "=> " << ((Mmax <= Mcp) ? "ĐẠT yêu cầu chịu lực ✅" : "KHÔNG ĐẠT ❌") << "\n";

        goi_y_thep(As);
    }
};

int main() {
    double L = 6.0;      // Nhịp dầm (m)
    double b = 200.0;    // Bề rộng dầm (mm)
    double q = 29.0;     // Tải phân bố đều (kN/m)
    double P = 10.0;     // Tải tập trung giữa nhịp (kN)

    Dam dam(L, b, q, P);
    dam.hien_thi();

    return 0;
}
