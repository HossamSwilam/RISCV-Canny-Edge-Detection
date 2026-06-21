#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <chrono>
#include "image.h"
#include "processing.h"

// تعريف دوال الـ RVV عشان نقدر نستخدمها لو الـ Flag متفعل
#ifdef __riscv_vector
extern void gaussian_blur_rvv(const Image& src, Image& dst);
extern void applySobelRVV(const Image& blurred, Image& magnitude, Image& direction);
#endif

Image readRaw(const std::string& filename, int width, int height) {
    Image img;
    img.allocate(width, height);
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        img.free_memory();
        return img;
    }
    file.read(reinterpret_cast<char*>(img.data), width * height);
    return img;
}

bool writeRaw(const std::string& filename, const Image& img) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) return false;
    file.write(reinterpret_cast<const char*>(img.data), img.width * img.height);
    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <input.raw> <width> <height>" << std::endl;
        return -1;
    }

    std::string inputPath = argv[1];
    int width = std::stoi(argv[2]);
    int height = std::stoi(argv[3]);

    std::cout << "--- Canny Edge Detection Profiling ---" << std::endl;

    Image input = readRaw(inputPath, width, height);
    if (!input.data) return -1;
    std::cout << "Image loaded: " << input.width << "x" << input.height << std::endl;

    // هنسيب الصور بدون allocate، الدوال هي اللي هتعملهم التخصيص المناسب لحجمهم
    Image blurred, magnitude, direction;

    // تم رفعها من 10 إلى 40 — حل وسط بين استقرار القياس (QEMU مش
    // cycle-accurate فالقياس المفرد فيه noise) وبين وقت تنفيذ السويب
    // الكامل، خصوصًا إن -O0 لوحدها بطيئة جدًا (~800ms/iteration للـ
    // Gaussian) وكانت بتاخد وقت طويل جدًا عند 100 iteration.
    int NUM_ITERATIONS = 10;
    double total_gaussian_time = 0;
    double total_sep_time = 0; // تم إضافة متغير لحساب وقت الدالة بتاعتك
    double total_sobel_time = 0;

    std::cout << "Running Benchmarks (" << NUM_ITERATIONS << " iterations)..." << std::endl;

#ifndef __riscv_vector
    // ================== SCALAR BENCHMARK ==================
    std::cout << "[Mode] SCALAR\n";
    
    // 1. Gaussian Blur (2D)
    auto start_g = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < NUM_ITERATIONS; i++) {
        if (i == NUM_ITERATIONS - 1) {
            blurred = applyGaussianBlur(input); // نحتفظ بآخر نسخة فقط
        } else {
            Image temp = applyGaussianBlur(input);
            temp.free_memory();
        }
    }
    auto end_g = std::chrono::high_resolution_clock::now();
    total_gaussian_time = std::chrono::duration<double, std::milli>(end_g - start_g).count() / NUM_ITERATIONS;

    // 1.5 Separable Gaussian (للتجربة والتقرير فقط - الإضافة بتاعتك)
    auto start_sep = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < NUM_ITERATIONS; i++) {
        Image temp = gaussianSeparable(input);
        temp.free_memory();
    }
    auto end_sep = std::chrono::high_resolution_clock::now();
    total_sep_time = std::chrono::duration<double, std::milli>(end_sep - start_sep).count() / NUM_ITERATIONS;

    // 2. Sobel Operator
    auto start_s = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < NUM_ITERATIONS; i++) {
        if (i == NUM_ITERATIONS - 1) {
            applySobel(blurred, magnitude, direction, true); // نحتفظ بآخر نسخة
        } else {
            Image temp_mag, temp_dir;
            applySobel(blurred, temp_mag, temp_dir, true);
            temp_mag.free_memory();
            temp_dir.free_memory();
        }
    }
    auto end_s = std::chrono::high_resolution_clock::now();
    total_sobel_time = std::chrono::duration<double, std::milli>(end_s - start_s).count() / NUM_ITERATIONS;

#else
    // ================== RVV BENCHMARK ==================
    std::cout << "[Mode] RISC-V VECTOR (RVV)\n";
    // يتم تخصيص الذاكرة هنا قبل الـ RVV Loops لو دوال الـ RVV لا تقوم بذلك أوتوماتيكياً
    blurred.allocate(width, height);
    magnitude.allocate(width, height);
    direction.allocate(width, height);

    auto start_g = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < NUM_ITERATIONS; i++) {
        gaussian_blur_rvv(input, blurred);
    }
    auto end_g = std::chrono::high_resolution_clock::now();
    total_gaussian_time = std::chrono::duration<double, std::milli>(end_g - start_g).count() / NUM_ITERATIONS;

    auto start_s = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < NUM_ITERATIONS; i++) {
        applySobelRVV(blurred, magnitude, direction);
    }
    auto end_s = std::chrono::high_resolution_clock::now();
    total_sobel_time = std::chrono::duration<double, std::milli>(end_s - start_s).count() / NUM_ITERATIONS;
#endif

    // ================== PERFORMANCE REPORT ==================
    std::cout << "\n================ PERFORMANCE REPORT ================\n";
    std::cout << "1. Gaussian Blur (Avg) : " << total_gaussian_time << " ms\n";
#ifndef __riscv_vector
    std::cout << "   -> Separable (Test) : " << total_sep_time << " ms\n"; 
#endif
    std::cout << "2. Sobel Op (Avg)      : " << total_sobel_time << " ms\n";
    std::cout << "====================================================\n\n";

    // --- حفظ الصور بعد كل مرحلة بأمان ---
    std::cout << "Saving intermediate images..." << std::endl;
    if(input.data) { std::cout << "-> Saving input...\n"; writeRaw("0_input.raw", input); }
    if(blurred.data) { std::cout << "-> Saving blurred...\n"; writeRaw("1_blurred.raw", blurred); }
    if(magnitude.data) { std::cout << "-> Saving magnitude...\n"; writeRaw("2_magnitude.raw", magnitude); }
    
    if(direction.data) {
        std::cout << "-> Saving direction...\n";
        Image vis_direction;
        // استخدام الأبعاد الفعلية للصورة الناتجة لتجنب قراءة خارج الذاكرة
        vis_direction.allocate(direction.width, direction.height);
        for(int i = 0; i < direction.width * direction.height; i++) {
            vis_direction.data[i] = direction.data[i] * 50; 
        }
        writeRaw("3_direction.raw", vis_direction);
        vis_direction.free_memory();
    }

    // =========================================================
    // 👇 استخراج الصورة النهائية (NMS & Thresholding) 👇
    // =========================================================
    std::cout << "Applying Final Canny Post-Processing (NMS & Thresholding)..." << std::endl;
    Image final_edges = applyCannyPostProcessing(magnitude, direction);
    if(final_edges.data) {
        std::cout << "-> Saving final edges...\n";
        writeRaw("4_final_edges.raw", final_edges);
        final_edges.free_memory(); // تحرير ذاكرة الصورة النهائية بعد حفظها
    }
    std::cout << "Images saved successfully!" << std::endl;
    // =========================================================

    // تحرير الذاكرة الأساسية
    input.free_memory();
    blurred.free_memory();
    magnitude.free_memory();
    direction.free_memory();

    return 0;
}
