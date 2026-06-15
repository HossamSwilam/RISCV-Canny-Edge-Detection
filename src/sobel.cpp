#include "processing.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdint>

void applySobel(const Image& input, Image& mag, Image& dir) {
    int w = input.width;
    int h = input.height;
    
    mag.width = w;
    mag.height = h;
    mag.data.assign(w * h, 0); // تهيئة بأصفار

    dir.width = w;
    dir.height = h;
    dir.data.assign(w * h, 0); // تهيئة بأصفار

    // 1. استخدام Structure of Arrays (SoA) وتخزين Gx و Gy بشكل منفصل
    // نوع int16_t مهم جداً عشان مرحلة الـ RVV Vectorization
    std::vector<int16_t> gx_array(w * h, 0);
    std::vector<int16_t> gy_array(w * h, 0);
    
    // مصفوفة مؤقتة لتخزين الـ Magnitude قبل الـ Normalization
    std::vector<int32_t> raw_mag(w * h, 0);

    int max_mag = 0;

    // --- Pass 1: حساب الـ Gradients والاتجاهات وأكبر Magnitude ---
    for (int y = 1; y < h - 1; ++y) {
        for (int x = 1; x < w - 1; ++x) {
            int idx = y * w + x;
            
            // حساب Sobel X
            int gx = -1 * input.data[(y - 1) * w + (x - 1)] 
                     +1 * input.data[(y - 1) * w + (x + 1)]
                     -2 * input.data[y * w + (x - 1)] 
                     +2 * input.data[y * w + (x + 1)]
                     -1 * input.data[(y + 1) * w + (x - 1)] 
                     +1 * input.data[(y + 1) * w + (x + 1)];

            // حساب Sobel Y
            int gy = -1 * input.data[(y - 1) * w + (x - 1)] 
                     -2 * input.data[(y - 1) * w + x] 
                     -1 * input.data[(y - 1) * w + (x + 1)]
                     +1 * input.data[(y + 1) * w + (x - 1)] 
                     +2 * input.data[(y + 1) * w + x] 
                     +1 * input.data[(y + 1) * w + (x + 1)];

            // تخزين القيم في الـ Arrays (Phase 6 هتحتاج ده جداً)
            gx_array[idx] = static_cast<int16_t>(gx);
            gy_array[idx] = static_cast<int16_t>(gy);

            // 2. حساب الـ Magnitude (بدون Clamp في اللفة دي)
            int magnitude = std::round(std::sqrt(gx * gx + gy * gy));
            raw_mag[idx] = magnitude;

            // تحديث أكبر قيمة عشان الـ Normalization
            if (magnitude > max_mag) {
                max_mag = magnitude;
            }

            // 3. حساب الاتجاهات بدون atan2 وبدون أرقام كسرية (Integer Cross-Multiplication)
            
            int ax = std::abs(gx);
            int ay = std::abs(gy);
            unsigned char direction = 0;

            if (5 * ay < 2 * ax) {
                direction = 0;   // زاوية قريبة من 0
            } else if (5 * ay > 12 * ax) {
                direction = 90;  // زاوية قريبة من 90
            } else {
                // الزوايا القطرية (الـ Diagonal)
                // في إحداثيات الصور الـ Y بتزيد لتحت، فالإشارات بتحدد الاتجاه
                if (gx * gy > 0) {
                    direction = 135; 
                } else {
                    direction = 45;
                }
            }
            dir.data[idx] = direction;
        }
    }

    // --- Pass 2: عمل Normalization للـ Magnitude عشان يكون بين [0, 255] ---
    if (max_mag == 0) max_mag = 1; // حماية عشان مانقسمش على صفر لو الصورة كلها سودة

    for (int y = 1; y < h - 1; ++y) {
        for (int x = 1; x < w - 1; ++x) {
            int idx = y * w + x;
            // التحويل لـ 0-255 باستخدام ضرب وقسمة أرقام صحيحة (Integer Math)
            int normalized_mag = (raw_mag[idx] * 255) / max_mag;
            mag.data[idx] = static_cast<unsigned char>(normalized_mag);
        }
    }
}
