#include <iostream>
using namespace std;

double tinh_RA(double L, double q, double P, double a) {
    double RA = (q * L) / 2 + (P * (L - a)) / L;
    return RA;
}

int main() {

    double RA1 = tinh_RA(4, , 30, 2);
    double RA2 = tinh_RA(4, 30, 30, 2);
    double RA = RA1 + RA2;
    cout << "Phan luc tai goi A (RA): " << RA << " kN" << endl;
    return 0;
}
