#include <cstdint>
#include "processing.h"
#include <algorithm>

// =========================================================================
// 1. The Template Function (Requested by the professor)
// =========================================================================
template <typename PixelT = uint8_t, typename AccT = int32_t, typename KernelT = int16_t>
Image applyGaussianBlurTemplate(const Image& input) {

    int W = input.width;
    int H = input.height;

    Image output;
    output.allocate(W, H);

    const KernelT kernel[5][5] = {
        {1, 4, 7, 4, 1},
        {4,16,26,16,4},
        {7,26,41,26,7},
        {4,16,26,16,4},
        {1, 4, 7, 4, 1}
    };

    const AccT KERNEL_SUM = 273;

    auto idx = [&](int x, int y) {
        return y * W + x;
    };

    for (int y = 2; y < H - 2; y++) {
        for (int x = 2; x < W - 2; x++) {

            AccT sum = 0;

            for (int ky = -2; ky <= 2; ky++) {
                for (int kx = -2; kx <= 2; kx++) {
                    sum += static_cast<AccT>(input.data[idx(x + kx, y + ky)]) *
                           kernel[ky + 2][kx + 2];
                }
            }

            sum /= KERNEL_SUM;

            output.data[idx(x, y)] =
                static_cast<PixelT>(
                    std::max<AccT>(0, std::min<AccT>(255, sum))
                );
        }
    }

    auto processPixelSafe = [&](int x, int y) {

        AccT sum = 0;

        for (int ky = -2; ky <= 2; ky++) {
            for (int kx = -2; kx <= 2; kx++) {

                int nx = x + kx;
                int ny = y + ky;

                AccT pixel = 0;

                if (nx >= 0 && nx < W && ny >= 0 && ny < H) {
                    pixel = static_cast<AccT>(input.data[idx(nx, ny)]);
                }

                sum += pixel * kernel[ky + 2][kx + 2];
            }
        }

        sum /= KERNEL_SUM;

        output.data[idx(x, y)] =
            static_cast<PixelT>(
                std::max<AccT>(0, std::min<AccT>(255, sum))
            );
    };

    for (int y = 0; y < 2; y++)
        for (int x = 0; x < W; x++) processPixelSafe(x, y);

    for (int y = H - 2; y < H; y++)
        for (int x = 0; x < W; x++) processPixelSafe(x, y);

    for (int y = 2; y < H - 2; y++)
        for (int x = 0; x < 2; x++) processPixelSafe(x, y);

    for (int y = 2; y < H - 2; y++)
        for (int x = W - 2; x < W; x++) processPixelSafe(x, y);

    return output;
}

// =========================================================================
// 2. The Standard Function (Expected by processing.h and main.cpp)
// =========================================================================
Image applyGaussianBlur(const Image& input) {
    // Call the template version with the required types
    return applyGaussianBlurTemplate<uint8_t, int32_t, int16_t>(input);
}


// =========================================================================
// 3. SEPARABLE GAUSSIAN (FIXED ZERO PADDING)
// =========================================================================

typedef int32_t AccT_Sep;

static const int kernel1D[5] = {1, 4, 7, 4, 1};
static const int KERNEL_1D_SUM = 17;

inline int getIdx(int x, int y, int W) {
    return y * W + x;
}

void horizontalPass(const Image& input, Image& temp) {

    int W = input.width;
    int H = input.height;

    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {

            AccT_Sep sum = 0;

            for (int k = -2; k <= 2; k++) {

                int nx = x + k;

                AccT_Sep pixel = 0;

                if (nx >= 0 && nx < W) {
                    pixel = input.data[getIdx(nx, y, W)];
                }

                sum += pixel * kernel1D[k + 2];
            }

            temp.data[getIdx(x, y, W)] = sum / KERNEL_1D_SUM;
        }
    }
}

void verticalPass(const Image& temp, Image& output) {

    int W = temp.width;
    int H = temp.height;

    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {

            AccT_Sep sum = 0;

            for (int k = -2; k <= 2; k++) {

                int ny = y + k;

                AccT_Sep pixel = 0;

                if (ny >= 0 && ny < H) {
                    pixel = temp.data[getIdx(x, ny, W)];
                }

                sum += pixel * kernel1D[k + 2];
            }

            output.data[getIdx(x, y, W)] = sum / KERNEL_1D_SUM;
        }
    }
}

Image gaussianSeparable(const Image& input) {

    Image temp;
    temp.allocate(input.width, input.height);

    Image output;
    output.allocate(input.width, input.height);

    horizontalPass(input, temp);
    verticalPass(temp, output);

    temp.free_memory(); 

    return output;
}
