#include <iostream>
#include <cmath>
#include <iomanip>

using namespace std;

// Hàm tính Mmax theo loại dầm
double tinh_Mmax(int loaiDam, double q, double P, double L) {
    switch (loaiDam) {
        case 1: // Dầm đơn giản – tải đều
            return (q * pow(L, 2)) / 8.0;
        case 2: // Dầm đơn giản – tải tập trung P giữa nhịp
            return (P * L) / 4.0;
        case 3: // Dầm 2 nhịp liên tục – tải đều (ước lượng tại gối giữa)
            return (q * pow(L, 2)) / 10.0;
        case 4: // Dầm công xôn – tải đều
            return (q * pow(L, 2)) / 2.0;
        case 5: // Dầm công xôn – tải tập trung P
            return P * L;
        default:
            cout << "Lựa chọn không hợp lệ.\n";
            return 0;
    }
}

int main() {
    int loaiDam;
    double q = 0, P = 0, L;

    cout << "Chọn loại dầm:\n";
    cout << "1. Dầm đơn giản – tải đều\n";
    cout << "2. Dầm đơn giản – tải tập trung P giữa nhịp\n";
    cout << "3. Dầm 2 nhịp liên tục – tải đều\n";
    cout << "4. Dầm công xôn – tải đều\n";
    cout << "5. Dầm công xôn – tải tập trung P\n";
    cout << "Nhập lựa chọn (1–5): ";
    cin >> loaiDam;

    cout << "Nhập chiều dài L (m): ";
    cin >> L;

    if (loaiDam == 1 || loaiDam == 3 || loaiDam == 4) {
        cout << "Nhập tải đều q (kN/m): ";
        cin >> q;
    }

    if (loaiDam == 2 || loaiDam == 5) {
        cout << "Nhập tải tập trung P (kN): ";
        cin >> P;
    }

    double Mmax = tinh_Mmax(loaiDam, q, P, L);

    cout << fixed << setprecision(2);
    cout << "=> Momen uốn lớn nhất Mmax = " << Mmax << " kNm\n";

    return 0;
}
