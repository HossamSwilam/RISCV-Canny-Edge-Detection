#include "processing.h"
#include <cstring>

Image applyCannyPostProcessing(const Image& mag, const Image& dir) {
    Image output;
    output.allocate(mag.width, mag.height);

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

            if (current >= neighbor1 && current >= neighbor2) {
                output.data[idx] = current;
            } else {
                output.data[idx] = 0;
            }
        }
    }
    return output;
}

Image applyDoubleThresholding(const Image& nms_img, unsigned char low_thresh, unsigned char high_thresh) {
    Image output;
    output.allocate(nms_img.width, nms_img.height);

    int total_pixels = nms_img.width * nms_img.height;

    for (int i = 0; i < total_pixels; ++i) {
        if (nms_img.data[i] >= high_thresh) {
            output.data[i] = 255; 
        } else if (nms_img.data[i] >= low_thresh) {
            output.data[i] = 128; 
        } else {
            output.data[i] = 0;   
        }
    }
    return output;
}

Image applyHysteresis(const Image& thresh_img) {
    Image output;
    output.allocate(thresh_img.width, thresh_img.height);
    
    int w = thresh_img.width;
    int h = thresh_img.height;
    int total_pixels = w * h;

    // نسخ البيانات بشكل عميق وصحيح للـ Pointer الجديد
    std::memcpy(output.data, thresh_img.data, total_pixels);

    for (int y = 1; y < h - 1; ++y) {
        for (int x = 1; x < w - 1; ++x) {
            int idx = y * w + x;
            
            if (output.data[idx] == 128) {
                if (output.data[(y-1)*w + x-1] == 255 || output.data[(y-1)*w + x] == 255 || output.data[(y-1)*w + x+1] == 255 ||
                    output.data[y*w + x-1] == 255     ||                                    output.data[y*w + x+1] == 255 ||
                    output.data[(y+1)*w + x-1] == 255 || output.data[(y+1)*w + x] == 255 || output.data[(y+1)*w + x+1] == 255) {
                    
                    output.data[idx] = 255; 
                }
            }
        }
    }

    for (int i = 0; i < total_pixels; ++i) {
        if (output.data[i] == 128) {
            output.data[i] = 0;
        }
    }

    return output;
}
