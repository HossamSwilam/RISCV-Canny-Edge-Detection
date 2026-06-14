#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <chrono>
#include <iostream>
#include <string>

// ماكرو جاهز عشان يقيس وقت أي دالة بسهولة
#define MEASURE_TIME(name, function_call) \
    do { \
        auto start = std::chrono::high_resolution_clock::now(); \
        function_call; \
        auto end = std::chrono::high_resolution_clock::now(); \
        std::chrono::duration<double, std::milli> duration = end - start; \
        std::cout << "[Benchmark] " << name << " took: " << duration.count() << " ms\n"; \
    } while(0)

#endif // BENCHMARK_H
