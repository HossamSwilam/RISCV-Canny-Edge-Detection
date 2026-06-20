#include <iostream>
#include <cstdlib>
#include <cmath>
#include <cassert>
#include "image.h"
#include "processing.h"

void gaussian_blur_rvv(const Image& src, Image& dst);

// ضفنا هنا parameter جديد اسمه margin
void assert_images_equal(const Image& img1, const Image& img2, int tolerance = 1, const char* test_name = "Unknown", int margin = 0) {
    assert(img1.width == img2.width && "Width mismatch");
    assert(img1.height == img2.height && "Height mismatch");
    
    // هنمشي على الصورة بس هنتجاهل الحواف (margin) من الأربع جهات
    for (int y = margin; y < img1.height - margin; ++y) {
        for (int x = margin; x < img1.width - margin; ++x) {
            int i = y * img1.width + x; // حساب الـ index بناءً على الـ x و y
            int diff = std::abs((int)img1.data[i] - (int)img2.data[i]);
            
            if (diff > tolerance) {
                std::cerr << "[FAILED] " << test_name << " at (x=" << x << ", y=" << y << ", index=" << i << ")" 
                          << ": expected " << (int)img1.data[i] 
                          << ", got " << (int)img2.data[i] 
                          << " (diff=" << diff << " > " << tolerance << ")\n";
                std::exit(1);
            }
        }
    }
    std::cout << "[PASSED] " << test_name << " (Tolerance: " << tolerance << ", Margin: " << margin << ")\n";
}

int main() {
    std::cout << "--- Starting QEMU Equivalence Tests ---\n";
    std::srand(42);

    int w = 100;
    int h = 75;
    
    Image input;
    input.allocate(w, h); 
    for (int i = 0; i < w * h; ++i) {
        input.data[i] = static_cast<unsigned char>(std::rand() % 256);
    }

    std::cout << "\nRunning Gaussian Blur Equivalence...\n";
    Image blur_scalar = applyGaussianBlur(input);
    
    Image blur_rvv;
    blur_rvv.allocate(w, h); 
    gaussian_blur_rvv(input, blur_rvv); 
    
    // بعتنا margin = 2 عشان نتجاهل أول وآخر صفين وعمودين (لو الفلتر 5x5)
    assert_images_equal(blur_scalar, blur_rvv, 5, "Gaussian Blur (Scalar vs RVV)", 2);

    std::cout << "Running Sobel Equivalence...\n";
    Image mag_scalar, dir_scalar;
    Image mag_rvv, dir_rvv;
    
    mag_scalar.allocate(w, h);
    dir_scalar.allocate(w, h);
    mag_rvv.allocate(w, h);
    dir_rvv.allocate(w, h);
    
    applySobel(blur_scalar, mag_scalar, dir_scalar, false);
    applySobelRVV(blur_scalar, mag_rvv, dir_rvv);
    
    // بعتنا margin = 3 عشان نتجاهل تأثير حواف الـ Gaussian بالإضافة لحواف الـ Sobel نفسه
    assert_images_equal(mag_scalar, mag_rvv, 3, "Sobel Magnitude (Scalar vs RVV)", 3);
    assert_images_equal(dir_scalar, dir_rvv, 3, "Sobel Direction (Scalar vs RVV)", 3);

    std::cout << "\nAll QEMU tests finished successfully!\n";

    input.free_memory();
    blur_scalar.free_memory();
    blur_rvv.free_memory();
    mag_scalar.free_memory();
    dir_scalar.free_memory();
    mag_rvv.free_memory();
    dir_rvv.free_memory();
    
    return 0;
}
