#include <iostream>
#include <algorithm>
#include <iomanip>
#include <vector>
#include <cmath>

using namespace std;

class VatLieu {
public:
    string loai_be_tong, loai_thep, loai_tuong;
    double R_b, R_s, Rs, Rb;
    double gamma;
    double dtuong = 0.23;
    double htuong = 3;
    double qtuong;

    VatLieu() : R_b(0), R_s(0), gamma(0), Rs(0), Rb(0), qtuong(0) {}

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

        Rs = R_s / 1.5;
        Rb = R_b / 1.15;
        qtuong = gamma * htuong * dtuong;
    }

    void hienThi() {
        cout << "\n✅ Đã nhập vật liệu:\n";
        cout << "   • Bê tông: " << loai_be_tong << " (Rb = " << R_b << " MPa)\n";
        cout << "   • Thép   : " << loai_thep << " (Rs = " << R_s << " MPa)\n";
        cout << "   • Tường  : " << loai_tuong << " (γ = " << gamma << " kN/m³)\n";
    }
};

class KichThuoc {
public:
    double Lx, Ly;
    KichThuoc(double Lx_in, double Ly_in) : Lx(Lx_in), Ly(Ly_in) {}

    void hienThi() {
        cout << "\n✅ Kích thước:\n";
        cout << "   • Lx = " << Lx << " m\n";
        cout << "   • Ly = " << Ly << " m\n";
    }
};

class San {
private:
    int duongKinh = 0;
    int soThanhTrenMet = 0;
    double khoangCach = 0;
    double AsCungCap = 0;

    double Lx, Ly;
    double q_san;
    double Rs, Rb;
public:
    double hsan;
    double q_Lx, q_Ly;
    double As = 0;

    San(const VatLieu& vl, double Lx_in, double Ly_in, double q_in = 7)
        : Lx(Lx_in), Ly(Ly_in), q_san(q_in),
        Rs(vl.Rs), Rb(vl.Rb),
        hsan(max(0.12, min(Lx_in, Ly_in) / 35.0)),
        q_Lx(q_in * Lx_in / 2),
        q_Ly(q_in * Ly_in / 2) {}

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

class Cot {
public:
    double RA = 0;
    double Rb;
    double Rs;

    Cot(const VatLieu& vl) : Rb(vl.Rb), Rs(vl.Rs) {}

    void tinh_RA(double L, double q, double P = 0, double a = 0) {
        RA += (q * L) / 2 + (P * (L - a)) / L;
    }

    void hienThi() const {
        cout << "📌 Tổng tải trọng tác dụng lên cột A: " << RA << " kN\n";
    }

    double tinh_As(double N, double Rsc) {
        return (N * 1e3) / Rsc;
    }

    bool kiem_tra_manh_cot(double b, double h, double l0) {
        double I = (b * pow(h, 3)) / 12.0;
        double Ac = b * h;
        double i = sqrt(I / Ac);
        double lambda = l0 / i;
        return lambda <= 70.0;
    }

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

    void chon_tiet_dien(double l0 = 3) {
        vector<int> kich_thuoc = {200, 250, 300, 350, 400};

        cout << "\n📊 Đang đánh giá các phương án cột...\n";

        for (int b : kich_thuoc) {
            for (int h : kich_thuoc) {
                double Ac = b * h;
                double Rc = 0.9 * Rb * Ac;
                double N_bt = Rc / 1e3;

                if (RA <= N_bt * 1.2) {
                    if (kiem_tra_manh_cot(b, h, l0)) {
                        double As = tinh_As(RA, Rs);
                        PhuongAnThep ph = tim_thep(As);

                        cout << "\n✅ Phương án hợp lệ:\n";
                        cout << "   - Tiết diện cột: " << b << " x " << h << " mm\n";
                        cout << "   - As yêu cầu: " << ceil(As) << " mm²\n";
                        cout << "   - Bố trí: " << ph.so_thanh << " thanh ∅" << ph.duong_kinh << " (As cung cấp: " << ph.As_cung_cap << " mm²)\n";
                        cout << "   - Kiểm tra mảnh cột: Đạt\n";
                        return;
                    }
                }
            }
        }
        cout << "\n⚠️ Không tìm được phương án tiết diện phù hợp!\n";
    }
};

int main() {
    double Lx = 4;
    double Ly = 2;

    double DS1 = 4;
    double DS2 = 2;

    VatLieu vatlieu;
    vatlieu.nhapVatLieu();
    vatlieu.hienThi();

    San san(vatlieu, Lx, Ly);
    san.tinh_thepsan();
    san.hienThi();

    Cot cotA(vatlieu);
    cotA.tinh_RA(DS1, san.q_Lx + vatlieu.qtuong/2);
    cotA.tinh_RA(DS1, san.q_Lx + vatlieu.qtuong/2);
    cotA.tinh_RA(DS1, san.q_Lx + vatlieu.qtuong/2);
    cotA.tinh_RA(DS2, san.q_Ly);
    cotA.hienThi();
    cotA.chon_tiet_dien();

    return 0;
}
