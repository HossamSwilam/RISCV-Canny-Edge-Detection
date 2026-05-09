#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "image.h"
#include "processing.h"

// دالة قراءة صور PGM (بصيغة P5 Binary)
Image readPGM(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return {0, 0, {}};
    }

    std::string magic;
    int width, height, maxVal;
    file >> magic >> width >> height >> maxVal;
    file.ignore(); // تخطي السطر الجديد بعد Header

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

// دالة حفظ الصور بصيغة PGM
bool writePGM(const std::string& filename, const Image& img) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) return false;

    file << "P5\n" << img.width << " " << img.height << "\n255\n";
    file.write(reinterpret_cast<const char*>(img.data.data()), img.data.size());
    
    return true;
}

int main() {
    std::cout << "--- Canny Edge Detection (Scalar Version) ---" << std::endl;

    // 1. قراءة الصورة (لازم يكون عندك ملف اسمه input.pgm عشان يشتغل)
    std::string inputPath = "input.pgm";
    Image input = readPGM(inputPath);
    
    if (input.data.empty()) {
        std::cout << "Please make sure 'input.pgm' exists in the project folder." << std::endl;
        return -1;
    }

    std::cout << "Image loaded: " << input.width << "x" << input.height << std::endl;

    // 2. المرحلة الأولى: Gaussian Blur (شغل المبرمج 1)
    std::cout << "Applying Gaussian Blur..." << std::endl;
    Image blurred = applyGaussianBlur(input);
    
    // 3. المرحلة الثانية: Sobel Operator (شغل المبرمج 2)
    std::cout << "Applying Sobel Operator..." << std::endl;
    Image magnitude, direction;
    applySobel(blurred, magnitude, direction);
    
    // 4. المرحلة الثالثة والرابعة: Post-Processing (شغل المبرمج 3 و 4)
    std::cout << "Applying Canny Post-Processing..." << std::endl;
    Image finalResult = applyCannyPostProcessing(magnitude, direction);

    // 5. حفظ النتيجة النهائية
    if (writePGM("output.pgm", finalResult)) {
        std::cout << "Success! Output saved as output.pgm" << std::endl;
    } else {
        std::cerr << "Error saving output file." << std::endl;
    }

    return 0;
}
