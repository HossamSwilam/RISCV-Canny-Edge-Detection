#include <cstdint>
#include "processing.h"
#include <vector>
#include <algorithm>

Image applyGaussianBlur(const Image& input) {
    Image output = input;

    const int kernel[5][5] = {
        {1, 4, 7, 4, 1},
        {4,16,26,16,4},
        {7,26,41,26,7},
        {4,16,26,16,4},
        {1, 4, 7, 4, 1}
    };

    const int KERNEL_SUM = 273;

    int W = input.width;
    int H = input.height;

    auto idx = [&](int x, int y) {
        return y * W + x;
    };

    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {

            int sum = 0;

            for (int ky = -2; ky <= 2; ky++) {
                for (int kx = -2; kx <= 2; kx++) {

                    int nx = x + kx;
                    int ny = y + ky;

                    int pixel = 0; // zero padding

                    if (nx >= 0 && nx < W && ny >= 0 && ny < H) {
                        pixel = input.data[idx(nx, ny)];
                    }

                    sum += pixel * kernel[ky + 2][kx + 2];
                }
            }

            sum /= KERNEL_SUM;

            // clamp
            sum = std::max(0, std::min(255, sum));

            output.data[idx(x, y)] = (uint8_t)sum;
        }
    }

    return output;
}
