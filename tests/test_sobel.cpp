#include <gtest/gtest.h>
#include "processing.h"

TEST(SobelTest, OutputsHaveCorrectDimensions) {
    Image input;
    input.width = 3; input.height = 3;
    input.data = {0, 0, 0, 255, 255, 255, 0, 0, 0};

    Image mag, dir;
    applySobel(input, mag, dir);
    
    EXPECT_EQ(mag.width, input.width);
    EXPECT_EQ(dir.width, input.width);
    EXPECT_EQ(mag.data.size(), input.data.size());
}
