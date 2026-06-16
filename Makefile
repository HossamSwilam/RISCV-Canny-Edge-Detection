# Default Compiler is native (x86)
CXX ?= g++
OPT ?= -O3

# 🌟 التعديل الأول: تفعيل الـ Cross-Compiler أوتوماتيك لو RVV=1
ifeq ($(RVV), 1)
    CXX = riscv64-linux-gnu-g++
    # ضفنا -D__riscv_vector عشان نفعل الـ #ifdef في الكود
    RVV_FLAGS = -march=rv64gcv -mabi=lp64d -D__riscv_vector
else
    RVV_FLAGS =
endif

# 🌟 التعديل التاني: دمج الـ RVV_FLAGS جوه الـ CXXFLAGS عشان الكومبايلر يشوفها
CXXFLAGS = -std=c++17 $(OPT) -Wall -Iinclude $(RVV_FLAGS)

# ===== APP SOURCES =====
APP_SRCS = src/canny_ops.cpp \
           src/gaussian.cpp \
           src/sobel.cpp \
           src/gaussian_blur_rvv.cpp \
           src/sobel_rvv.cpp \
           src/main.cpp

APP_OBJS = $(APP_SRCS:.cpp=.o)

# ===== TEST SOURCES =====
TEST_SRCS = tests/test_gaussian.cpp \
            tests/test_sobel.cpp \
            tests/test_canny.cpp

TEST_OBJS = $(TEST_SRCS:.cpp=.o)

LIBS = -lgtest -lgtest_main -lpthread

TARGET = canny_edge_detection
TEST_TARGET = run_tests

# ===== BUILD APP =====
all: $(TARGET)

$(TARGET): $(APP_OBJS)
	$(CXX) $(CXXFLAGS) $(APP_OBJS) -o $(TARGET)

# ===== OBJECT RULES =====
src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

tests/%.o: tests/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ===== TESTS =====
test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(filter-out src/main.o, $(APP_OBJS)) $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $(TEST_TARGET) $(LIBS)

# 🌟 التعديل التالت: أمر جديد لتشغيل المحاكي QEMU مع تفعيل الفيكتور (VLEN=128)
run-rvv: $(TARGET)
	qemu-riscv64 -L /usr/riscv64-linux-gnu -cpu rv64,v=true,vlen=128 ./$(TARGET) input.raw 512 512

# ===== CLEAN =====
clean:
	rm -f src/*.o tests/*.o $(TARGET) $(TEST_TARGET)

.PHONY: all clean test run-rvv
