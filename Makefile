# اسم الـ Compiler (تأكد إنه يدعم RISC-V GCV لو بتعمل Cross-Compile)
CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall -Iinclude

# الفلاجز الخاصة بتفعيل الـ RISC-V Vector Extensions (RVV)
# ملحوظة: لو بتعمل compile محلي على الـ Host للتجربة ممكن تحتاج تشيلها أو تخليها اختيارية
RVV_FLAGS = -march=rv64gcv -mabi=lp64d

# الملفات المصدرية (Scalar + RVV الإضافية)
SRCS = src/processing.cpp \
       src/gaussian_blur_rvv.cpp \
       src/sobel_rvv.cpp

OBJS = $(SRCS:.cpp=.o)

# ملفات الاختبارات
TEST_SRCS = tests/test_gaussian.cpp \
            tests/test_sobel.cpp \
            tests/test_canny.cpp

TEST_OBJS = $(TEST_SRCS:.cpp=.o)

# المكتبات المطلوبة للـ Google Test
LIBS = -lgtest -lgtest_main -lpthread

TARGET = canny_edge_detection
TEST_TARGET = run_tests

all: $(TARGET)

$(TARGET): main.cpp $(OBJS)
$(CXX) $(CXXFLAGS) main.cpp $(OBJS) -o $(TARGET)

# قاعدة مخصصة لملفات الـ RVV عشان تاخد الفلاجز بتاعة الفيكتورز
src/%_rvv.o: src/%_rvv.cpp
$(CXX) $(CXXFLAGS) $(RVV_FLAGS) -c $< -o $@

src/%.o: src/%.cpp
$(CXX) $(CXXFLAGS) -c $< -o $@

tests/%.o: tests/%.cpp
$(CXX) $(CXXFLAGS) -c $< -o $@

test: $(TEST_TARGET)
./$(TEST_TARGET)

$(TEST_TARGET): $(OBJS) $(TEST_OBJS)
$(CXX) $(CXXFLAGS) $(OBJS) $(TEST_OBJS) -o $(TEST_TARGET) $(LIBS)

clean:
rm -f src/*.o tests/*.o $(TARGET) $(TEST_TARGET)

.PHONY: all clean test
