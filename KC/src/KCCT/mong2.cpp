#include <iostream>
#include <cmath>
#include <string>
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

// Hàm tính diện tích móng, diện tích thép, và đề xuất cốt thép
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

    // 5. Tính moment uốn (M) tính theo công thức cho móng vuông
    double M = (q * a * a) / 16;  // Moment uốn của móng vuông

    // 6. Tính diện tích thép phân bố đều
    double R_s = 280; // Cường độ chịu kéo của thép (MPa), ví dụ thép SD390
    double z = h / 2; // Chiều sâu hiệu quả của móng (đối với móng vuông)
    double A_s = M * 1000 / (R_s * z); // Diện tích thép (mm²)

    // Kiểm tra nếu diện tích thép tính được hợp lý
    if (A_s <= 0) {
        cout << "Lỗi! Diện tích thép không hợp lý." << endl;
        return;
    }

    // Đề xuất đường kính thép và số lượng thép
    double phi = sqrt(A_s / 3.14);  // Đường kính thép (mm)
    double areaSteel = 3.14 * (phi / 2) * (phi / 2); // Diện tích một cây thép (mm²)
    int soLuongThep = ceil(A_s / areaSteel); // Số lượng thép cần thiết

    // In ra kết quả
    cout << "1. Diện tích móng A = " << A << " m²" << endl;
    cout << "2. Áp lực nền q = " << q << " kN/m²" << endl;
    cout << "3. Kích thước móng a = " << a << " m" << endl;
    cout << "4. Moment uốn M = " << M << " kNm" << endl;
    cout << "5. Diện tích thép A_s = " << A_s << " mm²" << endl;
    cout << "6. Đường kính thép đề xuất: Ø" << round(phi) << " mm" << endl;
    cout << "7. Số lượng thép cần thiết: " << soLuongThep << " cây" << endl;
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
