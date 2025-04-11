#include <iostream>
#include <string>

class As {
public:
    double Rb;
    double Rt;
    double b;
    double h;
    double As;

    Mcp(double Rb_in, double Rt_in, double b_in, double h_in)
        : Rb(Rb_in), Rt(Rt_in), b(b_in), h(h_in), As(Rb_in*b_in*h_in/Rt_in) {}
};

double As_value(double Rb, double Rt, double b, double h) {
    As as(R, W);
    return as.As;
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
