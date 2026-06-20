#include "processing.h"
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <algorithm>

void applySobel(const Image& input, Image& mag, Image& dir, bool use_l2) {
    const int w = input.width;
    const int h = input.height;

    mag.allocate(w, h);
    dir.allocate(w, h);

    // Zero-initialise so border pixels (not written by the loop) are 0,
    // not garbage from aligned_alloc.
    std::memset(mag.data, 0, w * h);
    std::memset(dir.data, 0, w * h);

    // Aligned intermediate buffers (SoA layout: Gx and Gy as separate arrays).
    // SoA is critical for the RVV version: a single vle16 loads consecutive Gx
    // values, whereas an AoS layout would require gather operations.
    int16_t* gx_array = static_cast<int16_t*>(
        aligned_alloc(64, w * h * sizeof(int16_t)));
    int16_t* gy_array = static_cast<int16_t*>(
        aligned_alloc(64, w * h * sizeof(int16_t)));
    int32_t* raw_mag  = static_cast<int32_t*>(
        aligned_alloc(64, w * h * sizeof(int32_t)));

    std::memset(gx_array, 0, w * h * sizeof(int16_t));
    std::memset(gy_array, 0, w * h * sizeof(int16_t));
    std::memset(raw_mag,  0, w * h * sizeof(int32_t));

    int max_mag = 0;

    // Sobel kernels (implicit, not stored as arrays to avoid extra memory):
    //   Gx: [[-1,0,+1],[-2,0,+2],[-1,0,+1]]  detects vertical edges
    //   Gy: [[-1,-2,-1],[0,0,0],[+1,+2,+1]]   detects horizontal edges
    //
    // Maximum possible output: 4*255 = 1020 — fits in int16_t (range ±32767).
    for (int y = 1; y < h - 1; ++y) {
        for (int x = 1; x < w - 1; ++x) {
            const int idx = y * w + x;

            int gx = -1 * input.data[(y-1)*w + (x-1)]
                     +1 * input.data[(y-1)*w + (x+1)]
                     -2 * input.data[ y   *w + (x-1)]
                     +2 * input.data[ y   *w + (x+1)]
                     -1 * input.data[(y+1)*w + (x-1)]
                     +1 * input.data[(y+1)*w + (x+1)];

            int gy = -1 * input.data[(y-1)*w + (x-1)]
                     -2 * input.data[(y-1)*w +  x   ]
                     -1 * input.data[(y-1)*w + (x+1)]
                     +1 * input.data[(y+1)*w + (x-1)]
                     +2 * input.data[(y+1)*w +  x   ]
                     +1 * input.data[(y+1)*w + (x+1)];

            gx_array[idx] = static_cast<int16_t>(gx);
            gy_array[idx] = static_cast<int16_t>(gy);

            int32_t magnitude = 0;
            if (use_l2) {
                // L2 norm: geometrically correct, requires sqrt (float).
                magnitude = static_cast<int32_t>(
                    std::round(std::sqrt(static_cast<double>(gx)*gx
                                       + static_cast<double>(gy)*gy)));
            } else {
                // L1 norm: integer-only, ~41% overestimate on diagonal edges.
                magnitude = std::abs(gx) + std::abs(gy);
            }
            raw_mag[idx] = magnitude;
            if (magnitude > max_mag) max_mag = magnitude;

            // Quantise gradient direction to one of four angles (in degrees).
            // Uses integer cross-multiplication instead of atan2():
            //   tan(22.5°) ≈ 2/5  → boundary:  5*|gy| < 2*|gx|  → 0°
            //   tan(67.5°) ≈ 12/5 → boundary: 5*|gy| > 12*|gx|  → 90°
            //   Otherwise sign(gx)==sign(gy) → 45°, else → 135°
            const int ax = std::abs(gx);
            const int ay = std::abs(gy);
            unsigned char direction;

            if (5 * ay < 2 * ax) {
                direction = 0;
            } else if (5 * ay > 12 * ax) {
                direction = 90;
            } else {
                direction = (gx * gy > 0) ? 135 : 45;
            }
            dir.data[idx] = direction;
        }
    }

    // Two-pass normalisation: first pass finds the global maximum (done above),
    // second pass scales every magnitude to [0, 255].
    // A single-pass approach is not straightforward because the scale factor
    // depends on the global maximum, which is unknown until all pixels are seen.
    if (max_mag == 0) max_mag = 1;  // avoid division by zero on blank images

    for (int y = 1; y < h - 1; ++y) {
        for (int x = 1; x < w - 1; ++x) {
            const int idx = y * w + x;
            mag.data[idx] = static_cast<unsigned char>(
                (raw_mag[idx] * 255) / max_mag);
        }
    }

    std::free(gx_array);
    std::free(gy_array);
    std::free(raw_mag);
}
