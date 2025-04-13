#include <iostream>
using namespace std;

void tinhPhanLuc(double L, double q, double P, double a) {
    double RA = (q * L) / 2 + (P * (L - a)) / L;

    // Xuất kết quả
    cout << "Phan luc tai goi A (RA): " << RA << " kN" << endl;
}

int main() {
    // Nhập dữ liệu
    double L, q, P, a;
    cout << "Nhap chieu dai dam (m): ";
    cin >> L;

    cout << "Nhap tai phan bo deu q (kN/m): ";
    cin >> q;

    cout << "Nhap tai tap trung P (kN): ";
    cin >> P;

    cout << "Nhap vi tri dat P tinh tu goi A (m): ";
    cin >> a;

    // Gọi hàm tính
    tinhPhanLuc(L, q, P, a);

    return 0;
}
