CXX = g++
CXXFLAGS = -Iinclude -Wall

SRCS = src/main.cpp src/gaussian.cpp src/sobel.cpp src/canny_ops.cpp
TARGET = canny_scalar

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f 	$(TARGET)
