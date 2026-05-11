#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "image.h"
#include "processing.h"

// Function to read PGM images (P5 Binary format)
Image readPGM(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return {0, 0, {}};
    }

    std::string magic;
    int width, height, maxVal;
    file >> magic >> width >> height >> maxVal;
    file.ignore(); // Skip the newline character after the header

    if (magic != "P5") {
        std::cerr << "Error: Only P5 PGM files are supported!" << std::endl;
        return {0, 0, {}};
    }

    Image img;
    img.width = width;
    img.height = height;
    img.data.resize(width * height);
    file.read(reinterpret_cast<char*>(img.data.data()), width * height);
    
    return img;
}

// Function to save images in PGM format
bool writePGM(const std::string& filename, const Image& img) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) return false;

    file << "P5\n" << img.width << " " << img.height << "\n255\n";
    file.write(reinterpret_cast<const char*>(img.data.data()), img.data.size());
    
    return true;
}

int main() {
    std::cout << "--- Canny Edge Detection (Scalar Version) ---" << std::endl;

    // 1. Read the image (Ensure 'input.pgm' exists in the project folder)
    std::string inputPath = "input.pgm";
    Image input = readPGM(inputPath);
    
    if (input.data.empty()) {
        std::cout << "Please make sure 'input.pgm' exists in the project folder." << std::endl;
        return -1;
    }

    std::cout << "Image loaded: " << input.width << "x" << input.height << std::endl;

    // 2. Stage 1: Gaussian Blur 
    std::cout << "Applying Gaussian Blur..." << std::endl;
    Image blurred = applyGaussianBlur(input);
    
    // 3. Stage 2: Sobel Operator 
    std::cout << "Applying Sobel Operator..." << std::endl;
    Image magnitude, direction;
    applySobel(blurred, magnitude, direction);
    
    // 4. Stage 3: Non-Maximum Suppression 
    std::cout << "Applying Non-Maximum Suppression..." << std::endl;
    Image nmsResult = applyCannyPostProcessing(magnitude, direction);

    // 5. Stage 4: Double Thresholding (Your Task)
    std::cout << "Applying Double Thresholding..." << std::endl;
    // Using experimental values for thresholds (Low: 20, High: 60)
    Image threshResult = applyDoubleThresholding(nmsResult, 20, 60);

    // 6. Stage 5: Hysteresis Edge Tracking (Your Task)
    std::cout << "Applying Hysteresis Edge Tracking..." << std::endl;
    Image finalResult = applyHysteresis(threshResult);

    // 7. Save the final result
    if (writePGM("output.pgm", finalResult)) {
        std::cout << "Success! Output saved as output.pgm" << std::endl;
    } else {
        std::cerr << "Error saving output file." << std::endl;
    }

    return 0;
}