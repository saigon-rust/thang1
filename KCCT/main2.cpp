#include <iostream>

namespace san {

class San {
    public:
        // Hàm tính tổng tải trọng sàn (kN/m2)
        double tong_tai_san() {
            int loaiCT;
            double l_bt = 25.0; // kN/m3 – Trọng lượng riêng bê tông
            double g1, g2 = 2.0, qsd;
            
            double h_san;
            std::cout << "Nhập chiều dày sàn (m): ";
            std::cin >> h_san;

            g1 = l_bt * h_san; // Tải trọng bản sàn bê tông (kN/m2)

            std::cout << "\nChọn loại công trình:\n";
            std::cout << "1. Mái nhà không có người lên (1.0 kN/m²)\n";
            std::cout << "2. Nhà ở (2.0 kN/m²)\n";
            std::cout << "3. Văn phòng (3.0 kN/m²)\n";
            std::cout << "4. Hành lang công cộng (5.0 kN/m²)\n";
            std::cout << "Nhập lựa chọn (1-4): ";
            std::cin >> loaiCT;

            switch (loaiCT) {
                case 1: qsd = 1.0; break;
                case 2: qsd = 2.0; break;
                case 3: qsd = 3.0; break;
                case 4: qsd = 5.0; break;
                default:
                    std::cout << "Lựa chọn không hợp lệ. Mặc định chọn Nhà ở.\n";
                    qsd = 2.0;
            }

            double q = g1 + g2 + qsd;
            std::cout << "\n✅ Tổng tải trọng trên 1 m² sàn (q): " << q << " kN/m²\n";
            return q;
        }

        void tai_san_len_dam(double Lx, double Ly) {
            double q = tong_tai_san();

            double Q = q * Lx * Ly;                // Tổng tải toàn sàn (kN)
            double q_ngan = q * Lx / 2.0;          // Tải lên mỗi dầm ngắn (kN/m)
            double q_dai = q * Ly / 2.0;           // Tải lên mỗi dầm dài (kN/m)

            std::cout << "\n🔸 Tổng tải trọng toàn sàn: Q = " << Q << " kN\n";
            std::cout << "🔸 Tải lên mỗi m dầm NGẮN (q_ngan): " << q_ngan << " kN/m\n";
            std::cout << "🔸 Tải lên mỗi m dầm DÀI  (q_dai):  " << q_dai << " kN/m\n";
        }
    };

} // namespace san

int main() {
    san::San s;
    double Lx, Ly;

    std::cout << "Nhập chiều dài ô sàn Lx (m): ";
    std::cin >> Lx;
    std::cout << "Nhập chiều rộng ô sàn Ly (m): ";
    std::cin >> Ly;

    s.tai_san_len_dam(Lx, Ly);

    return 0;
}
