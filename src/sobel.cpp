#include "processing.h"
#include <cmath>

void applySobel(const Image& input, Image& mag, Image& dir) {
    // Set up dimensions for output images
    int w = input.width;
    int h = input.height;
    
    mag.width = w;
    mag.height = h;
    mag.data.assign(w * h, 0); // Initialize array with zeros

    dir.width = w;
    dir.height = h;
    dir.data.assign(w * h, 0); // Initialize array with zeros

    // Loop through all image pixels (ignoring outer edges for border handling)
    for (int y = 1; y < h - 1; ++y) {
        for (int x = 1; x < w - 1; ++x) {
            
            // Calculate horizontal gradient (Sobel X)
            int gx = -1 * input.data[(y - 1) * w + (x - 1)] 
                     +1 * input.data[(y - 1) * w + (x + 1)]
                     -2 * input.data[y * w + (x - 1)] 
                     +2 * input.data[y * w + (x + 1)]
                     -1 * input.data[(y + 1) * w + (x - 1)] 
                     +1 * input.data[(y + 1) * w + (x + 1)];

            // Calculate vertical gradient (Sobel Y)
            int gy = -1 * input.data[(y - 1) * w + (x - 1)] 
                     -2 * input.data[(y - 1) * w + x] 
                     -1 * input.data[(y - 1) * w + (x + 1)]
                     +1 * input.data[(y + 1) * w + (x - 1)] 
                     +2 * input.data[(y + 1) * w + x] 
                     +1 * input.data[(y + 1) * w + (x + 1)];

            // 1. Calculate edge magnitude using L2 Norm
            int magnitude = std::round(std::sqrt(gx * gx + gy * gy));
            if (magnitude > 255) magnitude = 255; // Ensure value does not exceed 255
            mag.data[y * w + x] = static_cast<unsigned char>(magnitude);

            // 2. Calculate edge direction
            float angle = std::atan2(gy, gx) * 180.0f / M_PI;
            if (angle < 0) {
                angle += 180.0f; // Normalize angles to be between 0 and 180
            }

            // Approximate angle to 4 basic directions (0, 45, 90, 135)
            unsigned char direction = 0;
            if (angle >= 22.5 && angle < 67.5) {
                direction = 45;
            } else if (angle >= 67.5 && angle < 112.5) {
                direction = 90;
            } else if (angle >= 112.5 && angle < 157.5) {
                direction = 135;
            } else {
                direction = 0; // Angles close to 0 or 180
            }

            dir.data[y * w + x] = direction;
        }
    }
}
