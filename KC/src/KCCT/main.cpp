#include <iostream>
#include <algorithm>
#include <iomanip>
#include <vector>
#include <cmath>

using namespace std;

struct SteelBar {
    int phi;       // Đường kính (mm)
    double area;   // Diện tích (mm²)
};

class VatLieu {
public:
    string loai_be_tong, loai_thep, loai_tuong;
    double R_b = 0, R_s = 0, Rs = 0, Rb = 0;
    double gamma = 0;
    double dtuong = 0.23;
    double htuong = 3.0;
    double qtuong = 0;
    double qsan= 7;

    void nhapVatLieu() {
        cout << "\n📥 Nhập loại bê tông (M200, M250, M300, M350, M400): ";
        cin >> loai_be_tong;
        cout << "📥 Nhập loại thép (CB300, CB400): ";
        cin >> loai_thep;
        cout << "📥 Nhập loại tường (gach_the, gach_ong, gach_bong_gio): ";
        cin >> loai_tuong;

        if (loai_be_tong == "M200") R_b = 11.5;
        else if (loai_be_tong == "M250") R_b = 14.5;
        else if (loai_be_tong == "M300") R_b = 17;
        else if (loai_be_tong == "M350") R_b = 19.5;
        else if (loai_be_tong == "M400") R_b = 22.5;
        else {
            cout << "⚠️ Loại bê tông không hợp lệ, dùng mặc định M250.\n";
            R_b = 14.5;
            loai_be_tong = "M250";
        }

        if (loai_thep == "CB300") R_s = 300;
        else if (loai_thep == "CB400") R_s = 400;
        else {
            cout << "⚠️ Loại thép không hợp lệ, dùng mặc định CB300.\n";
            R_s = 300;
            loai_thep = "CB300";
        }

        if (loai_tuong == "gach_the") gamma = 19.0;
        else if (loai_tuong == "gach_ong") gamma = 18.0;
        else if (loai_tuong == "gach_nhe") gamma = 13.5;
        else {
            cout << "⚠️ Loại tường không hợp lệ, dùng mặc định gach_the.\n";
            gamma = 19.0;
            loai_tuong = "gach_the";
        }

        Rs = R_s / 1.5;
        Rb = R_b / 1.15;
        qtuong = gamma * htuong * dtuong;
    }

    void hienThi() const {
        cout << "\n✅ Thông tin vật liệu:\n";
        cout << "   • Bê tông: " << loai_be_tong << " (Rb = " << R_b << " MPa)\n";
        cout << "   • Thép   : " << loai_thep << " (Rs = " << R_s << " MPa)\n";
        cout << "   • Tường  : " << loai_tuong << " (gamma = " << gamma << " kN/m³)\n";
    }
};

class San {
private:
    int duongKinh = 0;
    int soThanhTrenMet = 0;
    double khoangCach = 0;
    double AsCungCap = 0;

    double Rs, Rb;

public:
    double hsan;
    double As = 0;
    double qsan;

    San(const VatLieu& vl)
        :Rs(vl.Rs), Rb(vl.Rb), qsan(vl.qsan) {}

    double q_DS(double DS) {
        return qsan * DS / 4;
    }

    bool goiY(double As_yeu_cau) {
        vector<int> danhSachDuongKinh = {10, 12, 14, 16, 18, 20};
        for (int d : danhSachDuongKinh) {
            double A1 = M_PI * d * d / 4.0;
            int n = ceil(As_yeu_cau / A1);
            double s = 1000.0 / n;
            if (s >= 80 && s <= 250) {
                duongKinh = d;
                soThanhTrenMet = n;
                khoangCach = s;
                AsCungCap = n * A1;
                return true;
            }
        }
        return false;
    }

    void tinh_As(double Lx,double Ly) {
        hsan = ceil(max(0.12, min(Lx, Ly) / 35.0)/0.01)*0.01;
        double tyLe = Lx / Ly;
        double alpha;

        if (tyLe >= 0.95) alpha = 0.045;
        else if (tyLe >= 0.75) alpha = 0.050;
        else alpha = 0.060;

        double M = alpha * qsan * Lx * Lx;
        cout << "\n→ Mô men uốn giữa nhịp: " << M << " kNm\n";

        double h0 = hsan * 1000 - 25;
        As = (M * 1e6) / (Rs * h0);

        cout << "➡️  Diện tích thép yêu cầu As: " << ceil(As) << " mm²/m\n";
        if (!goiY(As)) {
            cout << "⚠️ Không tìm được phương án bố trí thép phù hợp!\n";
        }
    }

    void hienThi() const {
        if (duongKinh == 0) return;
        cout << fixed << setprecision(1);
        std::cout << std::fixed << std::setprecision(2) << "  hsan = " << hsan << "m\n";
        cout << "🔩 Đề xuất thép sàn:\n";
        cout << "   • ∅" << duongKinh << " mm, cách nhau " << khoangCach << " mm\n";
        cout << "   • As cung cấp ≈ " << AsCungCap << " mm²/m\n";
    }
};

class Dam {
private:
    double b;      // Bề rộng dầm (mm)
    double Rb;     // Cường độ bê tông (MPa)
    double Rs;     // Cường độ chịu kéo của thép (MPa)
    double alpha;  // Hệ số giảm mô men cho phép (0.8 - 1.0)
    double As;

public:
    Dam(const VatLieu& vl, double b_in = 200, double alpha_in = 0.9)
        : b(b_in), Rb(vl.Rb), Rs(vl.Rs), alpha(alpha_in) {}

    void tinh_As(double L, double q, double P = 0, double a =0) {
        std::cout << "\n=== TÍNH TOÁN THÉP CHO DẦM ===\n";
        std::cout << std::fixed << std::setprecision(2);

        // 1. Mô men uốn lớn nhất
        double Mq = (q * L * L) / 8.0;       // kNm
        double Mp = (P * L) / 4.0;           // kNm
        double Mmax = Mq + Mp;               // kNm

        // 2. Chiều cao sơ bộ dầm và chiều cao hữu hiệu
        double h = ceil((L * 1000.0) / 12 / 50)*50;      // mm (giả thiết h = L/10)
        double h0 = h - 25;                   // mm

        // 3. Diện tích thép yêu cầu
        As = (Mmax * 1e6) / (Rs * h0);  // mm²

        // 4. Mô men cho phép Mcp
        double Mcp = alpha * Rb * b * h0 * h0 / 1e6;  // kNm

        // 5. Hiển thị kết quả
        std::cout << "Nhịp dầm         L   = " << L << " m\n";
        std::cout << "Tải phân bố      q   = " << q << " kN/m\n";
        std::cout << "Tải tập trung    P   = " << P << " kN\n";
        std::cout << "M_max                = " << Mmax << " kNm\n";
        std::cout << "Chiều cao dầm   h   = " << h << " mm\n";
        std::cout << "Chiều cao hữu hiệu h0 = " << h0 << " mm\n";
        std::cout << "Diện tích thép yêu cầu As = " << As << " mm²\n";
        std::cout << "Mô men cho phép Mcp     = " << Mcp << " kNm\n";

        // 6. Kiểm tra an toàn
        std::cout << "=> " << ((Mmax <= Mcp) ? "✅ Dầm đạt yêu cầu chịu uốn." : "❌ KHÔNG đạt yêu cầu chịu uốn.") << "\n";
    }
    void hienThi() {
        std::vector<SteelBar> bars = {
            {10, 78.5}, {12, 113}, {14, 154}, {16, 201},
            {18, 254}, {20, 314}, {22, 380}, {25, 491}
        };

        std::cout << "\n--- Gợi ý bố trí thép gần đúng cho As yêu cầu: "
                  << std::round(As) << " mm² ---\n";
        std::cout << std::left << std::setw(8) << "Φ"
                  << std::setw(12) << "Số lượng"
                  << std::setw(15) << "As thực tế" << "\n";

        for (const auto& bar : bars) {
            int n = std::ceil(As / bar.area);
            double As_actual = n * bar.area;
            std::cout << "Φ" << std::setw(7) << bar.phi
                      << std::setw(12) << n
                      << std::setw(15) << std::round(As_actual) << "\n";
        }
    }
};


class Cot {
private:
    double RA = 0;
    double Rb, Rs;

public:
    Cot(const VatLieu& vl) : Rb(vl.Rb), Rs(vl.Rs) {}

    void tinh_RA(double L, double q, double P = 0, double a = 0) {
        RA += (q * L) / 2 + (P * (L - a)) / L;
    }

    void hienThi() const {
        cout << "\n📌 Tổng tải trọng lên cột A: " << RA << " kN\n";
    }

    double tinh_As(double N, double Rsc) {
        return (N * 1e3) / Rsc;
    }

    bool kiem_tra_manh_cot(double b, double h, double l0) const {
        double I = (b * pow(h, 3)) / 12.0;
        double Ac = b * h;
        double i = sqrt(I / Ac);
        double lambda = l0 / i;
        return lambda <= 70.0;
    }

    void chon_tiet_dien(double l0 = 3.0) {
        vector<int> kich_thuoc = {200, 250, 300, 350, 400};

        cout << "\n📊 Đánh giá các phương án cột...\n";
        for (int b : kich_thuoc) {
            for (int h : kich_thuoc) {
                double Ac = b * h;
                double Rc = 0.9 * Rb * Ac;
                double N_bt = Rc / 1e3;
                if (RA <= N_bt * 1.2 && kiem_tra_manh_cot(b, h, l0)) {
                    double As = tinh_As(RA, Rs);

                    vector<int> duong_kinh = {14, 16, 18, 20, 22, 25};
                    for (int d : duong_kinh) {
                        double As1 = M_PI * d * d / 4.0;
                        int n = max(4, (int)ceil(As / As1));
                        if (n % 2 != 0) ++n;
                        double As_cap = n * As1;

                        cout << "✅ Phương án hợp lệ:\n";
                        cout << "   - Cột: " << b << " x " << h << " mm\n";
                        cout << "   - As yêu cầu: " << ceil(As) << " mm²\n";
                        cout << "   - Bố trí: " << n << " thanh ∅" << d << " (As cung cấp: " << As_cap << " mm²)\n";
                        cout << "   - Kiểm tra mảnh cột: Đạt\n";
                        return;
                    }
                }
            }
        }
        cout << "\n⚠️ Không tìm được phương án cột phù hợp!\n";
    }
};

int main() {
    VatLieu vatlieu;
    vatlieu.nhapVatLieu();
    vatlieu.hienThi();

    double Lx = 4, Ly = 6;
    San san(vatlieu);
    san.tinh_As(Lx, Ly);
    san.hienThi();

    double DS1 = 4;
    double DS2 = 2;
    Cot cotA(vatlieu);
    cotA.tinh_RA(DS1, san.q_DS(DS1+DS2) + vatlieu.qtuong/2);
    cotA.tinh_RA(DS1, san.q_DS(DS1*2) + vatlieu.qtuong/2);
    cotA.tinh_RA(DS1, san.q_DS(DS1+DS2) + vatlieu.qtuong/2);
    cotA.tinh_RA(DS2, san.q_DS(DS1*2));
    cotA.hienThi();
    cotA.chon_tiet_dien();

    Dam dam(vatlieu);
    dam.tinh_As(DS1, san.q_DS(DS1*2) + vatlieu.qtuong/2);
    dam.hienThi();

    return 0;
}
