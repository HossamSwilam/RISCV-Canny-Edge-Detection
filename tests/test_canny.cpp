#include <gtest/gtest.h>
#include "processing.h"
#include <cstring>

TEST(CannyPostProcessingTest, HandlesEmptyImageSafely) {
    Image empty;
    empty.allocate(0, 0);

    // Should not crash on empty image
    Image result = applyCannyPostProcessing(empty, empty);
    EXPECT_EQ(result.width, 0);
    EXPECT_EQ(result.height, 0);

    empty.free_memory();
    result.free_memory();
}