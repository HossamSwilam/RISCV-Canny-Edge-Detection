#include <gtest/gtest.h>
#include "processing.h"

TEST(GaussianBlurTest, DimensionsRemainUnchanged) {
    Image input;
    input.width = 3; input.height = 3;
    input.data = {10, 10, 10, 10, 50, 10, 10, 10, 10};

    Image output = applyGaussianBlur(input);
    
    EXPECT_EQ(output.width, input.width);
    EXPECT_EQ(output.height, input.height);
    EXPECT_EQ(output.data.size(), input.data.size());
}


TEST(GaussianBlurTest, BlackImageRemainsBlack) {
    Image input;
    input.width = 10;
    input.height = 10;
    input.data.assign(100, 0);

    Image output = applyGaussianBlur(input);

    for (auto p : output.data)
        EXPECT_EQ(p, 0);
}

TEST(GaussianBlurTest, UniformImageRemainsUniform) {
    Image input;
    input.width = 10;
    input.height = 10;
    input.data.assign(100, 128);

    Image output = applyGaussianBlur(input);

    for (int y = 2; y < 8; y++) {
        for (int x = 2; x < 8; x++) {
            EXPECT_NEAR(output.data[y * 10 + x], 128, 1);
        }
    }
}

TEST(GaussianBlurTest, ImpulseSpreadsSymmetrically) {
    Image input;
    input.width = 11;
    input.height = 11;
    input.data.assign(121, 0);

    input.data[5 * 11 + 5] = 255;

    Image output = applyGaussianBlur(input);

    EXPECT_GT(output.data[5 * 11 + 5], 0);

    EXPECT_EQ(output.data[4 * 11 + 5],
              output.data[6 * 11 + 5]);

    EXPECT_EQ(output.data[5 * 11 + 4],
              output.data[5 * 11 + 6]);
}
