#include "processing.h"
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <algorithm>

void applySobel(const Image& input, Image& mag, Image& dir, bool use_l2) {
    const int w = input.width;
    const int h = input.height;

    // Allocate memory for the final output images (magnitude and direction)
    mag.allocate(w, h);
    dir.allocate(w, h);

    // Zero-initialise the entire output buffers. 
    // This ensures that the 1-pixel outer boundary (which the Sobel 3x3 window 
    // cannot process) defaults to 0 (black edge) instead of retaining garbage data.
    std::memset(mag.data, 0, w * h);
    std::memset(dir.data, 0, w * h);

    // --- MEMORY ARCHITECTURE & SIMD OPTIMIZATION ---
    // 64-byte aligned allocations ensure the arrays perfectly align with typical cache lines.
    // We use a Structure of Arrays (SoA) layout (separate arrays for Gx, Gy, and raw_mag).
    // SoA is critical for the RISC-V Vector (RVV) extension: a single vector load (vle16.v) 
    // can load consecutive Gx values natively, whereas an Array of Structures (AoS) layout 
    // would require expensive and slow gather/scatter operations.
    int16_t* gx_array = static_cast<int16_t*>(
        aligned_alloc(64, w * h * sizeof(int16_t)));
    int16_t* gy_array = static_cast<int16_t*>(
        aligned_alloc(64, w * h * sizeof(int16_t)));
    int32_t* raw_mag  = static_cast<int32_t*>(
        aligned_alloc(64, w * h * sizeof(int32_t)));

    // Clear intermediate buffers to prevent undefined behavior
    std::memset(gx_array, 0, w * h * sizeof(int16_t));
    std::memset(gy_array, 0, w * h * sizeof(int16_t));
    std::memset(raw_mag,  0, w * h * sizeof(int32_t));

    // Tracks the absolute maximum gradient found in the image for later normalization
    int max_mag = 0;

    // --- SOBEL KERNEL APPLICATION ---
    // Sobel kernels are implicitly applied via unrolled arithmetic rather than nested loops.
    // Unrolling avoids loop overhead and allows the compiler to optimize instruction pipelines.
    //   Gx: [[-1, 0, +1], [-2, 0, +2], [-1, 0, +1]] -> detects vertical edges
    //   Gy: [[-1,-2, -1], [ 0, 0,  0], [+1,+2, +1]] -> detects horizontal edges
    // Maximum theoretical gradient: 4 * 255 = 1020, which safely fits inside int16_t.
    
    // We iterate from 1 to h-1 and 1 to w-1 to avoid out-of-bounds memory access at the image edges.
    for (int y = 1; y < h - 1; ++y) {
        for (int x = 1; x < w - 1; ++x) {
            const int idx = y * w + x;

            // Compute Gx (Horizontal Gradient)
            // We use standard 32-bit 'int' here to utilize the native CPU register width, 
            // maximizing arithmetic speed and preventing any intermediate overflow.
            int gx = -1 * input.data[(y-1)*w + (x-1)]
                     +1 * input.data[(y-1)*w + (x+1)]
                     -2 * input.data[ y   *w + (x-1)]
                     +2 * input.data[ y   *w + (x+1)]
                     -1 * input.data[(y+1)*w + (x-1)]
                     +1 * input.data[(y+1)*w + (x+1)];

            // Compute Gy (Vertical Gradient)
            int gy = -1 * input.data[(y-1)*w + (x-1)]
                     -2 * input.data[(y-1)*w +  x   ]
                     -1 * input.data[(y-1)*w + (x+1)]
                     +1 * input.data[(y+1)*w + (x-1)]
                     +2 * input.data[(y+1)*w +  x   ]
                     +1 * input.data[(y+1)*w + (x+1)];

            // Downcast to int16_t for storage. This halves the memory footprint and doubles
           
            gx_array[idx] = static_cast<int16_t>(gx);
            gy_array[idx] = static_cast<int16_t>(gy);

            // --- MAGNITUDE CALCULATION ---
            int32_t magnitude = 0;
            if (use_l2) {
                // L2 Norm (Euclidean distance): Accurate but computationally expensive due to sqrt()
                // Formula: $\sqrt{G_x^2 + G_y^2}$
                magnitude = static_cast<int32_t>(
                    std::round(std::sqrt(static_cast<double>(gx)*gx
                                       + static_cast<double>(gy)*gy)));
            } else {
                // L1 Norm (Manhattan distance): Hardware-friendly, integer-only approximation.
                // Formula: $|G_x| + |G_y|$
                // Highly efficient for Embedded/RISC-V systems as it avoids floating-point units.
                magnitude = std::abs(gx) + std::abs(gy);
            }
            
            raw_mag[idx] = magnitude;
            // Update the global maximum magnitude for the normalization pass
            if (magnitude > max_mag) max_mag = magnitude;

            // --- DIRECTION QUANTIZATION (EDGE ANGLE) ---
            // Quantise the gradient direction into 4 primary bins: 0, 45, 90, 135 degrees.
            // PERFORMANCE HACK: Instead of using the extremely slow atan2(gy, gx) floating-point function,
            // we use cross-multiplication logic based on tangent boundaries.
            //   tan(22.5°) ≈ 2/5  -> boundary: 5*|gy| < 2*|gx|  -> 0° (Vertical Edge)
            //   tan(67.5°) ≈ 12/5 -> boundary: 5*|gy| > 12*|gx| -> 90° (Horizontal Edge)
            //   Otherwise, check if Gx and Gy have the same sign for 45° vs 135° (Diagonal Edges).
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

    // --- NORMALIZATION PASS ---
    // Two-pass normalisation: We must wait until the entire image is processed to know 'max_mag'.
    // Here, we scale every pixel's magnitude proportionally so the strongest edge becomes exactly 255.
    
    // Safety check to prevent Division by Zero if the input image is completely blank/uniform.
    if (max_mag == 0) max_mag = 1;  

    for (int y = 1; y < h - 1; ++y) {
        for (int x = 1; x < w - 1; ++x) {
            const int idx = y * w + x;
            // Scale formula: (current_magnitude * 255) / maximum_magnitude
            mag.data[idx] = static_cast<unsigned char>(
                (raw_mag[idx] * 255) / max_mag);
        }
    }

    // Free intermediate dynamically allocated aligned memory to prevent memory leaks
    std::free(gx_array);
    std::free(gy_array);
    std::free(raw_mag);
}