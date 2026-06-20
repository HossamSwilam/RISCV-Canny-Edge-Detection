#pragma once

#include <cstdlib>   // aligned_alloc, free
#include <cstring>   // memcpy

// Simple grayscale image container.
// Pixels are stored as 8-bit values in row-major order.
struct Image {
    int width;
    int height;
    unsigned char* data;

    // Create an empty image or allocate storage for w × h pixels.
    Image(int w = 0, int h = 0)
        : width(w), height(h), data(nullptr) {
        if (w > 0 && h > 0) {
            data = static_cast<unsigned char*>(
                aligned_alloc(64, w * h)
            );
        }
    }

    // Release owned memory.
    ~Image() {
        free_memory();
    }

    // Deep-copy constructor.
    Image(const Image& other)
        : width(other.width), height(other.height), data(nullptr) {
        if (other.data && width > 0 && height > 0) {
            data = static_cast<unsigned char*>(
                aligned_alloc(64, width * height)
            );
            std::memcpy(data, other.data, width * height);
        }
    }

    // Deep-copy assignment operator.
    Image& operator=(const Image& other) {
        if (this != &other) {
            free_memory();

            width  = other.width;
            height = other.height;

            if (other.data && width > 0 && height > 0) {
                data = static_cast<unsigned char*>(
                    aligned_alloc(64, width * height)
                );
                std::memcpy(data, other.data, width * height);
            } else {
                data = nullptr;
            }
        }
        return *this;
    }

    // Allocate or reallocate image storage.
    void allocate(int w, int h) {
        free_memory();

        width  = w;
        height = h;

        if (w > 0 && h > 0) {
            data = static_cast<unsigned char*>(
                aligned_alloc(64, w * h)
            );
        }
    }

    // Pixel access (modifiable).
    inline unsigned char& at(int x, int y) {
        return data[y * width + x];
    }

    // Pixel access (read-only).
    inline const unsigned char& at(int x, int y) const {
        return data[y * width + x];
    }

    // Free allocated memory safely.
    void free_memory() {
        if (data) {
            std::free(data);
            data = nullptr;
        }
    }
};