#include <gtest/gtest.h>
#include "processing.h"

TEST(CannyPostProcessingTest, HandlesEmptyImageSafely) {
    Image mag, dir;
    mag.width = 0; mag.height = 0; dir.width = 0; dir.height = 0;

    Image output = applyCannyPostProcessing(mag, dir);
    
    EXPECT_TRUE(output.data.empty());
}
