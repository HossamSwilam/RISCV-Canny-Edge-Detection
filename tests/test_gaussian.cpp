#include <cstring>
#include <gtest/gtest.h>
#include "processing.h"

// 1. Test that dimensions remain unchanged
TEST(GaussianBlurTest, DimensionsRemainUnchanged) {
    Image input;
    input.allocate(3, 3);
    uint8_t vals[] = {10, 10, 10, 10, 50, 10, 10, 10, 10};
    std::memcpy(input.data, vals, 9);

    Image output = applyGaussianBlur(input);
    
    EXPECT_EQ(output.width, input.width);
    EXPECT_EQ(output.height, input.height);
    
    input.free_memory();
    output.free_memory();
}

// 2. Test that a black image remains black
TEST(GaussianBlurTest, BlackImageRemainsBlack) {
    Image input;
    input.allocate(10, 10);
    std::memset(input.data, 0, 100);

    Image output = applyGaussianBlur(input);

    for (int i = 0; i < 100; i++)
        EXPECT_EQ(output.data[i], 0);
        
    input.free_memory();
    output.free_memory();
}

// 3. Test that a uniform image remains uniform
TEST(GaussianBlurTest, UniformImageRemainsUniform) {
    Image input;
    input.allocate(10, 10);
    std::memset(input.data, 128, 100);

    Image output = applyGaussianBlur(input);

    for (int y = 2; y < 8; y++) {
        for (int x = 2; x < 8; x++) {
            EXPECT_NEAR(output.data[y * 10 + x], 128, 1);
        }
    }
    input.free_memory();
    output.free_memory();
}

// 4. Test symmetric spreading of an impulse
TEST(GaussianBlurTest, ImpulseSpreadsSymmetrically) {
    Image input;
    input.allocate(11, 11);
    std::memset(input.data, 0, 121);

    input.data[5 * 11 + 5] = 255;

    Image output = applyGaussianBlur(input);

    EXPECT_GT(output.data[5 * 11 + 5], 0);
    EXPECT_EQ(output.data[4 * 11 + 5], output.data[6 * 11 + 5]);
    EXPECT_EQ(output.data[5 * 11 + 4], output.data[5 * 11 + 6]);
    
    input.free_memory();
    output.free_memory();
}

// =========================================================================
// 5. Boundary Test: Ensure edges are not black or filled with garbage
// =========================================================================
TEST(GaussianBlurTest, BoundaryPixelsAreProcessed) {
    Image input;
    input.allocate(10, 10);
    std::memset(input.data, 255, 100); // Fully white image

    Image output = applyGaussianBlur(input);

    // Edges and corners should not be zero (black)
    EXPECT_GT(output.data[0], 0); // Top-left corner
    EXPECT_GT(output.data[9], 0); // Top-right corner
    EXPECT_GT(output.data[9 * 10 + 0], 0); // Bottom-left corner
    EXPECT_GT(output.data[9 * 10 + 9], 0); // Bottom-right corner
    
    input.free_memory();
    output.free_memory();
}

// =========================================================================
// 6. Equivalence Test: Compare standard 2D and Separable implementations
// =========================================================================
TEST(GaussianBlurTest, SeparableEquivalence) {
    Image input;
    input.allocate(10, 10);
    
    // Fill image with a gradient pattern for a realistic test
    for(int i = 0; i < 100; i++) {
        input.data[i] = i % 256; 
    }

    Image output_2d = applyGaussianBlur(input);
    Image output_sep = gaussianSeparable(input); 

    // Compare each pixel (using EXPECT_NEAR due to slight rounding differences 
    // caused by different kernel sums: 273 vs 289)
    for (int i = 0; i < 100; i++) {
        EXPECT_NEAR(output_2d.data[i], output_sep.data[i], 3); 
    }

    input.free_memory();
    output_2d.free_memory();
    output_sep.free_memory();
}
