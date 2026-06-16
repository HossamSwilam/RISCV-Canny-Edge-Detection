#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <chrono> // 1. إضافة مكتبة حساب الوقت
#include "image.h"
#include "processing.h"

// قراءة ملف Raw (بدون header) باستخدام الـ Aligned Allocation الجديد
Image readRaw(const std::string& filename, int width, int height) {
    Image img;
    img.allocate(width, height); // تخصيص الذاكرة المحاذية لـ 64 بايت لرفع أداء الـ SIMD
    
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        img.free_memory();
        return img; // هيرجع كائن فارغ والـ data جواه بـ nullptr
    }

    file.read(reinterpret_cast<char*>(img.data), width * height);
    return img;
}

// حفظ ملف Raw
bool writeRaw(const std::string& filename, const Image& img) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) return false;

    file.write(reinterpret_cast<const char*>(img.data), img.width * img.height);
    return true;
}

int main(int argc, char* argv[]) {
    // الدليل يشترط أن يقوم الـ Caller بتحديد الـ Width والـ Height عبر الـ Command Line
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <input.raw> <width> <height>" << std::endl;
        return -1;
    }

    std::string inputPath = argv[1];
    int width = std::stoi(argv[2]);
    int height = std::stoi(argv[3]);

    std::cout << "--- Canny Edge Detection (Scalar Version with Profiling) ---" << std::endl;

    // 1. Read Raw Image
    Image input = readRaw(inputPath, width, height);
    if (!input.data) return -1;
    std::cout << "Image loaded: " << input.width << "x" << input.height << std::endl;

    // =========================================================================
    // 2. Gaussian Blur 
    // =========================================================================
    std::cout << "Applying Gaussian Blur..." << std::endl;
    auto start_gaussian = std::chrono::high_resolution_clock::now();
    
    Image blurred = applyGaussianBlur(input);
    
    auto end_gaussian = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration_gaussian = end_gaussian - start_gaussian;

    // =========================================================================
    // 3. Sobel Operator 
    // =========================================================================
    std::cout << "Applying Sobel Operator..." << std::endl;
    Image magnitude, direction;
    bool use_l2 = true; 
    
    auto start_sobel = std::chrono::high_resolution_clock::now();
    
    applySobel(blurred, magnitude, direction, use_l2);
    
    auto end_sobel = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration_sobel = end_sobel - start_sobel;

    // =========================================================================
    // 4. Non-Maximum Suppression 
    // =========================================================================
    std::cout << "Applying Non-Maximum Suppression..." << std::endl;
    auto start_nms = std::chrono::high_resolution_clock::now();
    
    Image nmsResult = applyCannyPostProcessing(magnitude, direction);
    
    auto end_nms = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration_nms = end_nms - start_nms;

    // =========================================================================
    // 5. Double Thresholding
    // =========================================================================
    std::cout << "Applying Double Thresholding..." << std::endl;
    auto start_thresh = std::chrono::high_resolution_clock::now();
    
    Image threshResult = applyDoubleThresholding(nmsResult, 20, 60);
    
    auto end_thresh = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration_thresh = end_thresh - start_thresh;

    // =========================================================================
    // 6. Hysteresis Edge Tracking
    // =========================================================================
    std::cout << "Applying Hysteresis Edge Tracking..." << std::endl;
    auto start_hysteresis = std::chrono::high_resolution_clock::now();
    
    Image finalResult = applyHysteresis(threshResult);
    
    auto end_hysteresis = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration_hysteresis = end_hysteresis - start_hysteresis;

    // =========================================================================
    // 📊 طبع تقرير الأداء (Performance Report)
    // =========================================================================
    double total_time = duration_gaussian.count() + duration_sobel.count() + 
                        duration_nms.count() + duration_thresh.count() + 
                        duration_hysteresis.count();

    std::cout << "\n================ PERFORMANCE REPORT ================\n";
    std::cout << "1. Gaussian Blur      : " << duration_gaussian.count()   << " ms\n";
    std::cout << "2. Sobel Operator      : " << duration_sobel.count()      << " ms\n";
    std::cout << "3. Non-Max Suppression : " << duration_nms.count()        << " ms\n";
    std::cout << "4. Double Threshold    : " << duration_thresh.count()     << " ms\n";
    std::cout << "5. Hysteresis Tracking : " << duration_hysteresis.count() << " ms\n";
    std::cout << "----------------------------------------------------\n";
    std::cout << "Total Pipeline Time    : " << total_time                  << " ms\n";
    std::cout << "====================================================\n\n";

    // 7. Save the final result as RAW
    if (writeRaw("output.raw", finalResult)) {
        std::cout << "Success! Output saved as output.raw" << std::endl;
    } else {
        std::cerr << "Error saving output file." << std::endl;
    }

    // تحرير الذاكرة
    input.free_memory();
    blurred.free_memory();
    magnitude.free_memory();
    direction.free_memory();
    nmsResult.free_memory();
    threshResult.free_memory();
    finalResult.free_memory();

    return 0;
}
