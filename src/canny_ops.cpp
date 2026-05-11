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
