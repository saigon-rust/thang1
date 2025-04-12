#include <iostream>
#include <cmath>
#include <iomanip>

class Column {
    private:
        double Ntt;      // Tải trọng tính toán (kN)
        double Rb;       // Cường độ chịu nén bê tông (MPa)
        double phi;      // Hệ số an toàn (thường lấy 0.8)
        double b, h;     // Kích thước cột (mm)
        double l0;       // Chiều dài tính toán của cột (mm)

    public:
        // Constructor
        Column(double Ntt_, double Rb_, double phi_, double l0_)
            : Ntt(Ntt_), Rb(Rb_), phi(phi_), l0(l0_), b(0), h(0) {}

        // Hàm tính diện tích tiết diện cần thiết
        double requiredArea() const {
            return (Ntt * 1000) / (Rb * phi);  // Đổi kN -> N, MPa = N/mm2
        }

        // Hàm chọn tiết diện phù hợp
        void chooseSection() {
            double Ac = requiredArea();
            double side = sqrt(Ac); // Ước lượng tiết diện vuông trước

            // Làm tròn lên bội số của 50 mm
            b = std::ceil(side / 50) * 50;
            h = b;

            // Nếu muốn ưu tiên chữ nhật, có thể sửa tỉ lệ b/h ở đây
        }

        // Kiểm tra tỉ lệ b/h
        bool checkAspectRatio() const {
            double ratio = h / b;
            return ratio >= 1.0 && ratio <= 2.0;
        }

        // Kiểm tra mảnh cột: λ = l0 / i ≤ 70
        bool checkSlenderness() const {
            double I = (b * pow(h, 3)) / 12.0;
            double Ac = b * h;
            double i = sqrt(I / Ac);
            double lambda = l0 / i;
            return lambda <= 70.0;
        }

        // In kết quả
        void printReport() const {
            std::cout << std::fixed << std::setprecision(2);
            std::cout << "==== KET QUA TINH TOAN COT ====\n";
            std::cout << "Tiet dien chon: b = " << b << " mm, h = " << h << " mm\n";
            std::cout << "Dien tich can thiet: " << requiredArea() << " mm^2\n";
            std::cout << "Kiem tra ti le b/h: " << (checkAspectRatio() ? "Dat" : "Khong dat") << "\n";
            std::cout << "Kiem tra manh cot (lambda): " << (checkSlenderness() ? "Dat" : "Khong dat") << "\n";
        }
    };

int main() {
    double Ntt = 450.0;     // kN
    double Rb = 11.5;       // MPa (B20)
    double phi = 0.8;       // Hệ số an toàn
    double l0 = 3000.0;     // mm (chiều cao tầng)

    Column cot(Ntt, Rb, phi, l0);
    cot.chooseSection();
    cot.printReport();

    return 0;
}

