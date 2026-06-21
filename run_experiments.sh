#!/bin/bash

echo "====================================================="
echo "  CANNY EDGE DETECTION - OPTIMIZATION SWEEP RUNNER   "
echo "====================================================="

# Helper function to compile and execute tests
run_experiment() {
    local OPT_FLAG=$1
    local RVV_FLAG=$2
    local VLEN=$3
    local EXP_NAME=$4

    echo "-----------------------------------------------------"
    echo "Experiment: $EXP_NAME"
    echo "Compiler Flags: $OPT_FLAG | RVV: $RVV_FLAG | VLEN: $VLEN"
    
    # Clean workspace and compile
    make clean > /dev/null 2>&1
    make RVV=$RVV_FLAG OPT="$OPT_FLAG" CXX=riscv64-linux-gnu-g++ > /dev/null 2>&1
    
    if [ ! -f "canny_edge_detection" ]; then
        echo "Compilation FAILED!"
        return
    fi

    # Extract and print binary code size
    BIN_SIZE=$(riscv64-linux-gnu-size canny_edge_detection | awk 'NR==2 {print $1}')
    echo "Binary Size (Text/Code): $BIN_SIZE bytes"

    # Execute using QEMU, enabling RVV if required
    if [ "$RVV_FLAG" -eq 1 ]; then
        qemu-riscv64 -L /usr/riscv64-linux-gnu -cpu rv64,v=true,vlen=$VLEN ./canny_edge_detection input.raw 512 512 | grep -E "Gaussian Blur|Sobel Op"
    else
        qemu-riscv64 -L /usr/riscv64-linux-gnu ./canny_edge_detection input.raw 512 512 | grep -E "Gaussian Blur|Sobel Op"
    fi
}

# Ensure test image exists; create a 512x512 dummy file if missing
if [ ! -f "input.raw" ]; then
    echo "Warning: input.raw not found! Creating a dummy 512x512 file for testing..."
    dd if=/dev/urandom of=input.raw bs=1 count=262144 > /dev/null 2>&1
fi

# 1. Scalar Baseline & Optimizations
run_experiment "-O0" 0 128 "1. Scalar Baseline (-O0)"
run_experiment "-O2" 0 128 "2. Scalar Optimized (-O2)"
run_experiment "-O3" 0 128 "3. Scalar Optimized (-O3)"
run_experiment "-O3 -ftree-vectorize" 0 128 "4. Scalar Auto-Vectorized"

# 2. RVV Intrinsics (VLEN Sweep)
run_experiment "-O3" 1 128 "5. RVV Intrinsics (VLEN=128)"
run_experiment "-O3" 1 256 "6. RVV Intrinsics (VLEN=256)"
run_experiment "-O3" 1 512 "7. RVV Intrinsics (VLEN=512)"

echo "-----------------------------------------------------"
echo "Done! Copy these numbers into your PDF Table."
