#include <iostream>
#include <string>
#include <unordered_map>
#include <cmath>

#include <vector>
#include <iomanip>

struct SteelBar {
    int phi;           // Đường kính
    double area;       // Diện tích 1 cây
};

void suggestSteel(double As_required) {
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

class W_ {
public:
    double h;
    double b;
    double W;

    W_(double h_in, double b_in)
        : h(h_in), b(b_in), W((b_in * pow(h_in, 2)) / 6) {}

    void printValues() const {
        std::cout << "W = " << W << " mm^3" << std::endl;
    }
};

double W_value(double h, double b) {
    W_ w(h, b);
    w.printValues();
    return w.W;
}

class R_ {
public:
    const char* vatLieu;
    double f;
    double l;
    double R;

    R_(const char* vatLieu_in, double f_in, double l_in)
        : vatLieu(vatLieu_in), f(f_in), l(l_in), R(f_in / l_in) {}

    void printValues() const {
        std::cout << "\n--- Thong so vat lieu ---\n";
        std::cout << "Cuong do: " << vatLieu << std::endl;
        std::cout << "R = " << R << " MPa" << std::endl;
    }
};

double Rb_value(const std::string& vatLieuInput) {
    const std::unordered_map<std::string, std::pair<double, double>> materialData = {
        {"M200", {11.5, 1.5}},
        {"M250", {14.5, 1.5}},
        {"M300", {17, 1.5}},
        {"M350", {20, 1.5}}
    };

    if (materialData.count(vatLieuInput)) {
        const auto& data = materialData.at(vatLieuInput);
        R_ myObject(vatLieuInput.c_str(), data.first, data.second);
        myObject.printValues();
        return myObject.R;
    } else {
        std::cout << "Vat lieu '" << vatLieuInput << "' khong duoc ho tro." << std::endl;
        return -1.0;
    }
}

double Rt_value(const std::string& vatLieuInput) {
    const std::unordered_map<std::string, std::pair<double, double>> materialData = {
        {"CB300", {300.0, 1.15}},
        {"CB400", {400.0, 1.15}}
    };

    if (materialData.count(vatLieuInput)) {
        const auto& data = materialData.at(vatLieuInput);
        R_ myObject(vatLieuInput.c_str(), data.first, data.second);
        myObject.printValues();
        return myObject.R;
    } else {
        std::cout << "Vat lieu '" << vatLieuInput << "' khong duoc ho tro." << std::endl;
        return -1.0;
    }
}

class M_cp {
public:
    double R;
    double W;
    double Mcp;

    M_cp(double R_in, double W_in)
        : R(R_in), W(W_in), Mcp(R_in * W_in) {}

    void printValues() const {
        std::cout << "\n--- Momen chiu uon cho phep ---\n";
        std::cout << "Mcp = R * W = " << Mcp << " N.mm" << std::endl;
    }
};

double Mcp_value(double R, double W) {
    M_cp mcp(R, W);
    mcp.printValues();
    return mcp.Mcp;
}

class As_ {
    public:
        double Rb;
        double Rt;
        double b;
        double h;
        double As;
    
        As_(double Rb_in, double Rt_in, double h_in, double b_in)
            : Rb(Rb_in), Rt(Rt_in), b(b_in), h(h_in), As(Rb_in * b_in * h_in / Rt_in) {}
    
        void printValues() const {
            std::cout << "\n--- Tinh tiet dien thep As ---\n";
            std::cout << "As = (Rb * b * h) / Rt = " << As << " mm^2" << std::endl;
        }
    };
    
double As_value_(double Rb, double Rt, double h, double b) {
    As_ as(Rb, Rt, h, b);
    as.printValues();
    return as.As;
}

#include <iostream>

class AsCalculator {
public:
    double Rt;   // Cường độ chịu kéo của thép (MPa)
    double M;    // Mô men uốn (kNm)
    double h0;   // Chiều cao làm việc (mm)
    double j;    // Hệ số đòn bẩy (thường ≈ 0.9)
    double As;   // Kết quả diện tích cốt thép (mm^2)

    AsCalculator(double M_in, double Rt_in, double h0_in, double j_in = 0.9)
        : M(M_in), Rt(Rt_in), h0(h0_in), j(j_in)
    {
        As = (M) / (Rt * j * h0);  // M đơn vị kNm → N.mm
    }

    void printValues() const {
        std::cout << "\n--- Tính diện tích thép As (gần đúng) ---\n";
        std::cout << "Công thức: As = (M * 10^6) / (Rs * j * h0)\n";
        std::cout << "M = " << M << " kNm\n";
        std::cout << "Rs = " << Rt << " MPa\n";
        std::cout << "j = " << j << "\n";
        std::cout << "h0 = " << h0 << " mm\n";
        std::cout << "==> As ≈ " << As << " mm^2\n";
    }
};

// Hàm ngoài để sử dụng nhanh
double As_value(double M, double Rs, double h0, double j = 0.9) {
    AsCalculator as(M, Rs, h0, j);
    as.printValues();
    return as.As;
}

class MomentCalculator {
    public:
        int loaiDam;
        double q; // tải phân bố (kN/m)
        double P; // tải tập trung (kN)
        double L; // chiều dài dầm (m)
    
        MomentCalculator() : loaiDam(0), q(0), P(0), L(0) {}
    
        void nhapDuLieu() {
            std::cout << "\nChọn loại dầm:\n";
            std::cout << "1. Dầm đơn giản – tải đều\n";
            std::cout << "2. Dầm đơn giản – tải tập trung P giữa nhịp\n";
            std::cout << "3. Dầm 2 nhịp liên tục – tải đều\n";
            std::cout << "4. Dầm công xôn – tải đều\n";
            std::cout << "5. Dầm công xôn – tải tập trung\n";
            std::cout << "Nhập lựa chọn (1–5): ";
            std::cin >> loaiDam;
    
            std::cout << "Nhập chiều dài dầm L (m): ";
            std::cin >> L;
    
            if (loaiDam == 1 || loaiDam == 3 || loaiDam == 4) {
                std::cout << "Nhập tải đều q (kN/m): ";
                std::cin >> q;
            }
    
            if (loaiDam == 2 || loaiDam == 5) {
                std::cout << "Nhập tải tập trung P (kN): ";
                std::cin >> P;
            }
    
            // Thêm tùy chọn tổ hợp tải trọng phụ (gió, phức hợp)
            char coGio;
            std::cout << "Có tính tải trọng gió hoặc tổ hợp phức hợp? (y/n): ";
            std::cin >> coGio;
            if (coGio == 'y' || coGio == 'Y') {
                double Mphu;
                std::cout << "Nhập mô men bổ sung (kNm): ";
                std::cin >> Mphu;
    
                // Chuyển thành tải tương đương nếu cần (ở đây cộng trực tiếp)
                if (loaiDam == 1 || loaiDam == 3 || loaiDam == 4) {
                    // Tính thêm q phụ từ Mphu
                    double qphu = (8 * Mphu) / pow(L, 2);
                    q += qphu;
                } else {
                    // Tính thêm P phụ từ Mphu
                    double Pphu = (4 * Mphu) / L;
                    P += Pphu;
                }
            }
        }
    
        double tinhMmax() const {
            switch (loaiDam) {
                case 1: return (q * pow(L, 2)) / 8.0;
                case 2: return (P * L) / 4.0;
                case 3: return (q * pow(L, 2)) / 10.0;
                case 4: return (q * pow(L, 2)) / 2.0;
                case 5: return P * L;
                default:
                    std::cout << "Loại dầm không hợp lệ.\n";
                    return 0.0;
            }
        }
    };
    

    
int main() {
    // 1. Vat lieu → R
    std::string Bt;
    std::cout << "Nhap vao BT: (M200, M250, M300, M350): ";
    std::cin >> Bt;
    double Rb = Rb_value(Bt);

    std::string Ct;
    std::cout << "Nhap vao CT: (CB300, CB400): ";
    std::cin >> Ct;
    double Rt = Rt_value(Ct);


    // 2. Kich thuoc → W
    double h, b;
    std::cout << "\nNhap vao chieu cao h (mm): ";
    std::cin >> h;
    std::cout << "Nhap vao chieu rong b (mm): ";
    std::cin >> b;
    double W = W_value(h, b);

    // 3. Tinh M_cp = R * W
    double Mcp = Mcp_value(Rb, W); // đơn vị N.mm

    // // 4. Nhap tai trong q và L để tính M_max
    // double q, L;
    // std::cout << "\nNhap vao tai trong phan bo q (kN/m): ";
    // std::cin >> q;
    // std::cout << "Nhap vao chieu dai L cua dam (m): ";
    // std::cin >> L;

    MomentCalculator mc;
    mc.nhapDuLieu();
    double Mmax = mc.tinhMmax() * 1e6;

    double As = As_value(Mmax, Rt, h-25);
    std::cout << "Tiết diện thép As =  " << As << std::endl;
    suggestSteel(As);


    // 5. Kiem tra dieu kien Mmax <= M_cp
    std::cout << "\n=== KET LUAN ===\n";
    if (Mmax <= Mcp) {
        std::cout << "=> DAM AN TOAN (Mmax = " << Mmax << " N.mm <= M_cp = " << Mcp << " N.mm)\n";
    } else {
        std::cout << "=> DAM KHONG AN TOAN (Mmax = " << Mmax << " N.mm > M_cp = " << Mcp << " N.mm)\n";
    }

    // 6. Tong hop thong tin
    std::cout << "\n=== TONG HOP ===\n";
    std::cout << "Vat lieu be tong: " << Bt << ", Rb = " << Rb << " MPa\n";
    std::cout << "Vat lieu thep: " << Ct << ", Rt = " << Rt << " MPa\n";
    std::cout << "Kich thuoc dam: b = " << b << " mm, h = " << h << " mm\n";
    std::cout << "Mcp = " << Mcp << " N.mm, Mmax = " << Mmax << " N.mm\n";
    std::cout << "As (thep) can dung: " << As << " mm^2\n";


    return 0;
}
