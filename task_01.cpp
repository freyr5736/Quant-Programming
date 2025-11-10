#include <bits/stdc++.h>

bool check_circle(int x, int y, int r) {
    return ((x * x) + (y * y)) <= (r * r);
}

int main() {
    std::random_device rd_seed;
    std::mt19937 gen(rd_seed());
    std::uniform_real_distribution<> dis(1, 960);
    for (int i = 1; i <= 10; ++i) {
        int x = dis(gen);
        int y = dis(gen);
        int a = (check_circle(x, y, 480)) ? 1 : 0;
        std::cout << "x = " << x << " y = " << y << " circle = " << a
                  << std::endl;
    }
    return 0;
}