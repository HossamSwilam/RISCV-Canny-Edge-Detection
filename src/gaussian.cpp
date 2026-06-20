#include <cstdint>
#include <cstring>
#include <algorithm>
#include "processing.h"

// 5x5 Gaussian kernel coefficients (integer approximation, sigma ≈ 1.0).
// Sum = 273.  Using integers avoids any floating-point in the hot path.
static const int16_t GAUSS_KERNEL_1D[25] = {
     1,  4,  7,  4,  1,
     4, 16, 26, 16,  4,
     7, 26, 41, 26,  7,
     4, 16, 26, 16,  4,
     1,  4,  7,  4,  1
};
static const int GAUSS_KERNEL_SUM = 273;

// ---------------------------------------------------------------------------
// applyGaussianBlur
//
// Two-region strategy for performance:
//   Interior pixels (2 <= x < W-2, 2 <= y < H-2): no boundary check needed.
//   Border pixels (the outer 2-pixel ring): use convolve2D which zero-pads.
//
// The interior loop is branch-free, which lets the compiler auto-vectorise it
// and makes it a much better candidate for the RVV intrinsic version later.
// ---------------------------------------------------------------------------
Image applyGaussianBlur(const Image& input) {
    const int W = input.width;
    const int H = input.height;

    Image output;
    output.allocate(W, H);
    std::memset(output.data, 0, W * H);   // zero-initialise border pixels

    // --- Interior: branch-free, fully unrolled 5x5 convolution ---
    for (int y = 2; y < H - 2; ++y) {
        for (int x = 2; x < W - 2; ++x) {
            int32_t sum = 0;
            for (int ky = -2; ky <= 2; ++ky) {
                for (int kx = -2; kx <= 2; ++kx) {
                    sum += static_cast<int32_t>(input.data[(y + ky) * W + (x + kx)])
                         * static_cast<int32_t>(GAUSS_KERNEL_1D[(ky + 2) * 5 + (kx + 2)]);
                }
            }
            sum /= GAUSS_KERNEL_SUM;
            output.data[y * W + x] = static_cast<uint8_t>(
                std::max(0, std::min(255, static_cast<int>(sum))));
        }
    }

    // --- Border: use the generic convolve2D template with zero-padding ---
    // We allocate a temporary int32_t buffer for the full image, run
    // convolve2D on it, then copy only the border pixels into output.
    // This avoids duplicating the boundary logic here.
    int32_t* tmp = static_cast<int32_t*>(
        aligned_alloc(64, W * H * sizeof(int32_t)));

    convolve2D<uint8_t, int32_t, int16_t>(
        input.data, tmp, W, H, GAUSS_KERNEL_1D, 5);

    // Copy border rows (top 2, bottom 2)
    for (int y = 0; y < 2; ++y)
        for (int x = 0; x < W; ++x)
            output.data[y * W + x] = static_cast<uint8_t>(
                std::max(0, std::min(255, tmp[y * W + x] / GAUSS_KERNEL_SUM)));

    for (int y = H - 2; y < H; ++y)
        for (int x = 0; x < W; ++x)
            output.data[y * W + x] = static_cast<uint8_t>(
                std::max(0, std::min(255, tmp[y * W + x] / GAUSS_KERNEL_SUM)));

    // Copy border columns (left 2, right 2) — interior rows only
    for (int y = 2; y < H - 2; ++y) {
        for (int x = 0; x < 2; ++x)
            output.data[y * W + x] = static_cast<uint8_t>(
                std::max(0, std::min(255, tmp[y * W + x] / GAUSS_KERNEL_SUM)));
        for (int x = W - 2; x < W; ++x)
            output.data[y * W + x] = static_cast<uint8_t>(
                std::max(0, std::min(255, tmp[y * W + x] / GAUSS_KERNEL_SUM)));
    }

    std::free(tmp);
    return output;
}
