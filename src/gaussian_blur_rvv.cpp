#include "processing.h"
#include <cstdint>
#include <cmath>

#ifdef __riscv_vector
#include <riscv_vector.h>
#endif

void gaussian_blur_rvv(const Image& src, Image& dst) {
    int width  = src.width;
    int height = src.height;

    dst.allocate(width, height);

    const int kernel[5][5] = {
        {1,  4,  6,  4, 1},
        {4, 16, 24, 16, 4},
        {6, 24, 36, 24, 6},
        {4, 16, 24, 16, 4},
        {1,  4,  6,  4, 1}
    };

#ifdef __riscv_vector

    for (int r = 2; r < height - 2; ++r) {
        int c = 2;
        int remain = width - 4;

        while (remain > 0) {
            size_t vl = __riscv_vsetvl_e8m1(remain);
            vuint16m2_t sum_vec = __riscv_vmv_v_x_u16m2(0, vl);

            for (int kr = -2; kr <= 2; ++kr) {
                for (int kc = -2; kc <= 2; ++kc) {
                    uint16_t weight = kernel[kr + 2][kc + 2];
                    const uint8_t* ptr = src.data + (r + kr) * width + (c + kc);
                    vuint8m1_t p8   = __riscv_vle8_v_u8m1(ptr, vl);
                    vuint16m2_t p16 = __riscv_vwcvtu_x_x_v_u16m2(p8, vl);
                    sum_vec = __riscv_vmacc_vx_u16m2(sum_vec, weight, p16, vl);
                }
            }

            sum_vec = __riscv_vsrl_vx_u16m2(sum_vec, 8, vl);
            vuint8m1_t res8  = __riscv_vncvt_x_x_w_u8m1(sum_vec, vl);
            uint8_t* dst_ptr = dst.data + r * width + c;
            __riscv_vse8_v_u8m1(dst_ptr, res8, vl);

            c      += vl;
            remain -= vl;
        }
    }

#else
    // Scalar fallback for host machine (make test)
    // Simple 5x5 convolution with zero-padding
    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            int sum = 0;
            int weight_sum = 0;
            for (int kr = -2; kr <= 2; ++kr) {
                for (int kc = -2; kc <= 2; ++kc) {
                    int row = r + kr;
                    int col = c + kc;
                    if (row >= 0 && row < height && col >= 0 && col < width) {
                        int w = kernel[kr + 2][kc + 2];
                        sum        += w * src.data[row * width + col];
                        weight_sum += w;
                    }
                }
            }
            dst.data[r * width + c] = static_cast<uint8_t>(sum / weight_sum);
        }
    }
#endif
}