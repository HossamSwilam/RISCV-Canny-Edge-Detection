#include "processing.h"

Image applyCannyPostProcessing(const Image& mag, const Image& dir) {

    Image output;

    output.width = mag.width;
    output.height = mag.height;

    output.data.assign(mag.width * mag.height, 0);

    int w = mag.width;
    int h = mag.height;

    for (int y = 1; y < h - 1; ++y) {

        for (int x = 1; x < w - 1; ++x) {

            int idx = y * w + x;

            unsigned char current = mag.data[idx];

            unsigned char neighbor1 = 0;
            unsigned char neighbor2 = 0;

            unsigned char angle = dir.data[idx];

            switch(angle) {

                case 0:
                    neighbor1 = mag.data[idx - 1];
                    neighbor2 = mag.data[idx + 1];
                    break;

                case 45:
                    neighbor1 = mag.data[(y - 1) * w + (x + 1)];
                    neighbor2 = mag.data[(y + 1) * w + (x - 1)];
                    break;

                case 90:
                    neighbor1 = mag.data[(y - 1) * w + x];
                    neighbor2 = mag.data[(y + 1) * w + x];
                    break;

                case 135:
                    neighbor1 = mag.data[(y - 1) * w + (x - 1)];
                    neighbor2 = mag.data[(y + 1) * w + (x + 1)];
                    break;
            }

            if (current >= neighbor1 &&
                current >= neighbor2)
            {
                output.data[idx] = current;
            }
            else {
                output.data[idx] = 0;
            }
        }
    }

    return output;
}

// 1. Double Thresholding Stage
Image applyDoubleThresholding(const Image& nms_img, unsigned char low_thresh, unsigned char high_thresh) {
    Image output;
    output.width = nms_img.width;
    output.height = nms_img.height;
    output.data.assign(nms_img.width * nms_img.height, 0);

    // Iterate through each pixel to classify it based on thresholds
    for (size_t i = 0; i < nms_img.data.size(); ++i) {
        if (nms_img.data[i] >= high_thresh) {
            output.data[i] = 255; // Strong pixel (definite edge)
        } else if (nms_img.data[i] >= low_thresh) {
            output.data[i] = 128; // Weak pixel (potential edge)
        } else {
            output.data[i] = 0;   // Non-edge pixel (suppress)
        }
    }
    return output;
}

// 2. Hysteresis Edge Tracing Stage
Image applyHysteresis(const Image& thresh_img) {
    Image output = thresh_img; 
    int w = output.width;
    int h = output.height;

    // Iterate through the image to find weak pixels connected to strong ones
    for (int y = 1; y < h - 1; ++y) {
        for (int x = 1; x < w - 1; ++x) {
            int idx = y * w + x;
            
            // If the pixel is weak, check its 8 neighbors
            if (output.data[idx] == 128) {
                if (output.data[(y-1)*w + x-1] == 255 || output.data[(y-1)*w + x] == 255 || output.data[(y-1)*w + x+1] == 255 ||
                    output.data[y*w + x-1] == 255     ||                                    output.data[y*w + x+1] == 255 ||
                    output.data[(y+1)*w + x-1] == 255 || output.data[(y+1)*w + x] == 255 || output.data[(y+1)*w + x+1] == 255) {
                    
                    output.data[idx] = 255; // Promote to strong pixel
                }
            }
        }
    }

    // Final cleanup: suppress any remaining weak pixels that are not connected to strong ones
    for (size_t i = 0; i < output.data.size(); ++i) {
        if (output.data[i] == 128) {
            output.data[i] = 0;
        }
    }

    return output;
}