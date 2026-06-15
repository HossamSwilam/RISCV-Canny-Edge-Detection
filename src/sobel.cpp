#include "processing.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdint>

void applySobel(const Image& input, Image& mag, Image& dir) {
    int w = input.width;
    int h = input.height;
    
    // تخصيص الذاكرة المحاذية للمخرجات
    mag.allocate(w, h);
    dir.allocate(w, h);

    // المصفوفات الداخلية دي عادية لأنها local وتنتهي بنهاية الدالة
    std::vector<int16_t> gx_array(w * h, 0);
    std::vector<int16_t> gy_array(w * h, 0);
    std::vector<int32_t> raw_mag(w * h, 0);

    int max_mag = 0;

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

            gx_array[idx] = static_cast<int16_t>(gx);
            gy_array[idx] = static_cast<int16_t>(gy);

            int magnitude = std::round(std::sqrt(gx * gx + gy * gy));
            raw_mag[idx] = magnitude;

            if (magnitude > max_mag) {
                max_mag = magnitude;
            }

            int ax = std::abs(gx);
            int ay = std::abs(gy);
            unsigned char direction = 0;

            if (5 * ay < 2 * ax) {
                direction = 0;  
            } else if (5 * ay > 12 * ax) {
                direction = 90; 
            } else {
                if (gx * gy > 0) {
                    direction = 135; 
                } else {
                    direction = 45;
                }
            }
            dir.data[idx] = direction;
        }
    }

    if (max_mag == 0) max_mag = 1;

    for (int y = 1; y < h - 1; ++y) {
        for (int x = 1; x < w - 1; ++x) {
            int idx = y * w + x;
            int normalized_mag = (raw_mag[idx] * 255) / max_mag;
            mag.data[idx] = static_cast<unsigned char>(normalized_mag);
        }
    }
}
