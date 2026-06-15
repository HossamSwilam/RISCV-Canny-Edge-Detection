#include "processing.h"
#include <cstdint>
#include <cmath>

#ifdef __riscv_vector
    const uint8_t* src = input.data.data();
#include <riscv_vector.h>
#endif

// ─────────────────────────────────────────────────────────────────────────────
// RVV Sobel Implementation
//
// Strategy:
//   - Outer loop over rows (y) is scalar — stays sequential
//   - Inner loop over columns (x) is strip-mined with RVV
//   - For each chunk of x positions, we load 8 pixel vectors from the 3
//     surrounding rows, combine them to get Gx and Gy vectors, compute
//     L1 magnitude, clamp to 255, and store.
//   - Direction stays scalar (small % of runtime, not worth vectorizing)
//
// Data types:
//   - Input pixels: uint8_t → widened to int16_t for arithmetic
//   - Gx, Gy accumulators: int16_t (max value ±1020, fits in int16)
//   - Magnitude: int16_t before clamp, uint8_t after
//   - LMUL=1 for int16 vectors (chosen after sweep — good balance of
//     register pressure vs elements per iteration)
//
// Memory layout:
//   - Input is flat row-major: pixel(y,x) = data[y*w + x]
//   - mag and dir outputs same layout
// ─────────────────────────────────────────────────────────────────────────────

void applySobelRVV(const Image& input, Image& mag, Image& dir) {

    const int w = input.width;
    const int h = input.height;

    // Set up output images — same as scalar version
    mag.width  = w;  mag.height = h;  mag.data.assign(w * h, 0);
    dir.width  = w;  dir.height = h;  dir.data.assign(w * h, 0);


    uint8_t*       dst_mag = mag.data.data();
    uint8_t*       dst_dir = dir.data.data();

#ifdef __riscv_vector
    const uint8_t* src = input.data.data();
    // ── RVV path ─────────────────────────────────────────────────────────────
    for (int y = 1; y < h - 1; ++y) {

        // Pointers to the three rows we need for this output row
        const uint8_t* above = src + (y - 1) * w;
        const uint8_t* curr  = src +  y      * w;
        const uint8_t* below = src + (y + 1) * w;

        int x = 1;
        while (x < w - 1) {

            // Ask hardware: how many int16 elements can you handle?
            // We use e16m1 — 16-bit elements, LMUL=1.
            // At VLEN=128: vl=8, VLEN=256: vl=16, VLEN=512: vl=32.
            // The code never assumes any specific value — that is VLA.
            size_t vl = __riscv_vsetvl_e16m1((w - 1) - x);

            // ── Load 8 pixel neighborhoods as uint8, widen to int16 ──────────
            //
            // For Gx kernel:   -1  0  +1
            //                  -2  0  +2
            //                  -1  0  +1
            // We need: above[x-1], above[x+1],
            //          curr[x-1],  curr[x+1],
            //          below[x-1], below[x+1]
            //
            // For Gy kernel:   -1 -2  -1
            //                   0  0   0
            //                  +1 +2  +1
            // We need: above[x-1], above[x], above[x+1],
            //          below[x-1], below[x], below[x+1]
            //
            // Total unique loads: above[x-1], above[x], above[x+1]
            //                     curr[x-1],  curr[x+1]
            //                     below[x-1], below[x], below[x+1]
            // = 8 loads. above[x] and below[x] only used in Gy.
            // curr[x] not used at all (weight=0 in both kernels).

            // Load uint8 and widen to int16 in one step using vuint8→vint16
            // __riscv_vwcvtu_x converts unsigned 8-bit to wider type

            vint16m1_t va_left  = __riscv_vwcvtu_x_x_v_i16m1(
                __riscv_vle8_v_u8mf2(above + x - 1, vl), vl);

            vint16m1_t va_mid   = __riscv_vwcvtu_x_x_v_i16m1(
                __riscv_vle8_v_u8mf2(above + x,     vl), vl);

            vint16m1_t va_right = __riscv_vwcvtu_x_x_v_i16m1(
                __riscv_vle8_v_u8mf2(above + x + 1, vl), vl);

            vint16m1_t vc_left  = __riscv_vwcvtu_x_x_v_i16m1(
                __riscv_vle8_v_u8mf2(curr  + x - 1, vl), vl);

            vint16m1_t vc_right = __riscv_vwcvtu_x_x_v_i16m1(
                __riscv_vle8_v_u8mf2(curr  + x + 1, vl), vl);

            vint16m1_t vb_left  = __riscv_vwcvtu_x_x_v_i16m1(
                __riscv_vle8_v_u8mf2(below + x - 1, vl), vl);

            vint16m1_t vb_mid   = __riscv_vwcvtu_x_x_v_i16m1(
                __riscv_vle8_v_u8mf2(below + x,     vl), vl);

            vint16m1_t vb_right = __riscv_vwcvtu_x_x_v_i16m1(
                __riscv_vle8_v_u8mf2(below + x + 1, vl), vl);

            // ── Compute Gx ───────────────────────────────────────────────────
            // Gx = -1*above_left + 1*above_right
            //      -2*curr_left  + 2*curr_right
            //      -1*below_left + 1*below_right
            //
            // = (above_right - above_left)
            //   + 2*(curr_right - curr_left)
            //   + (below_right - below_left)

            // above_right - above_left
            vint16m1_t gx = __riscv_vsub_vv_i16m1(va_right, va_left, vl);

            // += 2*(curr_right - curr_left)
            vint16m1_t curr_diff = __riscv_vsub_vv_i16m1(vc_right, vc_left, vl);
            // multiply by 2 = left shift by 1
            curr_diff = __riscv_vsll_vx_i16m1(curr_diff, 1, vl);
            gx = __riscv_vadd_vv_i16m1(gx, curr_diff, vl);

            // += (below_right - below_left)
            vint16m1_t below_diff_x = __riscv_vsub_vv_i16m1(vb_right, vb_left, vl);
            gx = __riscv_vadd_vv_i16m1(gx, below_diff_x, vl);

            // ── Compute Gy ───────────────────────────────────────────────────
            // Gy = -1*above_left - 2*above_mid - 1*above_right
            //      +1*below_left + 2*below_mid + 1*below_right
            //
            // = (below_left - above_left)
            //   + 2*(below_mid - above_mid)
            //   + (below_right - above_right)

            // below_left - above_left
            vint16m1_t gy = __riscv_vsub_vv_i16m1(vb_left, va_left, vl);

            // += 2*(below_mid - above_mid)
            vint16m1_t mid_diff = __riscv_vsub_vv_i16m1(vb_mid, va_mid, vl);
            mid_diff = __riscv_vsll_vx_i16m1(mid_diff, 1, vl);
            gy = __riscv_vadd_vv_i16m1(gy, mid_diff, vl);

            // += (below_right - above_right)
            vint16m1_t below_diff_y = __riscv_vsub_vv_i16m1(vb_right, va_right, vl);
            gy = __riscv_vadd_vv_i16m1(gy, below_diff_y, vl);

            // ── L1 Magnitude = |Gx| + |Gy| ──────────────────────────────────
            // No sqrt needed. Integer only. Vectorizable.
            //
            // |v| = max(v, -v)
            // This works because:
            //   if v >= 0: max(v, -v) = v = |v| ✓
            //   if v <  0: max(v, -v) = -v = |v| ✓

            vint16m1_t abs_gx = __riscv_vmax_vv_i16m1(
                gx, __riscv_vneg_v_i16m1(gx, vl), vl);

            vint16m1_t abs_gy = __riscv_vmax_vv_i16m1(
                gy, __riscv_vneg_v_i16m1(gy, vl), vl);

            vint16m1_t vmag = __riscv_vadd_vv_i16m1(abs_gx, abs_gy, vl);

            // ── Clamp to [0, 255] ────────────────────────────────────────────
            // L1 max is 1020+1020=2040, must clamp before narrowing to uint8
            vint16m1_t v255 = __riscv_vmv_v_x_i16m1(255, vl);
            vmag = __riscv_vmin_vv_i16m1(vmag, v255, vl);

            // ── Narrow int16 → uint8 and store magnitude ─────────────────────
            // __riscv_vncvt_x truncates upper bits — safe here because
            // we already clamped to [0,255] so upper byte is always 0
            vuint8mf2_t vmag8 = __riscv_vncvtu_x_x_w_u8mf2(
                __riscv_vreinterpret_v_i16m1_u16m1(vmag), vl);
            __riscv_vse8_v_u8mf2(dst_mag + y * w + x, vmag8, vl);

            x += vl;
        }

        // ── Direction — stays scalar ──────────────────────────────────────────
        // Direction is ~8% of runtime per the profiling report.
        // Not worth vectorizing. Uses the same atan2 logic as scalar version.
        for (int x2 = 1; x2 < w - 1; ++x2) {
            int gx_s = -1 * input.data[(y-1)*w + (x2-1)]
                       +1 * input.data[(y-1)*w + (x2+1)]
                       -2 * input.data[ y   *w + (x2-1)]
                       +2 * input.data[ y   *w + (x2+1)]
                       -1 * input.data[(y+1)*w + (x2-1)]
                       +1 * input.data[(y+1)*w + (x2+1)];

            int gy_s = -1 * input.data[(y-1)*w + (x2-1)]
                       -2 * input.data[(y-1)*w +  x2   ]
                       -1 * input.data[(y-1)*w + (x2+1)]
                       +1 * input.data[(y+1)*w + (x2-1)]
                       +2 * input.data[(y+1)*w +  x2   ]
                       +1 * input.data[(y+1)*w + (x2+1)];

            float angle = std::atan2(gy_s, gx_s) * 180.0f / M_PI;
            if (angle < 0) angle += 180.0f;

            unsigned char direction = 0;
            if      (angle >= 22.5  && angle < 67.5 ) direction = 45;
            else if (angle >= 67.5  && angle < 112.5) direction = 90;
            else if (angle >= 112.5 && angle < 157.5) direction = 135;
            else                                       direction = 0;

            dst_dir[y * w + x2] = direction;
        }
    }

#else
    // ── Scalar fallback (when not compiling for RISC-V) ──────────────────────
    // This runs on your host machine during `make test`.
    // Identical logic to src/sobel.cpp so tests can verify correctness.
    for (int y = 1; y < h - 1; ++y) {
        for (int x = 1; x < w - 1; ++x) {
            int gx = -1*input.data[(y-1)*w+(x-1)] +1*input.data[(y-1)*w+(x+1)]
                     -2*input.data[ y   *w+(x-1)] +2*input.data[ y   *w+(x+1)]
                     -1*input.data[(y+1)*w+(x-1)] +1*input.data[(y+1)*w+(x+1)];
            int gy = -1*input.data[(y-1)*w+(x-1)] -2*input.data[(y-1)*w+ x   ]
                     -1*input.data[(y-1)*w+(x+1)]
                     +1*input.data[(y+1)*w+(x-1)] +2*input.data[(y+1)*w+ x   ]
                     +1*input.data[(y+1)*w+(x+1)];

            int magnitude = std::abs(gx) + std::abs(gy);  // L1
            if (magnitude > 255) magnitude = 255;
            dst_mag[y * w + x] = static_cast<uint8_t>(magnitude);

            float angle = std::atan2(gy, gx) * 180.0f / M_PI;
            if (angle < 0) angle += 180.0f;
            unsigned char direction = 0;
            if      (angle >= 22.5  && angle < 67.5 ) direction = 45;
            else if (angle >= 67.5  && angle < 112.5) direction = 90;
            else if (angle >= 112.5 && angle < 157.5) direction = 135;
            dst_dir[y * w + x] = direction;
        }
    }
#endif
}
