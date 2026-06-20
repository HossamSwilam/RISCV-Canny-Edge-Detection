#include <iostream>
#include <cstdlib>
#include <cmath>
#include <cassert>
#include "image.h"
#include "processing.h"

// دالة مساعدة لمقارنة صورتين مع السماح باختلاف بسيط (Tolerance)
void assert_images_equal(const Image& img1, const Image& img2, int tolerance = 1, const char* test_name = "Unknown") {
    assert(img1.width == img2.width && "Width mismatch");
    assert(img1.height == img2.height && "Height mismatch");
    
    int size = img1.width * img1.height;
    for (int i = 0; i < size; ++i) {
        int diff = std::abs((int)img1.data[i] - (int)img2.data[i]);
        if (diff > tolerance) {
            std::cerr << "[FAILED] " << test_name << " at index " << i 
                      << ": expected " << (int)img1.data[i] 
                      << ", got " << (int)img2.data[i] 
                      << " (diff=" << diff << " > " << tolerance << ")\n";
            std::exit(1); // يوقف البرنامج فورا لو فيه اختلاف
        }
    }
    std::cout << "[PASSED] " << test_name << " (Tolerance: " << tolerance << ")\n";
}

int main() {
    std::cout << "--- Starting QEMU Equivalence Tests ---\n";

    // 1. الدليل يشترط استخدام أبعاد ليست من مضاعفات الـ 2 لاختبار الـ tail case (Strip-mining tail)
    int w = 100;
    int h = 75;
    
    Image input;
    input.allocate(w, h);
    
    // ملء الصورة بقيم عشوائية كـ Test Pattern
    for (int i = 0; i < w * h; ++i) {
        input.data[i] = std::rand() % 256;
    }

    // ==========================================
    // 2. اختبار تطابق الـ Gaussian Blur
    // ==========================================
    std::cout << "\nRunning Gaussian Blur Equivalence...\n";
    Image blur_scalar = applyGaussianBlur(input);

    // 3. اختبار تطابق الـ Sobel Operator (L1 Norm)
    // ==========================================
    std::cout << "Running Sobel (L1 Norm) Equivalence...\n";
    Image mag_scalar, dir_scalar;
    // الدليل طالب Vectorization للـ L1 Norm، فهنختبر الطريقة دي بإننا نبعت false
    applySobel(blur_scalar, mag_scalar, dir_scalar, false); 
    

    std::cout << "\nAll QEMU tests finished (RVV pending).\n";

    // تحرير الذاكرة
    input.free_memory();
    blur_scalar.free_memory();
    mag_scalar.free_memory();
    dir_scalar.free_memory();
    
    return 0;
}
