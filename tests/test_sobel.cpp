#include <gtest/gtest.h>
#include "processing.h"
#include <cmath>
#include <cstring>

// 1. Ideal Vertical Edge Test
// Tests if the Sobel operator correctly identifies a sharp vertical boundary.
TEST(SobelTest, SimpleVerticalEdge) {
    Image input;
    input.allocate(3, 3);
    
    // Create an image with a sharp transition from left (black) to right (white)
    uint8_t vals[] = {
        0, 128, 255,
        0, 128, 255,
        0, 128, 255
    };
    std::memcpy(input.data, vals, 9);

    Image mag, dir;
    applySobel(input, mag, dir, true); // true = Use L2 norm

    // Center pixel (index 4) should have maximum magnitude (255)
    EXPECT_EQ(mag.data[4], 255);
    // A vertical edge means the color gradient moves horizontally, so angle is 0 degrees
    EXPECT_EQ(dir.data[4], 0); 
    
    input.free_memory(); mag.free_memory(); dir.free_memory();
}

// 2. Ideal Horizontal Edge Test
// Tests if the Sobel operator correctly identifies a sharp horizontal boundary.
TEST(SobelTest, SimpleHorizontalEdge) {
    Image input;
    input.allocate(3, 3);
    
    // Create an image with a sharp transition from top (black) to bottom (white)
    uint8_t vals[] = {
        0,   0,   0,
        128, 128, 128,
        255, 255, 255
    };
    std::memcpy(input.data, vals, 9);

    Image mag, dir;
    applySobel(input, mag, dir, true);

    // Center pixel should have maximum magnitude
    EXPECT_EQ(mag.data[4], 255);
    // A horizontal edge means the color gradient moves vertically, so angle is 90 degrees
    EXPECT_EQ(dir.data[4], 90); 
    
    input.free_memory(); mag.free_memory(); dir.free_memory();
}

// 3. Diagonal Edge Test
// Ensures the algorithm correctly detects edges that are not strictly aligned to X or Y axes.
TEST(SobelTest, DiagonalEdge) {
    Image input;
    input.allocate(5, 5);
    
    // Split the image diagonally into a white half and a black half (x >= y)
    for (int y = 0; y < 5; ++y) {
        for (int x = 0; x < 5; ++x) {
            input.data[y * 5 + x] = (x >= y) ? 255 : 0;
        }
    }

    Image mag, dir;
    applySobel(input, mag, dir, true);

    // The center pixel (index 12) should detect a strong edge
    EXPECT_GT(mag.data[12], 0); 
    
    // A diagonal transition should fall into either the 45-degree or 135-degree bin
    EXPECT_TRUE(dir.data[12] == 45 || dir.data[12] == 135); 
    
    input.free_memory(); mag.free_memory(); dir.free_memory();
}

// 4. Uniform Image Invariant Test
// A core property test: an image with no color variations must produce zero gradients.
TEST(SobelTest, UniformImage_ZeroMagnitude) {
    Image input;
    input.allocate(5, 5);
    
    // Fill the entire image with a solid gray color (128)
    std::memset(input.data, 128, 25);

    Image mag, dir;
    applySobel(input, mag, dir, true);

    // Iterate over the inner pixels (ignoring the 1-pixel border)
    // All magnitudes must be strictly 0 since there are no edges
    for (int y = 1; y < 4; ++y)
        for (int x = 1; x < 4; ++x)
            EXPECT_EQ(mag.data[y * 5 + x], 0);
            
    input.free_memory(); mag.free_memory(); dir.free_memory();
}

// 5. L1 vs L2 Norm Integrity Test
// Verifies that both magnitude calculation methods (Euclidean and Manhattan approximations)
// function correctly without crashing or producing zero on a gradient image.
TEST(SobelTest, BothMagnitudeMethodsWork) {
    Image input;
    input.allocate(5, 5);
    
    // Generate a simple gradient image
    for(int i = 0; i < 25; i++) input.data[i] = i * 10; 

    Image mag_l2, dir_l2;
    applySobel(input, mag_l2, dir_l2, true); // Process using L2 Norm (sqrt)
    
    Image mag_l1, dir_l1;
    applySobel(input, mag_l1, dir_l1, false); // Process using L1 Norm (abs)
    
    // Both methods should successfully detect non-zero magnitudes in the center
    EXPECT_GT(mag_l2.data[12], 0);
    EXPECT_GT(mag_l1.data[12], 0);
    
    input.free_memory(); 
    mag_l2.free_memory(); dir_l2.free_memory();
    mag_l1.free_memory(); dir_l1.free_memory();
}