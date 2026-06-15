#pragma once
#include <cstdlib>

struct Image {
    int width;
    int height;
    unsigned char* data; // Pointer بدل std::vector

    // Constructor عشان نـ allocate الـ memory صح
    Image(int w = 0, int h = 0) : width(w), height(h), data(nullptr) {
        if (w > 0 && h > 0) {
            // حجز ذاكرة محاذية لـ 64 بايت
            data = static_cast<unsigned char*>(aligned_alloc(64, w * h));
        }
    }
void allocate(int w, int h) {
    free_memory();

    width = w;
    height = h;

    if (w > 0 && h > 0) {
        data = static_cast<unsigned char*>(aligned_alloc(64, w * h));
    }
}
    // عشان ننضف الـ Memory (Memory Management)
    void free_memory() {
        if (data != nullptr) {
            std::free(data);
            data = nullptr;
        }
    }
};
