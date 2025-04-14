#include <iostream>
#include <algorithm>
#include <iomanip>
#include <vector>
#include <cmath>

using namespace std;

class VatLieu {
public:
    string loai_be_tong, loai_thep, loai_tuong;
    double R_b = 0, R_s = 0, Rs = 0, Rb = 0;
    double gamma = 0;
    double dtuong = 0.23;
    double htuong = 3.0;
    double qtuong = 0;

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

    double Lx, Ly, q_san;
    double Rs, Rb;

public:
    double hsan;
    double q_Lx, q_Ly;
    double As = 0;

    San(const VatLieu& vl, double lx, double ly, double q = 7.0)
        : Lx(lx), Ly(ly), q_san(q), Rs(vl.Rs), Rb(vl.Rb) {
        hsan = max(0.12, min(Lx, Ly) / 35.0);
        q_Lx = q * Ly / 2;
        q_Ly = q * Lx / 2;
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

    void tinh_thepsan() {
        double tyLe = Lx / Ly;
        double alpha;

        if (tyLe >= 0.95) alpha = 0.045;
        else if (tyLe >= 0.75) alpha = 0.050;
        else alpha = 0.060;

        double M = alpha * q_san * Lx * Lx;
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
        cout << "🔩 Đề xuất thép sàn:\n";
        cout << "   • ∅" << duongKinh << " mm, cách nhau " << khoangCach << " mm\n";
        cout << "   • As cung cấp ≈ " << AsCungCap << " mm²/m\n";
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
        cout << "📌 Tổng tải trọng lên cột A: " << RA << " kN\n";
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

                        cout << "\n✅ Phương án hợp lệ:\n";
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
    double Lx = 8, Ly = 6.0;

    VatLieu vatlieu;
    vatlieu.nhapVatLieu();
    vatlieu.hienThi();

    San san(vatlieu, Lx, Ly);
    san.tinh_thepsan();
    san.hienThi();

    Cot cotA(vatlieu);
    cotA.tinh_RA(Lx, san.q_Lx);
    cotA.tinh_RA(Ly, san.q_Ly);
    cotA.hienThi();
    cotA.chon_tiet_dien();

    return 0;
}
