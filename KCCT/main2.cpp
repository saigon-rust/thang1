namespace san {
    class San {
        double tai_san_len_dam(double q, double Lx, double Ly) {
            // Tổng tải trọng toàn sàn
            double Q = q * Lx * Ly;

            // Tải lên mỗi dầm ngắn (theo phương Ly)
            double q_ngan = q * Lx / 2.0;

            // Tải lên mỗi dầm dài (theo phương Lx)
            double q_dai = q * Ly / 2.0;

            std::cout << "\n🔸 Tổng tải trọng toàn sàn: Q = " << Q << " kN\n";
            std::cout << "🔸 Tải lên mỗi dầm NGẮN (q_ngan): " << q_ngan << " kN/m\n";
            std::cout << "🔸 Tải lên mỗi dầm DÀI  (q_dai):  " << q_dai << " kN/m\n";
            return Q, q_ngan, q_dai
        }
    }
}