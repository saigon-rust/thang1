#include <iostream>
#include <algorithm>
#include <iomanip>
#include <vector>
#include <cmath>

using namespace std;

class VatLieu {
public:
    string loai_be_tong, loai_thep, loai_tuong;
    double R_b, R_s, gamma;

    VatLieu() : R_b(0), R_s(0), gamma(0) {}

    void nhapVatLieu() {
        cout << "\n📥 Nhập loại bê tông (M200, M250, M300, M350, M400): ";
        cin >> loai_be_tong;

        cout << "📥 Nhập loại thép (CB300, CB400): ";
        cin >> loai_thep;

        cout << "📥 Nhập loại tường (gach_the, gach_ong, gach_bong_gio, kinh): ";
        cin >> loai_tuong;

        if (loai_be_tong == "M200") R_b = 11.5;
        else if (loai_be_tong == "M250") R_b = 14.5;
        else if (loai_be_tong == "M300") R_b = 17;
        else if (loai_be_tong == "M350") R_b = 19.5;
        else if (loai_be_tong == "M400") R_b = 22.5;
        else {
            cout << "⚠️ Loại bê tông không hợp lệ, dùng mặc định M250 (Rb = 14.5 MPa).\n";
            R_b = 14.5;
            loai_be_tong = "M250";
        }

        if (loai_thep == "CB300") R_s = 300;
        else if (loai_thep == "CB400") R_s = 400;
        else {
            cout << "⚠️ Loại thép không hợp lệ, dùng mặc định CB300 (Rs = 300 MPa).\n";
            R_s = 300;
            loai_thep = "CB300";
        }

        if (loai_tuong == "gach_the") gamma = 19.0;
        else if (loai_tuong == "gach_ong") gamma = 18.0;
        else if (loai_tuong == "gach_bong_gio") gamma = 13.5;
        else {
            cout << "⚠️ Loại tường không hợp lệ, dùng mặc định gach_the (γ = 19.0 kN/m³).\n";
            loai_tuong = "gach_the";
            gamma = 19.0;
        }
    }

    void hienThi() {
        cout << "\n✅ Đã nhập vật liệu:\n";
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

public:
    double R_s, R_b, Rs, Rb;
    double Lx, Ly;
    double q_san;
    double hsan;
    double q_ngan, q_dai;
    double As = 0;

    San(const VatLieu &vl, double Lx_in, double Ly_in, double q_in = 7)
        : Lx(Lx_in), Ly(Ly_in), q_san(q_in),
          R_s(vl.R_s), R_b(vl.R_b), Rs(vl.R_s / 1.5), Rb(vl.R_b / 1.15),
          hsan(max(0.12, min(Lx_in, Ly_in) / 35)),
          q_ngan(q_in * Lx_in / 2),
          q_dai(q_in * Ly_in / 2) {}

    void tinh_hsan_1phuong(double L_1phuong) {
        hsan = max(0.12, L_1phuong / 30);
    }

    void tinh_hsan_congxon(double L_congxon) {
        hsan = max(0.12, L_congxon / 10);
    }

    bool goiY(double As_yeu_cau) {
        vector<int> danhSachDuongKinh = {10, 12, 14, 16, 18, 20};
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
            cout << "\n⚠️ Không có phương án thép phù hợp!\n";
            return;
        }

        cout << std::fixed << std::setprecision(1);
        cout << "\n🔩 Đề xuất cấu hình thép sàn:\n";
        cout << "   • Đường kính: ∅" << duongKinh << " mm\n";
        cout << "   • Khoảng cách đặt thanh: " << khoangCach << " mm\n";
        cout << "   • Diện tích As cung cấp: ≈ " << AsCungCap << " mm²/m\n";
    }

    void tinh_thepsan() {
        double tyLe = Lx / Ly;
        double alpha_nhip;

        if (tyLe >= 0.95) alpha_nhip = 0.045;
        else if (tyLe >= 0.75) alpha_nhip = 0.050;
        else alpha_nhip = 0.060;

        double M = alpha_nhip * q_san * Lx * Lx;
        cout << "→ Mô men uốn giữa nhịp: " << M << " kNm\n";

        double h0 = hsan * 1000 - 25; // mm
        As = (M * 1e6) / (Rs * h0);   // mm²/m

        cout << "➡️  Diện tích cốt thép yêu cầu As: " << std::ceil(As) << " mm²/m\n";

        if (!goiY(As)) {
            cout << "⚠️ Không tìm được phương án bố trí thép hợp lý!\n";
        }
    }
};

class Dam {
public:
    double L;
    double hdam;
    double R_s, R_b, Rs, Rb;

    Dam(const VatLieu &vl, double L_in, double hdam_in)
        : L(L_in), hdam(hdam_in),
          R_s(vl.R_s), R_b(vl.R_b), Rs(vl.R_s / 1.5), Rb(vl.R_b / 1.15) {}

    void hienThi() const {
        cout << "\n--- Thông số Dầm ---\n";
        cout << "Rs = " << Rs << " MPa\n";
        cout << "Rb = " << Rb << " MPa\n";
    }
};

int main() {
    double Lx = 4;
    double Ly = 6;

    VatLieu vatlieu;
    vatlieu.nhapVatLieu();
    vatlieu.hienThi();

    San san(vatlieu, Lx, Ly);
    san.tinh_thepsan();
    san.hienThi();

    Dam dam(vatlieu, Lx, san.hsan);
    dam.hienThi();

    return 0;
}

