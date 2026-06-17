CXX = riscv64-linux-gnu-g++
HOST_CXX = g++

CXXFLAGS = -std=c++17 -O3 -Wall -Iinclude -march=rv64gcv -mabi=lp64d

APP_SRCS = \
    src/canny_ops.cpp \
    src/gaussian.cpp \
    src/sobel.cpp \
    src/main.cpp

APP_OBJS = $(APP_SRCS:.cpp=.o)

TARGET = canny_edge_detection

all: $(TARGET)

$(TARGET): $(APP_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

TEST_TARGET = run_tests
LIBS = -lgtest -lgtest_main -lpthread

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET):
	$(HOST_CXX) -std=c++17 -O3 -Wall -Iinclude \
	tests/test_gaussian.cpp \
	tests/test_sobel.cpp \
	tests/test_canny.cpp \
	src/canny_ops.cpp \
	src/gaussian.cpp \
	src/sobel.cpp \
	-o $(TEST_TARGET) $(LIBS)

clean:
	rm -f src/*.o $(TARGET) $(TEST_TARGET)

.PHONY: all test clean
