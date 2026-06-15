CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall -Iinclude

ifeq ($(RVV), 1)
    RVV_FLAGS = -march=rv64gcv -mabi=lp64d
else
    RVV_FLAGS =
endif

# ===== APP SOURCES (WITHOUT main in tests) =====
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

# ===== CLEAN =====
clean:
	rm -f src/*.o tests/*.o $(TARGET) $(TEST_TARGET)

.PHONY: all clean test