#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <time.h>
#include <stdio.h>

// Returns elapsed milliseconds between two timespec values.
// Used internally by the macros below.
static inline double timespec_diff_ms(struct timespec start, struct timespec end) {
    return (end.tv_sec  - start.tv_sec)  * 1000.0
         + (end.tv_nsec - start.tv_nsec) / 1.0e6;
}

// MEASURE_TIME_LOOP(name, call, N):
//   Runs `call` exactly N times, prints the average time per iteration.
//   Use N >= 100 for stable measurements on QEMU (hides JIT warm-up noise).
#define MEASURE_TIME_LOOP(name, function_call, N)                        \
    do {                                                                  \
        struct timespec _s, _e;                                           \
        clock_gettime(CLOCK_MONOTONIC, &_s);                              \
        for (int _i = 0; _i < (N); _i++) { function_call; }              \
        clock_gettime(CLOCK_MONOTONIC, &_e);                              \
        double _total = timespec_diff_ms(_s, _e);                        \
        printf("[Bench] %-25s avg/%d runs: %.4f ms\n",                   \
               name, (N), _total / (N));                                  \
    } while (0)

// MEASURE_TIME_MS(name, call, out_ms):
//   Runs `call` once, stores elapsed milliseconds in `out_ms` (double).
//   Use this for per-stage profiling where you need the raw value.
#define MEASURE_TIME_MS(name, function_call, out_ms)                     \
    do {                                                                  \
        struct timespec _s, _e;                                           \
        clock_gettime(CLOCK_MONOTONIC, &_s);                              \
        function_call;                                                    \
        clock_gettime(CLOCK_MONOTONIC, &_e);                              \
        (out_ms) = timespec_diff_ms(_s, _e);                             \
    } while (0)

// PRINT_PROFILE_REPORT(t_gauss, t_sobel, t_nms, t_thresh, t_hyst):
//   Prints a formatted per-stage breakdown with percentages.
//   Required for Phase 5 (Profiling and Hotspot Identification).
#define PRINT_PROFILE_REPORT(t_gauss, t_sobel, t_nms, t_thresh, t_hyst) \
    do {                                                                  \
        double _total = (t_gauss)+(t_sobel)+(t_nms)+(t_thresh)+(t_hyst);\
        if (_total < 1e-9) _total = 1e-9;                                \
        printf("\n============== PERFORMANCE REPORT ==============\n");   \
        printf("Stage               Time(ms)   Percent\n");               \
        printf("------------------------------------------------\n");     \
        printf("1. Gaussian Blur    %8.4f   %5.1f%%\n",                  \
               (t_gauss),  (t_gauss)  / _total * 100.0);                 \
        printf("2. Sobel Operator   %8.4f   %5.1f%%\n",                  \
               (t_sobel),  (t_sobel)  / _total * 100.0);                 \
        printf("3. Non-Max Supp.    %8.4f   %5.1f%%\n",                  \
               (t_nms),    (t_nms)    / _total * 100.0);                 \
        printf("4. Double Thresh.   %8.4f   %5.1f%%\n",                  \
               (t_thresh), (t_thresh) / _total * 100.0);                 \
        printf("5. Hysteresis       %8.4f   %5.1f%%\n",                  \
               (t_hyst),   (t_hyst)   / _total * 100.0);                 \
        printf("------------------------------------------------\n");     \
        printf("Total Pipeline      %8.4f   100.0%%\n", _total);         \
        printf("================================================\n\n");   \
    } while (0)

#endif // BENCHMARK_H
