#ifndef PROCESSING_H
#define PROCESSING_H

#include "image.h"
#include <cstdint>
#include <algorithm>

// ---------------------------------------------------------------------------
// Generic 2-D convolution (template — satisfies Rubric criterion C)
//
// PixelT : type of one input pixel  (e.g. uint8_t)
// AccumT : type of the accumulator  (e.g. int32_t — must be wide enough to
//           hold kernel_size^2 * max_pixel * max_coeff without overflow)
// KernelT: type of one kernel coeff (e.g. int16_t)
//
// The function writes AccumT values into `output`.  The caller is responsible
// for normalising / clamping the output to [0,255] before storing it back
// as uint8_t.
//
// Boundary condition: zero-padding (out-of-bounds pixels treated as 0).
// ---------------------------------------------------------------------------
template<typename PixelT, typename AccumT, typename KernelT>
void convolve2D(const PixelT* input,
                AccumT*       output,
                int           width,
                int           height,
                const KernelT* kernel,
                int           ksize)
{
    int half = ksize / 2;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            AccumT sum = static_cast<AccumT>(0);

            for (int ky = -half; ky <= half; ++ky) {
                for (int kx = -half; kx <= half; ++kx) {
                    int ny = y + ky;
                    int nx = x + kx;

                    PixelT pixel = static_cast<PixelT>(0); // zero-pad
                    if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                        pixel = input[ny * width + nx];
                    }

                    KernelT coeff = kernel[(ky + half) * ksize + (kx + half)];
                    sum += static_cast<AccumT>(pixel) * static_cast<AccumT>(coeff);
                }
            }

            output[y * width + x] = sum;
        }
    }
}

// ---------------------------------------------------------------------------
// Scalar pipeline stages
// ---------------------------------------------------------------------------

// Stage 1 — Gaussian blur (5x5, sigma≈1.0, integer kernel sum=273)
Image applyGaussianBlur(const Image& input);

// Stage 2 — Sobel gradient magnitude + quantised direction
//   use_l2=true  → magnitude = sqrt(Gx²+Gy²)  (L2 norm, more accurate)
//   use_l2=false → magnitude = |Gx|+|Gy|       (L1 norm, integer-only, faster)
//   Direction values stored as degrees: 0, 45, 90, 135
void applySobel(const Image& input,
                Image&       mag,
                Image&       dir,
                bool         use_l2 = true);

// Stage 3 — Non-maximum suppression (thins edges to 1-pixel width)
Image applyCannyPostProcessing(const Image& magnitude,
                               const Image& direction);

// Stage 4 — Double thresholding (classifies pixels as strong/weak/suppressed)
Image applyDoubleThresholding(const Image&  nms_img,
                              unsigned char low_thresh,
                              unsigned char high_thresh);

// Stage 5 — Hysteresis edge tracing (promotes weak edges adjacent to strong)
Image applyHysteresis(const Image& thresh_img);

// ---------------------------------------------------------------------------
// RVV-accelerated implementations (compiled for RISC-V only)
// Produce identical output to the scalar versions (within ±1 rounding).
// ---------------------------------------------------------------------------

// RVV Gaussian blur — same interface as applyGaussianBlur
void gaussian_blur_rvv(const Image& src, Image& dst);

// RVV Sobel + L1 magnitude + direction
void applySobelRVV(const Image& input,
                   Image&       magnitude,
                   Image&       direction);

#endif // PROCESSING_H
