#include <gtest/gtest.h>

#include "processing.h"

#include "image.h"

#include <cmath>



// ─────────────────────────────────────────────────────────────────────────────

// HELPER: compare two images pixel by pixel with optional ±tolerance

// tolerance=0 → exact match (use for scalar-vs-scalar)

// tolerance=1 → allow rounding difference (use for scalar-vs-RVV in Phase 6)

// ─────────────────────────────────────────────────────────────────────────────

void compareImages(const Image& img1, const Image& img2, int tolerance = 0) {

    ASSERT_EQ(img1.width,  img2.width)  << "Width mismatch";

    ASSERT_EQ(img1.height, img2.height) << "Height mismatch";

    for (size_t i = 0; i < img1.data.size(); ++i) {

        int diff = std::abs((int)img1.data[i] - (int)img2.data[i]);

        EXPECT_LE(diff, tolerance)

            << "Pixel " << i << ": got " << (int)img1.data[i]

            << ", expected " << (int)img2.data[i]

            << " (tolerance=" << tolerance << ")";

    }

}



// ─────────────────────────────────────────────────────────────────────────────

// TEST 1: Sharp vertical edge → Gx large, Gy=0, mag=255, dir=0

//

// Hand calculation at center pixel (y=1, x=1):

//   Gx = (-1×0 + 1×255)    top row

//      + (-2×0 + 2×255)    mid row

//      + (-1×0 + 1×255)    bot row

//      = 255 + 510 + 255 = 1020

//   Gy = (-1×0 - 2×128 - 1×255)   top row  = -511

//      + (+1×0 + 2×128 + 1×255)   bot row  = +511

//      = 0

//   magnitude = sqrt(1020² + 0²) = 1020 → clamped to 255

//   direction = atan2(0, 1020) = 0° → quantized to 0

// ─────────────────────────────────────────────────────────────────────────────

TEST(SobelTest, SimpleVerticalEdge) {

    Image input;

    input.width  = 3;

    input.height = 3;

    input.data = {

          0, 128, 255,

          0, 128, 255,

          0, 128, 255

    };



    Image mag, dir;

    applySobel(input, mag, dir);



    // Center pixel is index y*w+x = 1*3+1 = 4

    EXPECT_EQ(mag.data[4], 255)

        << "Magnitude should be 255 (clamped from 1020)";

    EXPECT_EQ(dir.data[4], 0)

        << "Direction should be 0 (purely horizontal gradient)";

}



// ─────────────────────────────────────────────────────────────────────────────

// TEST 2: Diagonal edge → direction must be specifically 45°

//

// Hand calculation at center pixel (y=1, x=1):

//   Gx = (-1×255 + 1×0) + (-2×255 + 2×0) + (-1×0 + 1×0) = -765

//   Gy = (-1×255 - 2×255 - 1×0) + (+1×0 + 2×0 + 1×0)    = -765

//   atan2(-765, -765) = -135° → +180° = 45° → quantized to 45

// ─────────────────────────────────────────────────────────────────────────────

TEST(SobelTest, DiagonalEdge_Direction45) {

    Image input;

    input.width  = 3;

    input.height = 3;

    input.data = {

        255, 255,   0,

        255,   0,   0,

          0,   0,   0

    };



    Image mag, dir;

    applySobel(input, mag, dir);



    // SPECIFIC assertion — not "one of four values" but exactly 45

    EXPECT_EQ(dir.data[4], 45)

        << "Diagonal edge should quantize to 45 degrees";



    // Also verify magnitude is nonzero — there IS an edge here

    EXPECT_GT(mag.data[4], 0)

        << "Diagonal edge should have nonzero magnitude";

}



// ─────────────────────────────────────────────────────────────────────────────

// TEST 3: Uniform image → zero gradient, zero magnitude

//

// Mathematical guarantee: both Sobel kernels sum to zero.

// On a constant image every left-side contribution cancels

// its right-side counterpart exactly. mag must be 0 everywhere.

// ─────────────────────────────────────────────────────────────────────────────

TEST(SobelTest, UniformImage_ZeroMagnitude) {

    Image input;

    input.width  = 5;

    input.height = 5;

    // Fill all 25 pixels with 128

    input.data.assign(25, 128);



    Image mag, dir;

    applySobel(input, mag, dir);



    // Check all interior pixels (y=1..3, x=1..3)

    for (int y = 1; y < 4; ++y)

        for (int x = 1; x < 4; ++x)

            EXPECT_EQ(mag.data[y * 5 + x], 0)

                << "Uniform image: mag must be 0 at (" << x << "," << y << ")";

}



// ─────────────────────────────────────────────────────────────────────────────

// TEST 4: Magnitude is always in [0, 255] and direction in {0,45,90,135}

// ─────────────────────────────────────────────────────────────────────────────

TEST(SobelTest, OutputBoundsAlwaysValid) {

    Image input;

    input.width  = 5;

    input.height = 5;

    // Worst-case: checkerboard of 0 and 255

    input.data = {

          0, 255,   0, 255,   0,

        255,   0, 255,   0, 255,

          0, 255,   0, 255,   0,

        255,   0, 255,   0, 255,

          0, 255,   0, 255,   0

    };



    Image mag, dir;

    applySobel(input, mag, dir);



    for (size_t i = 0; i < input.data.size(); ++i) {

        EXPECT_LE(mag.data[i], 255) << "Overflow at index " << i;

        EXPECT_GE(mag.data[i],   0) << "Underflow at index " << i;



        uint8_t d = dir.data[i];

        EXPECT_TRUE(d == 0 || d == 45 || d == 90 || d == 135)

            << "Invalid direction " << (int)d << " at index " << i;

    }

}
