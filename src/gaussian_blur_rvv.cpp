#include <cstdint>

void gaussian_blur_rvv(const unsigned char* input,
                        unsigned char* output,
                        int width,
                        int height)
{
    // Gaussian kernel:
    // 1 2 1
    // 2 4 2   /16
    // 1 2 1

    for (int y = 1; y < height - 1; y++)
    {
        for (int x = 1; x < width - 1; x++)
        {
            int idx = y * width + x;

            int sum =
                input[(y - 1) * width + (x - 1)] * 1 +
                input[(y - 1) * width + (x)]     * 2 +
                input[(y - 1) * width + (x + 1)] * 1 +

                input[(y) * width + (x - 1)]     * 2 +
                input[(y) * width + (x)]         * 4 +
                input[(y) * width + (x + 1)]     * 2 +

                input[(y + 1) * width + (x - 1)] * 1 +
                input[(y + 1) * width + (x)]     * 2 +
                input[(y + 1) * width + (x + 1)] * 1;

            output[idx] = (unsigned char)(sum >> 4); // divide by 16
        }
    }

    // handle borders (simple copy)
    for (int x = 0; x < width; x++)
    {
        output[x] = input[x];
        output[(height - 1) * width + x] = input[(height - 1) * width + x];
    }

    for (int y = 0; y < height; y++)
    {
        output[y * width] = input[y * width];
        output[y * width + (width - 1)] = input[y * width + (width - 1)];
    }
}
