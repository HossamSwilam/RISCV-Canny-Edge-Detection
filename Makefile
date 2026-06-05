CXX = g++
CXXFLAGS = -Iinclude -I/usr/src/gtest/include -I/usr/src/gtest -Wall -g

SRCS = src/main.cpp src/gaussian.cpp src/sobel.cpp src/canny_ops.cpp
TARGET = canny_scalar

# تجميع السورس كود الخاص بجوجل تست مباشرة مع التست لضمان توافق الـ ABI
TEST_SRCS = /usr/src/gtest/src/gtest-all.cc /usr/src/gtest/src/gtest_main.cc tests/test_gaussian.cpp tests/test_sobel.cpp tests/test_canny.cpp src/gaussian.cpp src/sobel.cpp src/canny_ops.cpp
TEST_TARGET = run_tests
TEST_LIBS = -pthread

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_SRCS)
	$(CXX) $(CXXFLAGS) $(TEST_SRCS) -o $(TEST_TARGET) $(TEST_LIBS)

clean:
	rm -f $(TARGET) $(TEST_TARGET)
