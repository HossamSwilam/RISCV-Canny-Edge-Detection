# =============================================================================
# Canny Edge Detection — RISC-V Vector Extension Project
# =============================================================================
#
# Common usage:
#   make test              build & run GoogleTest suite on the host (x86/ARM)
#   make canny_rv          cross-compile the pipeline for RISC-V
#   make run               run the RISC-V binary on QEMU (default VLEN=128)
#   make clean             remove all generated files
#
# Phase 4 compiler sweep (run each and record time + binary size):
#   make canny_rv OPT=-O0
#   make canny_rv OPT=-O2
#   make canny_rv OPT=-O3
#   make canny_rv OPT=-Os
#   make canny_rv OPT=-Ofast
#
# VLEN sweep (Phase 6):
#   make run VLEN=128
#   make run VLEN=256
#   make run VLEN=512
# =============================================================================

# ---------------------------------------------------------------------------
# Toolchain selection
# Default: native g++ for host-side testing (GoogleTest, fast iteration).
# Set RVV=1 to switch to the RISC-V cross-compiler for QEMU execution.
# ---------------------------------------------------------------------------
CXX  ?= g++
OPT  ?= -O3
VLEN ?= 128

ifeq ($(RVV), 1)
    CXX       = riscv64-linux-gnu-g++
    RVV_FLAGS = -march=rv64gcv -mabi=lp64d -D__riscv_vector
else
    RVV_FLAGS =
endif

CXXFLAGS = -std=c++17 $(OPT) -Wall -Iinclude $(RVV_FLAGS)

# ---------------------------------------------------------------------------
# Source files
# RVV files are guarded with #ifdef __riscv_vector inside each .cpp,
# so they compile safely on the host (they become empty stubs).
# ---------------------------------------------------------------------------
APP_SRCS = src/canny_ops.cpp       \
           src/gaussian.cpp         \
           src/sobel.cpp            \
           src/gaussian_blur_rvv.cpp \
           src/sobel_rvv.cpp        \
           src/main.cpp

APP_OBJS = $(APP_SRCS:.cpp=.o)

# test_Qemu.cpp contains assert-based equivalence tests (scalar vs RVV).
# It is compiled natively on the host; the RVV path is guarded with #ifdef.
TEST_SRCS = tests/test_gaussian.cpp \
            tests/test_sobel.cpp    \
            tests/test_Qemu.cpp

TEST_OBJS = $(TEST_SRCS:.cpp=.o)

LIBS = -lgtest -lgtest_main -lpthread

TARGET      = canny_edge_detection
RV_TARGET   = canny_rv
TEST_TARGET = run_tests

# ---------------------------------------------------------------------------
# Build targets
# ---------------------------------------------------------------------------

# Default: build the native (host) binary
all: $(TARGET)

$(TARGET): $(APP_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Cross-compiled RISC-V binary (required by project spec)
# Usage: make canny_rv        (uses OPT=-O3 by default)
#        make canny_rv OPT=-O0
canny_rv:
	$(MAKE) RVV=1 OPT=$(OPT) $(RV_TARGET)_bin

$(RV_TARGET)_bin: $(APP_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $(RV_TARGET)

# Compile object files (works for both host and cross builds)
src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

tests/%.o: tests/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ---------------------------------------------------------------------------
# Test targets
# ---------------------------------------------------------------------------

# Build and run the GoogleTest suite on the host
test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(filter-out src/main.o, $(APP_OBJS)) $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LIBS)

# ---------------------------------------------------------------------------
# QEMU execution
# ---------------------------------------------------------------------------

# Run the RISC-V binary on QEMU (builds canny_rv first if needed)
# Usage: make run            (VLEN=128, default image)
#        make run VLEN=256
#        make run VLEN=512
run: canny_rv
	qemu-riscv64 -L /usr/riscv64-linux-gnu \
	    -cpu rv64,v=true,vlen=$(VLEN) \
	    ./$(RV_TARGET) input.raw 512 512

# ---------------------------------------------------------------------------
# Utility
# ---------------------------------------------------------------------------
clean:
	rm -f src/*.o tests/*.o $(TARGET) $(RV_TARGET) $(TEST_TARGET)

.PHONY: all test canny_rv run clean