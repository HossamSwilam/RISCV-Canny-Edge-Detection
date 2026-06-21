# Canny Edge Detection on RISC-V (RVV 1.0)

Supervised by Dr. Omar Ahmed Nasr

A Canny edge detection pipeline implemented first as a portable scalar C++ baseline, then
hand-optimized with RISC-V Vector (RVV) 1.0 intrinsics, cross-compiled for `rv64gcv` and
verified on QEMU user-mode emulation across multiple `VLEN` configurations.

---

## 1. What This Project Does

The pipeline implements:

1. **Gaussian Blur** — 5×5 convolution (scalar baseline + RVV intrinsic version)
2. **Sobel Gradient** — Gx/Gy via 3×3 kernels, magnitude (L1), and 4-direction quantization
   (0°, 45°, 90°, 135°), in both scalar and RVV intrinsic versions
3. **Non-Maximum Suppression, Double Thresholding, and Hysteresis** — implemented as a
   scalar post-processing stage (`applyCannyPostProcessing`) that completes the full
   five-stage Canny algorithm beyond the minimum Gaussian + Sobel deliverable

The RVV kernels are vector-length-agnostic: the same binary runs correctly at `VLEN=128`,
`256`, and `512` without recompilation, verified against the scalar reference output.

---

## 2. Repository Structure

```
.
├── include/                  # Image struct, processing.h declarations
├── src/
│   ├── canny_ops.cpp         # NMS / thresholding / hysteresis (scalar)
│   ├── gaussian.cpp          # Scalar Gaussian blur + separable variant
│   ├── sobel.cpp             # Scalar Sobel (Gx/Gy, magnitude, direction)
│   ├── gaussian_blur_rvv.cpp # RVV intrinsic Gaussian blur
│   ├── sobel_rvv.cpp         # RVV intrinsic Sobel (magnitude + direction)
│   └── main.cpp              # Pipeline driver, benchmarking, raw image I/O
├── tests/
│   ├── test_gaussian.cpp     # GoogleTest — host-side unit tests
│   ├── test_sobel.cpp        # GoogleTest — host-side unit tests
│   └── test_canny.cpp        # GoogleTest — host-side unit tests
├── equivalence_test.cpp      # QEMU-side assert-based scalar vs. RVV equivalence test
├── Makefile
└── README.md
```

---

## 3. Build Requirements

- `riscv64-linux-gnu-g++` (GCC, with `-march=rv64gcv` support)
- `qemu-riscv64` with RISC-V user-mode emulation and `v=true` vector support
- GoogleTest (`libgtest`, `libgtest_main`) for host-side unit tests
- A native `g++` (C++17) for host builds and testing

See `RV-Embedded-detailed_hints_guide.pdf` (Phase 1) for full WSL2/toolchain/QEMU build
instructions if these aren't already installed.

---

## 4. Building and Running

The Makefile supports a **dual-target** build: native host (scalar, for testing) and
cross-compiled RISC-V (scalar or RVV).

### Native build (host, scalar fallback)
```bash
make
./canny_edge_detection input.raw 512 512
```

### Cross-compiled scalar build for RISC-V
```bash
make clean
make RVV=0
qemu-riscv64 -L /usr/riscv64-linux-gnu ./canny_edge_detection input.raw 512 512
```

### Cross-compiled RVV build, run on QEMU
```bash
make clean
make RVV=1
make run-rvv VLEN=128 RVV=1
```

`VLEN` can be set to `128`, `256`, or `512` to verify vector-length-agnostic correctness:
```bash
make run-rvv VLEN=256 RVV=1
make run-rvv VLEN=512 RVV=1
```

> **Note:** `run-rvv` currently hardcodes `input.raw 512 512` in the Makefile target. Swap
> in your own image path/dimensions there if needed.

## 🚀 Canny Edge Detection Pipeline Results

Here is the step-by-step output of our hardware-accelerated pipeline:

| Original Input | Blurred (Gaussian) |
| :---: | :---: |
| <img src="assets/0_input.png" width="400"> | <img src="assets/1_blurred.png" width="400"> |

| Magnitude (Sobel) | Final Edges (NMS) |
| :---: | :---: |
| <img src="assets/2_magnitude.png" width="400"> | <img src="assets/4_final_edges.png" width="400"> |

> **Note:** The final edges demonstrate the success of our Non-Maximum Suppression and Double Thresholding implementation, resulting in crisp, 1-pixel thin continuous edges.

### Running host-side unit tests
```bash
make test
```
Runs the GoogleTest suite (`test_gaussian.cpp`, `test_sobel.cpp`, `test_canny.cpp`) natively
on the host compiler — fast iteration, no QEMU required.

### Running the QEMU scalar-vs-RVV equivalence test
Compile `equivalence_test.cpp` against the same object files with `RVV=1` and run it under
`qemu-riscv64` the same way as the main binary. It generates a 100×75 random test image
(non-power-of-two, to exercise the strip-mining tail case), runs both scalar and RVV
versions of Gaussian blur and Sobel, and asserts they match within tolerance.

---

## 5. Image Format

Raw headerless grayscale: exactly `width * height` bytes, one byte per pixel (0 = black,
255 = white). No compression, no metadata. Width/height are passed as command-line
arguments since they aren't stored in the file.

```python
import numpy as np
img = np.fromfile('img.raw', dtype=np.uint8).reshape(height, width)
```

---

## 6. Pipeline Stages and Outputs

Running `main.cpp` against an input image produces intermediate raw files at each stage:

| File | Stage |
|---|---|
| `0_input.raw` | Original grayscale input |
| `1_blurred.raw` | After 5×5 Gaussian blur |
| `2_magnitude.raw` | Sobel L1 magnitude (`|Gx| + |Gy|`, clamped to 255) |
| `3_direction.raw` | Quantized gradient direction (scaled ×50 for visibility) |

NMS, thresholding, and hysteresis (`applyCannyPostProcessing`) run once on the final
magnitude/direction pair to produce the full Canny output, separate from the benchmarked
Gaussian/Sobel loop.

---

## 7. Performance Results

Measured on QEMU user-mode emulation, wall-clock timing via `clock_gettime`, averaged over
10 iterations. Absolute numbers are not cycle-accurate (QEMU doesn't model a real
microarchitecture) — the relative comparisons across optimization levels are what matter.

| Stage | Flags | VLEN | Binary (.text) | Gaussian Blur | Sobel |
|---|---|---|---|---|---|
| 1. Scalar Baseline | `-O0` | — | 19,707 B | 571.73 ms | 258.80 ms |
| 2. Scalar Optimized | `-O2` | — | 14,878 B | 38.13 ms | 166.91 ms |
| 3. Scalar Optimized | `-O3` | — | 27,246 B | 11.14 ms | 145.40 ms |
| 4. Auto-Vectorized | `-O3 -ftree-vectorize` | — | 27,246 B | 21.34 ms | 344.08 ms |
| 5. RVV Intrinsics | `-O3` (RVV=1) | 128 | 25,996 B | 142.48 ms | 47.86 ms |
| 6. RVV Intrinsics | `-O3` (RVV=1) | 256 | 25,996 B | 128.04 ms | 27.86 ms |
| 7. RVV Intrinsics | `-O3` (RVV=1) | 512 | 25,996 B | 67.74 ms | 16.26 ms |


## 9. Testing Strategy

- **Host-side (GoogleTest, `make test`):** fast correctness checks on pipeline logic —
  uniform-image invariants, zero-gradient on flat regions, direction quantization on
  synthetic edges — compiled and run natively, no cross-compiler or QEMU needed.
- **QEMU-side (`equivalence_test.cpp`):** cross-compiled with `RVV=1`, run under
  `qemu-riscv64`. Generates a 100×75 (non-power-of-two) random image specifically to force
  the strip-mining tail case, then asserts scalar and RVV outputs match within a tolerance
  (and margin, per the note above) at `VLEN=128`, `256`, and `512`.


## 11. Team

| Hossam swilam| Project coordination, Qemu_test script configuration, overall code review,phase 5 deliverables, and simulation management. |
| Hazem Adel |Development and integration of all Gaussian filter implementations throughout the project lifecycle. |
| Abdulrahman Hamada | Development and integration of all Sobel operator implementations throughout the project lifecycle. |
| Hassan salah & Braa mekky  | Implementation of canny_ops and execution of all Phase 4 deliverables. |
