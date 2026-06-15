#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include "image.h"
#include "processing.h"

// قراءة ملف Raw (بدون header)
Image readRaw(const std::string& filename, int width, int height) {
    Image img(width, height);
    std::ifstream file(filename, std::ios::binary);
    
    if (!file) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        img.free_memory();
        return {0, 0};
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
    // الدليل طالب إن الـ Caller يحدد الـ Width والـ Height
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <input.raw> <width> <height>" << std::endl;
        return -1;
    }

    std::string inputPath = argv[1];
    int width = std::stoi(argv[2]);
    int height = std::stoi(argv[3]);

    std::cout << "--- Canny Edge Detection (Scalar Version) ---" << std::endl;

    // 1. Read Raw Image
    Image input = readRaw(inputPath, width, height);
    if (!input.data) return -1;
    std::cout << "Image loaded: " << input.width << "x" << input.height << std::endl;

    // 2. Gaussian Blur 
    std::cout << "Applying Gaussian Blur..." << std::endl;
    Image blurred = applyGaussianBlur(input);
    
    // 3. Sobel Operator 
    std::cout << "Applying Sobel Operator..." << std::endl;
    Image magnitude(width, height), direction(width, height);
    applySobel(blurred, magnitude, direction);
    
    // 4. Non-Maximum Suppression 
    std::cout << "Applying Non-Maximum Suppression..." << std::endl;
    Image nmsResult = applyCannyPostProcessing(magnitude, direction);

    // 5. Double Thresholding
    std::cout << "Applying Double Thresholding..." << std::endl;
    Image threshResult = applyDoubleThresholding(nmsResult, 20, 60);

    // 6. Hysteresis Edge Tracking
    std::cout << "Applying Hysteresis Edge Tracking..." << std::endl;
    Image finalResult = applyHysteresis(threshResult);

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
