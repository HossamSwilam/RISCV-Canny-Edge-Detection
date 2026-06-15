#include <gtest/gtest.h>
#include "processing.h"
#include <cmath>
#include <cstring>

TEST(SobelTest, SimpleVerticalEdge) {
    Image input;
    input.allocate(3, 3);
    uint8_t vals[] = {
        0, 128, 255,
        0, 128, 255,
        0, 128, 255
    };
    std::memcpy(input.data, vals, 9);

    Image mag, dir;
    applySobel(input, mag, dir, true); // true = L2 norm

    EXPECT_EQ(mag.data[4], 255);
    EXPECT_EQ(dir.data[4], 0);
    
    input.free_memory(); mag.free_memory(); dir.free_memory();
}

// الاختبار الناقص: حافة أفقية
TEST(SobelTest, SimpleHorizontalEdge) {
    Image input;
    input.allocate(3, 3);
    uint8_t vals[] = {
        0,   0,   0,
        128, 128, 128,
        255, 255, 255
    };
    std::memcpy(input.data, vals, 9);

    Image mag, dir;
    applySobel(input, mag, dir, true);

    EXPECT_EQ(mag.data[4], 255);
    EXPECT_EQ(dir.data[4], 90); // Direction for horizontal edge should be 90
    
    input.free_memory(); mag.free_memory(); dir.free_memory();
}

TEST(SobelTest, UniformImage_ZeroMagnitude) {
    Image input;
    input.allocate(5, 5);
    std::memset(input.data, 128, 25);

    Image mag, dir;
    applySobel(input, mag, dir, true);

    for (int y = 1; y < 4; ++y)
        for (int x = 1; x < 4; ++x)
            EXPECT_EQ(mag.data[y * 5 + x], 0);
            
    input.free_memory(); mag.free_memory(); dir.free_memory();
}

// الاختبار الناقص: التأكد إن الطريقتين L1 و L2 شغالين ومابيعملوش كراش
TEST(SobelTest, BothMagnitudeMethodsWork) {
    Image input;
    input.allocate(5, 5);
    // صورة عشوائية بسيطة
    for(int i=0; i<25; i++) input.data[i] = i * 10; 

    Image mag_l2, dir_l2;
    applySobel(input, mag_l2, dir_l2, true); // L2
    
    Image mag_l1, dir_l1;
    applySobel(input, mag_l1, dir_l1, false); // L1
    
    // التأكد إن النتايج مش أصفار في الطريقتين
    EXPECT_GT(mag_l2.data[12], 0);
    EXPECT_GT(mag_l1.data[12], 0);
    
    input.free_memory(); 
    mag_l2.free_memory(); dir_l2.free_memory();
    mag_l1.free_memory(); dir_l1.free_memory();
}
