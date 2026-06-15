#include <gtest/gtest.h>

int main(int argc, char **argv) {
    // 1. تهيئة مكتبة GoogleTest وقراءة أي شهادات أو آرغومنتس من الـ Command Line
    ::testing::InitGoogleTest(&argc, argv);
    
    // 2. تشغيل كل الـ TESTs المكتوبة في كل الملفات المرتبطة (Linked) مع الملف ده
    return RUN_ALL_TESTS();
}
