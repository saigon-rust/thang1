// ket_cau_cong_trinh.cpp
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <iomanip>

using namespace std;

class VatLieu {
public:
    string ten;
    double Rb;     // Cường độ chịu nén bê tông (MPa)
    double Rs;     // Cường độ chịu kéo thép (MPa)
    double gamma_b;
    double gamma_s;

    VatLieu(string t, double rb, double rs, double gb = 1.15, double gs = 1.5)
        : ten(t), Rb(rb), Rs(rs), gamma_b(gb), gamma_s(gs) {}
};

class TaiTrong {
public:
    double tinh_tai;       // kN/m2
    double hoat_tai;       // kN/m2
    double phan_bo;        // hệ số phân bố tải trọng

    TaiTrong(double tt, double ht, double pb)
        : tinh_tai(tt), hoat_tai(ht), phan_bo(pb) {}

    double tongTaiTrong() const {
        return (tinh_tai + hoat_tai) * phan_bo;
    }
};

class San {
public:
    double dai, rong, day;
    TaiTrong tai_trong;
    VatLieu vat_lieu;

    San(double d, double r, double h, TaiTrong tt, VatLieu vl)
        : dai(d), rong(r), day(h), tai_trong(tt), vat_lieu(vl) {}

    double tinhMomenUon() const {
        double q = tai_trong.tongTaiTrong();
        return q * dai * dai / 8; // Giả sử kê 2 đầu
    }

    double tinhAs() const {
        double M = tinhMomenUon();
        double h0 = day - 0.03; // chiều cao hữu hiệu
        return M * 1e6 / (vat_lieu.Rs * h0); // mm2/m
    }

    void xuatThongTin() const {
        cout << "\n=== SÀN ===" << endl;
        cout << "Momen uốn: " << tinhMomenUon() << " kNm" << endl;
        cout << "Thép yêu cầu As: " << tinhAs() << " mm2/m" << endl;
    }
};

class Dam {
public:
    double dai, be_rong, chieu_cao;
    double tai_trong_phan_bo;
    VatLieu vat_lieu;

    Dam(double d, double b, double h, double q, VatLieu vl)
        : dai(d), be_rong(b), chieu_cao(h), tai_trong_phan_bo(q), vat_lieu(vl) {}

    double tinhMomenUon() const {
        return tai_trong_phan_bo * dai * dai / 8;
    }

    double tinhW() const {
        return be_rong * chieu_cao * chieu_cao / 6.0 * 1e6; // mm3
    }

    double tinhMcp() const {
        return vat_lieu.Rb * tinhW() / 1e6; // kNm
    }

    double tinhAs() const {
        double M = tinhMomenUon();
        double h0 = chieu_cao - 0.03;
        return M * 1e6 / (vat_lieu.Rs * h0); // mm2
    }

    bool kiemTraAnToan() const {
        return tinhMomenUon() <= tinhMcp();
    }

    void xuatThongTin() const {
        cout << "\n=== DẦM ===" << endl;
        cout << "Momen uốn: " << tinhMomenUon() << " kNm" << endl;
        cout << "Momen cho phép Mcp: " << tinhMcp() << " kNm" << endl;
        cout << "Thép yêu cầu As: " << tinhAs() << " mm2" << endl;
        cout << "An toàn: " << (kiemTraAnToan() ? "Đạt" : "Không đạt") << endl;
    }
};

//-----------------------------------------
// Class Cot
//-----------------------------------------
class Cot {
public:
    double be_rong, chieu_cao;
    double luc_doc;
    VatLieu vat_lieu;

    Cot(double b, double h, double N, VatLieu vl)
        : be_rong(b), chieu_cao(h), luc_doc(N), vat_lieu(vl) {}

    double dienTichCot() const {
        return be_rong * chieu_cao * 1e6; // mm2
    }

    bool kiemTraChiuNen() const {
        return luc_doc * 1e3 <= dienTichCot() * vat_lieu.Rb; // N
    }

    void xuatThongTin() const {
        cout << "\n=== CỘT ===" << endl;
        cout << "Lực dọc: " << luc_doc << " kN" << endl;
        cout << "Diện tích tiết diện: " << dienTichCot() << " mm2" << endl;
        cout << "Chịu nén an toàn: " << (kiemTraChiuNen() ? "Đạt" : "Không đạt") << endl;
    }
};

//-----------------------------------------
// Class CongTrinh
//-----------------------------------------
class CongTrinh {
public:
    vector<San> dsSan;
    vector<Dam> dsDam;
    vector<Cot> dsCot;

    void tinhToanVaXuatBaoCao() {
        cout << fixed << setprecision(2);

        for (const auto& san : dsSan) san.xuatThongTin();
        for (const auto& dam : dsDam) dam.xuatThongTin();
        for (const auto& cot : dsCot) cot.xuatThongTin();
    }
};

//-----------------------------------------
// Hàm main
//-----------------------------------------
int main() {
    VatLieu beTong("B20", 11.5, 280);
    TaiTrong taiSan(1.0, 2.0, 1.1);

    San san1(4.0, 3.0, 0.12, taiSan, beTong);
    Dam dam1(4.0, 0.2, 0.4, 5.0, beTong);
    Cot cot1(0.3, 0.3, 250.0, beTong);

    CongTrinh congTrinh;
    congTrinh.dsSan.push_back(san1);
    congTrinh.dsDam.push_back(dam1);
    congTrinh.dsCot.push_back(cot1);

    congTrinh.tinhToanVaXuatBaoCao();

    return 0;
}
