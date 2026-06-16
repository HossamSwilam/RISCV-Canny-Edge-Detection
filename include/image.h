#pragma once
#include <cstdlib>

struct Image {
    int width;
    int height;
    unsigned char* data; // Pointer بدل std::vector

    // Constructor 
    Image(int w = 0, int h = 0) : width(w), height(h), data(nullptr) {
        if (w > 0 && h > 0) {
            data = static_cast<unsigned char*>(aligned_alloc(64, w * h));
        }
    }

    // 🌟 الدالة السحرية الجديدة اللي كانت ناقصة لفك الـ Error
    void allocate(int w, int h) {
        free_memory(); // لضمان عدم حدوث Memory Leak لو كان محجوز له ذاكرة قبل كده
        width = w;
        height = h;
        if (w > 0 && h > 0) {
            data = static_cast<unsigned char*>(aligned_alloc(64, w * h));
        }
    }

    // عشان ننضف الـ Memory
    void free_memory() {
        if (data != nullptr) {
            std::free(data);
            data = nullptr;
        }
    }
};
