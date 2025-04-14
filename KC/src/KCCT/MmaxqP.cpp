#include <iostream>
using namespace std;

// Hàm tính Moment uốn lớn nhất
double tinhMmax(double L, double q, double P, double a) {
    double b = L - a;
    double Mmax = (q * L * L) / 8 + (P * a * b) / L;
    return Mmax;
}

// Chương trình chính để test hàm
int main() {
    double L, q, P, a;

    cout << "Nhap chieu dai dam L (m): ";
    cin >> L;

    cout << "Nhap tai phan bo q (kN/m): ";
    cin >> q;

    cout << "Nhap tai tap trung P (kN): ";
    cin >> P;

    cout << "Nhap vi tri tai P tinh tu goi A (m): ";
    cin >> a;

    double Mmax = tinhMmax(L, q, P, a);
    cout << "Moment uon lon nhat Mmax = " << Mmax << " kNm" << endl;

    return 0;
}
