#include <iostream>
#include <string>

class Mcp {
public:
    double R;
    double W;
    double M_cp;

    Mcp(double R_in, double W_in)
        : R(R_in), W(W_in), M_cp(R_in * W_in) {}

    void printValues() const {
        std::cout << "R = " << R << std::endl;
        std::cout << "W = " << W << std::endl;
        std::cout << "M_cp = " << M_cp << std::endl;
    }
};

double M_cp_value(double R, double W) {
    Mcp mcp(R, W);
    mcp.printValues();
    return mcp.M_cp;
}

int main() {
    double R, W;
    std::cout << "Nhap vao R: ";
    std::cin >> R;

    std::cout << "Nhap vao W: ";
    std::cin >> W;

    double result = M_cp_value(R, W);

    std::cout << "Gia tri M_cp tra ve: " << result << std::endl;

    return 0;
}
