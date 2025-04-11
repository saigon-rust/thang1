#include <iostream>
#include <string>
#include <cmath>

class W_ {
public:
    double h;
    double b;
    double W;

    W_(double h_in, double b_in)
        : h(h_in), b(b_in), W((b_in * pow(h_in, 2)) / 6) {}

    void printValues() const {
        std::cout << "h = " << h << std::endl;
        std::cout << "b = " << b << std::endl;
        std::cout << "W = " << W << std::endl;
    }
};

double W_value(double h, double b) {
    W_ w(h, b);
    w.printValues();
    return w.W;
}

int main() {
    double h, b;
    std::cout << "Nhap vao chieu cao h (mm): ";
    std::cin >> h;

    std::cout << "Nhap vao chieu rong b (mm): ";
    std::cin >> b;

    double result = W_value(h, b);

    std::cout << "Gia tri W tra ve: " << result << std::endl;

    return 0;
}
