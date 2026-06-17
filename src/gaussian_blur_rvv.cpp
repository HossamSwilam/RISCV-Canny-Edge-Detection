#include "image.h"
#include <riscv_vector.h>

void gaussian_blur_rvv(const Image& src, Image& dst) {
    int width = src.width;
    int height = src.height;

    // مصفوفة الفلتر 5x5 كأعداد صحيحة (مجموعها = 256)
    const int kernel[5][5] = {
        {1,  4,  6,  4, 1},
        {4, 16, 24, 16, 4},
        {6, 24, 36, 24, 6},
        {4, 16, 24, 16, 4},
        {1,  4,  6,  4, 1}
    };

    // الـ Loop الخارجية للصفوف (بنبعد عن الحواف 2 بكسل عشان الـ 5x5)
    for (int r = 2; r < height - 2; ++r) {
        int c = 2;
        int remain = width - 4; // عدد البكسلات اللي هنعالجها في السطر ده

        // الـ Loop الداخلية باستخدام VLA (Vector Length Agnostic)
        while (remain > 0) {
           
            size_t vl = __riscv_vsetvl_e8m1(remain);

      
            vuint16m2_t sum_vec = __riscv_vmv_v_x_u16m2(0, vl);

            // نطبق الفلتر 5x5 على البكسلات
            for (int kr = -2; kr <= 2; ++kr) {
                for (int kc = -2; kc <= 2; ++kc) {
                    uint16_t weight = kernel[kr + 2][kc + 2];
                    
                    // حساب مكان البداية اللي هنسحب منه البكسلات
                    const uint8_t* ptr = src.data + (r + kr) * width + (c + kc);

                    
                    vuint8m1_t p8 = __riscv_vle8_v_u8m1(ptr, vl);

                  
                    vuint16m2_t p16 = __riscv_vwcvtu_x_x_v_u16m2(p8, vl);

                 
                    sum_vec = __riscv_vmacc_vx_u16m2(sum_vec, weight, p16, vl);
                }
            }

            sum_vec = __riscv_vsrl_vx_u16m2(sum_vec, 8, vl);

          
            vuint8m1_t res8 = __riscv_vncvt_x_x_w_u8m1(sum_vec, vl);

          
            uint8_t* dst_ptr = dst.data + r * width + c;
            __riscv_vse8_v_u8m1(dst_ptr, res8, vl);

            // تحديث العدادات للفة اللي بعدها
            c += vl;
            remain -= vl;
        }
    }
}
