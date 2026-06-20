#include <gtest/gtest.h>
#include <cstring>
#include "processing.h"

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
