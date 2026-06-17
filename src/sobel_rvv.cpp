#include "image.h"
#include <cstdint>
#include <cmath>

#ifdef __riscv_vector
#include <riscv_vector.h>
#endif

void applySobelRVV(const Image& input, Image& mag, Image& dir) {
    const int w = input.width;
    const int h = input.height;

    mag.allocate(w, h);
    dir.allocate(w, h);

    uint8_t* dst_mag = mag.data;
    uint8_t* dst_dir = dir.data;

#ifdef __riscv_vector
    const uint8_t* src = input.data;
    
    for (int y = 1; y < h - 1; ++y) {
        const uint8_t* above = src + (y - 1) * w;
        const uint8_t* curr  = src +  y      * w;
        const uint8_t* below = src + (y + 1) * w;

        int x = 1;
        while (x < w - 1) {
            // 🔥 هنا السر: استخدام m2 بدلاً من m1 لمضاعفة طول الفيكتور وتقليل عدد اللفات
            size_t vl = __riscv_vsetvl_e16m2((w - 1) - x);

            // ── 1. Load (m1) & Widen to 16-bit (m2) ──────────────────────────
            vint16m2_t va_left  = __riscv_vreinterpret_v_u16m2_i16m2(__riscv_vzext_vf2_u16m2(__riscv_vle8_v_u8m1(above + x - 1, vl), vl));
            vint16m2_t va_mid   = __riscv_vreinterpret_v_u16m2_i16m2(__riscv_vzext_vf2_u16m2(__riscv_vle8_v_u8m1(above + x,     vl), vl));
            vint16m2_t va_right = __riscv_vreinterpret_v_u16m2_i16m2(__riscv_vzext_vf2_u16m2(__riscv_vle8_v_u8m1(above + x + 1, vl), vl));

            vint16m2_t vc_left  = __riscv_vreinterpret_v_u16m2_i16m2(__riscv_vzext_vf2_u16m2(__riscv_vle8_v_u8m1(curr  + x - 1, vl), vl));
            vint16m2_t vc_right = __riscv_vreinterpret_v_u16m2_i16m2(__riscv_vzext_vf2_u16m2(__riscv_vle8_v_u8m1(curr  + x + 1, vl), vl));

            vint16m2_t vb_left  = __riscv_vreinterpret_v_u16m2_i16m2(__riscv_vzext_vf2_u16m2(__riscv_vle8_v_u8m1(below + x - 1, vl), vl));
            vint16m2_t vb_mid   = __riscv_vreinterpret_v_u16m2_i16m2(__riscv_vzext_vf2_u16m2(__riscv_vle8_v_u8m1(below + x,     vl), vl));
            vint16m2_t vb_right = __riscv_vreinterpret_v_u16m2_i16m2(__riscv_vzext_vf2_u16m2(__riscv_vle8_v_u8m1(below + x + 1, vl), vl));

            // ── 2. Compute Gx & Gy (m2) ──────────────────────────────────────
            vint16m2_t gx = __riscv_vadd_vv_i16m2(__riscv_vsub_vv_i16m2(va_right, va_left, vl), __riscv_vsub_vv_i16m2(vb_right, vb_left, vl), vl);
            gx = __riscv_vadd_vv_i16m2(gx, __riscv_vsll_vx_i16m2(__riscv_vsub_vv_i16m2(vc_right, vc_left, vl), 1, vl), vl);

            vint16m2_t gy = __riscv_vadd_vv_i16m2(__riscv_vsub_vv_i16m2(vb_left, va_left, vl), __riscv_vsub_vv_i16m2(vb_right, va_right, vl), vl);
            gy = __riscv_vadd_vv_i16m2(gy, __riscv_vsll_vx_i16m2(__riscv_vsub_vv_i16m2(vb_mid, va_mid, vl), 1, vl), vl);

            // ── 3. Magnitude Calculation ─────────────────────────────────────
            vint16m2_t abs_gx = __riscv_vmax_vv_i16m2(gx, __riscv_vneg_v_i16m2(gx, vl), vl);
            vint16m2_t abs_gy = __riscv_vmax_vv_i16m2(gy, __riscv_vneg_v_i16m2(gy, vl), vl);
            vint16m2_t vmag = __riscv_vmin_vv_i16m2(__riscv_vadd_vv_i16m2(abs_gx, abs_gy, vl), __riscv_vmv_v_x_i16m2(255, vl), vl);

            vuint8m1_t vmag8 = __riscv_vncvt_x_x_w_u8m1(__riscv_vreinterpret_v_i16m2_u16m2(vmag), vl);
            __riscv_vse8_v_u8m1(dst_mag + y * w + x, vmag8, vl);

            // ── 4. Direction Calculation (Masks for m2 are vbool8_t) ─────────
            vint16m2_t abs_gx_2 = __riscv_vsll_vx_i16m2(abs_gx, 1, vl);
            vint16m2_t abs_gy_2 = __riscv_vsll_vx_i16m2(abs_gy, 1, vl);

            vbool8_t mask_0  = __riscv_vmslt_vv_i16m2_b8(abs_gy_2, abs_gx, vl); 
            vbool8_t mask_90 = __riscv_vmslt_vv_i16m2_b8(abs_gx_2, abs_gy, vl); 

            vbool8_t gx_neg = __riscv_vmslt_vx_i16m2_b8(gx, 0, vl);
            vbool8_t gy_neg = __riscv_vmslt_vx_i16m2_b8(gy, 0, vl);
            vbool8_t opp_sign = __riscv_vmxor_mm_b8(gx_neg, gy_neg, vl); 

            vint16m2_t vdir = __riscv_vmv_v_x_i16m2(45, vl); 
            vdir = __riscv_vmerge_vxm_i16m2(vdir, 135, opp_sign, vl); 
            vdir = __riscv_vmerge_vxm_i16m2(vdir, 0, mask_0, vl);     
            vdir = __riscv_vmerge_vxm_i16m2(vdir, 90, mask_90, vl);   

            vuint8m1_t vdir8 = __riscv_vncvt_x_x_w_u8m1(__riscv_vreinterpret_v_i16m2_u16m2(vdir), vl);
            __riscv_vse8_v_u8m1(dst_dir + y * w + x, vdir8, vl);

            x += vl;
        }
    }
#else
    // Scalar fallback remains the same...
    for (int y = 1; y < h - 1; ++y) {
        for (int x = 1; x < w - 1; ++x) {
            int gx = -1*input.data[(y-1)*w+(x-1)] +1*input.data[(y-1)*w+(x+1)]
                     -2*input.data[ y   *w+(x-1)] +2*input.data[ y   *w+(x+1)]
                     -1*input.data[(y+1)*w+(x-1)] +1*input.data[(y+1)*w+(x+1)];
            int gy = -1*input.data[(y-1)*w+(x-1)] -2*input.data[(y-1)*w+ x   ]
                     -1*input.data[(y-1)*w+(x+1)]
                     +1*input.data[(y+1)*w+(x-1)] +2*input.data[(y+1)*w+ x   ]
                     +1*input.data[(y+1)*w+(x+1)];

            int magnitude = std::abs(gx) + std::abs(gy);  
            if (magnitude > 255) magnitude = 255;
            dst_mag[y * w + x] = static_cast<uint8_t>(magnitude);

            int abs_gx = std::abs(gx);
            int abs_gy = std::abs(gy);
            unsigned char direction = 45;

            if ((abs_gy << 1) < abs_gx)       direction = 0;
            else if ((abs_gx << 1) < abs_gy)  direction = 90;
            else if ((gx < 0) ^ (gy < 0))     direction = 135;

            dst_dir[y * w + x] = direction;
        }
    }
#endif
}
