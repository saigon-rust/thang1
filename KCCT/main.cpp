#include <iostream>
using namespace std;

double tinh_RA(double L, double q, double P, double a) {
    return (q * L) / 2 + (P * (L - a)) / L;
}

double tinh_qs() {
    double q = 25*0.12 + 2 + 2;
}

double tinh_qt() {
    return 18*0.23*3;
}

int main() {

    double RA1 = tinh_RA(3, tinh_qs() + tinh_qt(), 0, 0);
    double RA2 = tinh_RA(6, tinh_qs() + tinh_qt(), 0, 0);
    double RA = RA1 + RA2;
    cout << "Phan luc tai goi A (RA): " << RA << " kN" << endl;

    double RB1 = tinh_RA(4, tinh_qs() + tinh_qt(), 0, 0);
    double RB2 = tinh_RA(4, tinh_qs() + tinh_qt(), 0, 0);
    double RB3 = tinh_RA(4, tinh_qs() + tinh_qt(), 0, 0);
    double RB = RB1 + RB2+ RB3;
    cout << "Phan luc tai goi B (RB): " << RB << " kN" << endl;

    double RC1 = tinh_RA(4, tinh_qs() + tinh_qt(), 0, 0);
    double RC2 = tinh_RA(2, tinh_qs() + tinh_qt(), 0, 0);
    double RC3 = tinh_RA(4, tinh_qs() + tinh_qt(), 0, 0);
    double RC = RC1 + RC2+ RC3;
    cout << "Phan luc tai goi C (RB): " << RC << " kN" << endl;

    double RD1 = tinh_RA(4, tinh_qs() + tinh_qt(), 0, 0);
    double RD2 = tinh_RA(2, tinh_qs() + tinh_qt(), 0, 0);
    double RD3 = tinh_RA(4, tinh_qs() + tinh_qt(), 0, 0);
    double RD4 = tinh_RA(4, tinh_qs() + tinh_qt(), 0, 0);
    double RD = RD1 + RD2+ RD3 + RD4;
    cout << "Phan luc tai goi D (RB): " << RD << " kN" << endl;

    return 0;
}
