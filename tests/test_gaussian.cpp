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
