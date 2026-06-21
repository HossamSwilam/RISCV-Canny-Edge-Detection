#include "processing.h"
#include <cmath>
#include <algorithm>

void applySobel(const Image& input, Image& mag, Image& dir, bool use_l2) {
    int w = input.width;
    int h = input.height;
    
    mag.allocate(w, h);
    dir.allocate(w, h);

    for (int y = 1; y < h - 1; ++y) {
        for (int x = 1; x < w - 1; ++x) {
            int idx = y * w + x;
            
            int gx = -1 * input.data[(y - 1) * w + (x - 1)] 
                     +1 * input.data[(y - 1) * w + (x + 1)]
                     -2 * input.data[y * w + (x - 1)] 
                     +2 * input.data[y * w + (x + 1)]
                     -1 * input.data[(y + 1) * w + (x - 1)] 
                     +1 * input.data[(y + 1) * w + (x + 1)];

            int gy = -1 * input.data[(y - 1) * w + (x - 1)] 
                     -2 * input.data[(y - 1) * w + x] 
                     -1 * input.data[(y - 1) * w + (x + 1)]
                     +1 * input.data[(y + 1) * w + (x - 1)] 
                     +2 * input.data[(y + 1) * w + x] 
                     +1 * input.data[(y + 1) * w + (x + 1)];

            int magnitude = 0;
            if (use_l2) {
                magnitude = std::round(std::sqrt(gx * gx + gy * gy));
            } else {
                magnitude = std::abs(gx) + std::abs(gy);
            }

            // --- التعديل الأول: استخدام Clamping بدلاً من Two-Pass ---
            if (magnitude > 255) {
                magnitude = 255;
            }
            mag.data[idx] = static_cast<unsigned char>(magnitude);

            int ax = std::abs(gx);
            int ay = std::abs(gy);
            unsigned char direction = 0;

            // --- التعديل الثاني: توحيد شروط الزوايا لتطابق كود الـ RVV ---
            if (2 * ay < ax) {
                direction = 0;  
            } else if (2 * ax < ay) {
                direction = 90; 
            } else {
                if ((gx < 0) ^ (gy < 0)) { // XOR comparison
                    direction = 135; 
                } else {
                    direction = 45;
                }
            }
            dir.data[idx] = direction;
        }
    }
}
