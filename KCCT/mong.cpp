#include <iostream>
#include <cmath>
#include <string>
#include <vector>
using namespace std;

// Hàm tính sức chịu tải của đất nền theo loại đất (dùng switch với các số 1, 2, 3, 4)
double getSucChiuTai(int loaiDat) {
    double R = 0.0;  // Giá trị mặc định nếu không tìm thấy loại đất hợp lệ
    switch (loaiDat) {
        case 1:  // Cát khô
            R = 150.0; // kN/m²
            break;
        case 2:  // Cát ẩm
            R = 100.0; // kN/m²
            break;
        case 3:  // Sét mềm
            R = 50.0; // kN/m²
            break;
        case 4:  // Sét cứng
            R = 200.0; // kN/m²
            break;
        case 5:  // Đá
            R = 500.0; // kN/m²
            break;
        default:
            cout << "Loại đất không hợp lệ!" << endl;
            return 0.0;
    }

    return R;
}

// Hàm đề xuất thép
struct PhuongAnThep {
    int duongKinh;
    int soLuongThep;
    double khoangCach;
    double AsCap;
};

PhuongAnThep phuongAnThep(double As_yeu_cau) {
    // Danh sách các đường kính thép tiêu chuẩn (mm)
    std::vector<int> danhSachDuongKinh = {10, 12, 14, 16, 18, 20};

    for (int d : danhSachDuongKinh) {
        double A1 = (M_PI * d * d) / 4.0; // Diện tích một thanh thép (mm²)
        int n = std::ceil(As_yeu_cau / A1); // Số lượng thép cần thiết
        double s = 1000.0 / n; // Khoảng cách giữa các thanh thép (mm)

        double AsCap = n * A1; // Tổng diện tích thép

        // Kiểm tra khoảng cách có hợp lệ hay không
        if (s >= 80 && s <= 250) {
            return {d, n, s, AsCap}; // Trả về phương án thép
        }
    }

    cout << "\n⚠️  Không tìm thấy phương án phù hợp với As = " << As_yeu_cau << " mm²/m.\n";
    return {0, 0, 0, 0}; // Trả về kết quả không hợp lệ
}

// Hàm tính diện tích móng, diện tích thép, và đề xuất cốt thép phân bố đều
void tinhMongDon(double N, int loaiDat, double gamma, double h) {
    // 1. Tính sức chịu tải của đất nền
    double R = getSucChiuTai(loaiDat);
    if (R == 0) return;  // Nếu loại đất không hợp lệ, kết thúc chương trình

    // 2. Tính diện tích móng A
    double A = N / R;  // Diện tích móng được tính theo tải trọng và sức chịu tải của đất
    
    // 3. Kiểm tra áp lực nền q
    double q = N / A;
    if (q > R) {
        cout << "Lỗi! Áp lực nền vượt quá sức chịu tải của đất." << endl;
        return;
    }

    // 4. Tính kích thước móng (giả sử móng vuông)
    double a = sqrt(A);  // Kích thước cạnh móng vuông

    // 5. Tính diện tích thép yêu cầu
    double R_s = 280; // Cường độ chịu kéo của thép (MPa), ví dụ thép SD390
    double A_s = N * 1000 / (R_s * gamma); // Diện tích thép cần thiết (mm²)

    // Kiểm tra nếu diện tích thép tính được hợp lý
    if (A_s <= 0) {
        cout << "Lỗi! Diện tích thép không hợp lý." << endl;
        return;
    }

    // 6. Đề xuất phương án thép
    PhuongAnThep pa = phuongAnThep(A_s); // Gọi hàm đề xuất thép

    // Kiểm tra kết quả từ phương án thép
    if (pa.duongKinh == 0) {
        return; // Nếu không có phương án phù hợp, kết thúc
    }

    // In ra kết quả
    cout << "1. Diện tích móng A = " << A << " m²" << endl;
    cout << "2. Áp lực nền q = " << q << " kN/m²" << endl;
    cout << "3. Kích thước móng a = " << a << " m" << endl;
    cout << "4. Diện tích thép A_s = " << A_s << " mm²" << endl;
    cout << "5. Đường kính thép đề xuất: Ø" << pa.duongKinh << " mm" << endl;
    cout << "6. Khoảng cách giữa các thanh thép: " << pa.khoangCach << " mm" << endl;
    cout << "7. Số lượng thép cần thiết: " << pa.soLuongThep << " cây" << endl;
}

int main() {
    // Nhập dữ liệu từ người dùng
    double N, gamma, h;
    int loaiDat;
    
    cout << "Nhập tải trọng N (kN): ";
    cin >> N;

    cout << "Nhập loại đất (1: Cát khô, 2: Cát ẩm, 3: Sét mềm, 4: Sét cứng, 5: Đá): ";
    cin >> loaiDat;

    cout << "Nhập hệ số an toàn gamma: ";
    cin >> gamma;

    cout << "Nhập chiều cao móng h (m): ";
    cin >> h;

    // Gọi hàm tính toán
    tinhMongDon(N, loaiDat, gamma, h);

    return 0;
}
