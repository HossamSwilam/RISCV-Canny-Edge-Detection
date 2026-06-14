CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall -Iinclude

# تحديد إذا كان الـ Compiler يدعم RISC-V أو نترجم كود عادي للتجربة
# لو عاوز تفعل الـ RVV مجبر اكتب في الترمينال: make RVV=1
ifeq ($(RVV), 1)
    RVV_FLAGS = -march=rv64gcv -mabi=lp64d
else
    RVV_FLAGS =
endif

SRCS = src/canny_ops.cpp \
       src/gaussian.cpp \
       src/sobel.cpp \
       src/gaussian_blur_rvv.cpp \
       src/sobel_rvv.cpp \
       src/main.cpp

OBJS = $(SRCS:.cpp=.o)

TEST_SRCS = tests/test_gaussian.cpp \
            tests/test_sobel.cpp \
            tests/test_canny.cpp

TEST_OBJS = $(TEST_SRCS:.cpp=.o)

LIBS = -lgtest -lgtest_main -lpthread
TARGET = canny_edge_detection
TEST_TARGET = run_tests

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

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
