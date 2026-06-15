#include <cstdint>
#include "processing.h"
#include <algorithm>
#include <cstring>

Image applyGaussianBlur(const Image& input) {
    int W = input.width;
    int H = input.height;

    Image output;
    output.allocate(W, H); // حجز ذاكرة Aligned جديدة

    const int kernel[5][5] = {
        {1, 4, 7, 4, 1},
        {4,16,26,16,4},
        {7,26,41,26,7},
        {4,16,26,16,4},
        {1, 4, 7, 4, 1}
    };
    const int KERNEL_SUM = 273;

    auto idx = [&](int x, int y) {
        return y * W + x;
    };

    // 1. معالجة "قلب الصورة" (بدون أي شروط if - سريع جداً ومثالي للـ Pipeline)
    for (int y = 2; y < H - 2; y++) {
        for (int x = 2; x < W - 2; x++) {
            int sum = 0;
            for (int ky = -2; ky <= 2; ky++) {
                for (int kx = -2; kx <= 2; kx++) {
                    sum += input.data[idx(x + kx, y + ky)] * kernel[ky + 2][kx + 2];
                }
            }
            sum /= KERNEL_SUM;
            output.data[idx(x, y)] = (uint8_t)std::max(0, std::min(255, sum));
        }
    }

    // دالة مساعدة لمعالجة الحواف الآمنة
    auto processPixelSafe = [&](int x, int y) {
        int sum = 0;
        for (int ky = -2; ky <= 2; ky++) {
            for (int kx = -2; kx <= 2; kx++) {
                int nx = x + kx;
                int ny = y + ky;
                int pixel = 0; // Zero padding
                if (nx >= 0 && nx < W && ny >= 0 && ny < H) {
                    pixel = input.data[idx(nx, ny)];
                }
                sum += pixel * kernel[ky + 2][kx + 2];
            }
        }
        sum /= KERNEL_SUM;
        output.data[idx(x, y)] = (uint8_t)std::max(0, std::min(255, sum));
    };

    // 2. معالجة الحواف
    for (int y = 0; y < 2; y++) {
        for (int x = 0; x < W; x++) processPixelSafe(x, y);
    }
    for (int y = H - 2; y < H; y++) {
        for (int x = 0; x < W; x++) processPixelSafe(x, y);
    }
    for (int y = 2; y < H - 2; y++) {
        for (int x = 0; x < 2; x++) processPixelSafe(x, y);
    }
    for (int y = 2; y < H - 2; y++) {
        for (int x = W - 2; x < W; x++) processPixelSafe(x, y);
    }

    return output;
}
