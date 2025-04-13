#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

struct SteelBar {
    int phi;           // Đường kính (mm)
    double area;       // Diện tích mặt cắt (mm2)
};

class Dam {
private:
    double L;      // Nhịp dầm (m)
    double b;      // Bề rộng dầm (mm)
    double h;      // Chiều cao dầm (mm)
    double a;      // Lớp bảo vệ cốt thép (mm)
    double q;      // Tải phân bố đều (kN/m)

    // Vật liệu
    double Rb;     // Cường độ bê tông chịu nén (MPa)
    double Rs;     // Cường độ chịu kéo cốt thép (MPa)
    double alpha;  // Hệ số điều kiện làm việc (0.8 ~ 1.0)

public:
    Dam(double L_, double b_, double h_, double a_, double q_,
        double Rb_ = 11.5, double Rs_ = 280, double alpha_ = 0.9)
        : L(L_), b(b_), h(h_), a(a_), q(q_), Rb(Rb_), Rs(Rs_), alpha(alpha_) {}

    double tinh_Mmax() const {
        return (q * L * L) / 8.0; // kNm
    }

    double tinh_h0() const {
        return h - a; // mm
    }

    double tinh_As(double Mmax, double h0) const {
        return (Mmax * 1e6) / (Rs * h0); // mm2
    }

    double tinh_Mcp(double h0) const {
        return (alpha * Rb * b * h0 * h0) / 1e6; // kNm
    }

    void goi_y_thep(double As_required) const {
        std::vector<SteelBar> bars = {
            {10, 78.5}, {12, 113}, {14, 154}, {16, 201},
            {18, 254}, {20, 314}, {22, 380}, {25, 491}
        };

        std::cout << "\n--- Gợi ý bố trí thép gần đúng cho As yêu cầu: " 
                  << std::round(As_required) << " mm² ---\n";
        std::cout << std::left << std::setw(8) << "Φ" 
                  << std::setw(12) << "Số lượng" 
                  << std::setw(15) << "As thực tế" << "\n";

        for (const auto& bar : bars) {
            int n = std::ceil(As_required / bar.area);
            double As_actual = n * bar.area;
            std::cout << "Φ" << std::setw(7) << bar.phi 
                      << std::setw(12) << n 
                      << std::setw(15) << std::round(As_actual) << "\n";
        }
    }

    void hien_thi() {
        double Mmax = tinh_Mmax();       // kNm
        double h0 = tinh_h0();           // mm
        double As = tinh_As(Mmax, h0);   // mm2
        double Mcp = tinh_Mcp(h0);       // kNm

        std::cout << "\n=== KẾT QUẢ TÍNH TOÁN DẦM ===\n";
        std::cout << "Mô men uốn lớn nhất       Mmax = " << Mmax << " kNm\n";
        std::cout << "Chiều cao hữu hiệu        h0   = " << h0 << " mm\n";
        std::cout << "Diện tích cốt thép yêu cầu As   = " << As << " mm²\n";
        std::cout << "Mô men kháng uốn cho phép Mcp  = " << Mcp << " kNm\n";

        if (Mmax <= Mcp)
            std::cout << "✅ Dầm ĐẠT yêu cầu chịu uốn.\n";
        else
            std::cout << "❌ Dầm KHÔNG ĐẠT yêu cầu chịu uốn.\n";

        goi_y_thep(As);
    }
};
int main() {
    double L = 6.0;     // m
    double b = 200.0;   // mm
    double h = 400.0;   // mm
    double a = 25.0;    // lớp bảo vệ mm
    double q = 26.42;    // kN/m

    Dam dam(L, b, h, a, q);
    dam.hien_thi();

    return 0;
}
