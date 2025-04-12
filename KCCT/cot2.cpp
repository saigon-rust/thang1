#include <iostream>
#include <vector>
#include <iomanip>

class Cot {
private:
    double chieuRong;     // b (mm)
    double chieuCao;      // h (mm)
    double chieuCaoCot;   // chiều cao cột (mm)
    double Ra;            // cường độ chịu nén bê tông (daN/cm2)
    double taiTrong;      // lực nén tác dụng lên cột (kN)
    double phi;           // hệ số giảm (0.9 - 1.0)

public:
    Cot(double b, double h, double H, double Ra, double phi = 1.0)
        : chieuRong(b), chieuCao(h), chieuCaoCot(H), Ra(Ra), phi(phi), taiTrong(0) {}

    void ganTaiTrong(double tai) {
        taiTrong = tai; // đơn vị: kN
    }

    double tinhDienTich() const {
        return (chieuRong / 10.0) * (chieuCao / 10.0); // cm²
    }

    double tinhSucChiuTai() const {
        return phi * Ra * tinhDienTich(); // daN
    }

    bool kiemTraChiuLuc() const {
        return (taiTrong * 100.0) <= tinhSucChiuTai(); // đổi kN → daN
    }

    void hienThiThongTin(int stt) const {
        std::cout << "Cột #" << stt << ":\n";
        std::cout << " - Tiết diện: " << chieuRong << " x " << chieuCao << " (mm)\n";
        std::cout << " - Chiều cao: " << chieuCaoCot << " mm\n";
        std::cout << " - Tải trọng truyền: " << taiTrong << " kN\n";
        std::cout << " - Sức chịu tải: " << tinhSucChiuTai() / 100.0 << " kN\n";
        std::cout << " - Trạng thái: " << (kiemTraChiuLuc() ? "✅ Đạt" : "❌ Không đạt") << "\n\n";
    }
};
int main() {
    double tongTaiTrong = 900.0; // tổng tải trọng từ dầm (kN)
    int soCot = 3;

    // Tạo danh sách các cột
    std::vector<Cot> dsCot = {
        Cot(300, 300, 3500, 160), // b, h, H, Ra (ví dụ: M200 ~ 160 daN/cm²)
        Cot(250, 400, 3500, 160),
        Cot(300, 350, 3500, 160)
    };

    // Chia tải trọng theo diện tích tiết diện
    double tongDienTich = 0.0;
    for (const auto& cot : dsCot)
        tongDienTich += cot.tinhDienTich();

    for (auto& cot : dsCot) {
        double phanTai = tongTaiTrong * (cot.tinhDienTich() / tongDienTich);
        cot.ganTaiTrong(phanTai);
    }

    // Hiển thị thông tin từng cột
    for (int i = 0; i < dsCot.size(); ++i)
        dsCot[i].hienThiThongTin(i + 1);

    return 0;
}
