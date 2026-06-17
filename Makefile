CXX = riscv64-linux-gnu-g++
HOST_CXX = g++

CXXFLAGS = -std=c++17 -O3 -Wall -Iinclude $(RVV_FLAGS)

ifeq ($(RVV), 1)
    RVV_FLAGS = -march=rv64gcv -mabi=lp64d
else
    RVV_FLAGS =
endif

# ===== SOURCES =====
APP_SRCS = src/canny_ops.cpp \
           src/gaussian.cpp \
           src/sobel.cpp \
           src/gaussian_blur_rvv.cpp \
           src/sobel_rvv.cpp \
           src/main.cpp

APP_OBJS = $(APP_SRCS:.cpp=.o)

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
	$(CXX) $(CXXFLAGS) $^ -o $@

# ===== OBJECT RULES (RISC-V APP) =====
src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ===== TEST OBJECTS (HOST) =====
tests/%.o: tests/%.cpp
	$(HOST_CXX) $(CXXFLAGS) -c $< -o $@

# ===== TEST EXECUTION (HOST ONLY) =====
test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(filter-out src/main.o, $(APP_OBJS)) $(TEST_OBJS)
	$(HOST_CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

# ===== CLEAN =====
clean:
	rm -f src/*.o tests/*.o $(TARGET) $(TEST_TARGET)

.PHONY: all clean test
