/**
 * cpu_benchmark.cpp - CPU benchmark implementation
 * 
 * Tests computational performance using integer, floating-point,
 * and memory-bound operations.
 */

#include "cpu_benchmark.h"
#include "timer.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <algorithm>

CpuBenchmark::CpuBenchmark() noexcept {
}

CpuBenchmark::Results CpuBenchmark::run(std::size_t iterations) {
    Results results{};
    results.iterations = iterations;
    results.benchmark_successful = false;
    results.benchmark_type = "Mixed CPU Workload";

    // Validate inputs
    if (iterations == 0) {
        std::cerr << "Error: Iterations must be greater than 0\n";
        return results;
    }

    Timer total_timer;
    total_timer.start();

    // Run different types of CPU workloads
    std::uint64_t int_result = compute_integer_workload(iterations);
    double float_result = compute_float_workload(iterations);
    std::uint64_t mem_result = compute_memory_workload(iterations);

    double elapsed_seconds = total_timer.elapsed_seconds();

    // Use results to prevent optimization
    volatile std::uint64_t dummy1 = int_result;
    volatile double dummy2 = float_result;
    volatile std::uint64_t dummy3 = mem_result;
    (void)dummy1;
    (void)dummy2;
    (void)dummy3;

    // Calculate metrics
    results.timing.total_time_seconds = elapsed_seconds;
    results.timing.operations_per_second = static_cast<double>(iterations * 3) / elapsed_seconds; // 3 workloads
    results.timing.time_per_operation_ns = (elapsed_seconds / static_cast<double>(iterations * 3)) * 1'000'000'000.0;

    results.benchmark_successful = true;

    return results;
}

std::uint64_t CpuBenchmark::compute_integer_workload(std::size_t iterations) noexcept {
    std::uint64_t result = 1;
    for (std::size_t i = 0; i < iterations; ++i) {
        result = (result * 31 + 17) % 1000000007ULL; // Modular multiplication
        result ^= (result << 13) | (result >> 19);    // Bit operations
    }
    return result;
}

double CpuBenchmark::compute_float_workload(std::size_t iterations) noexcept {
    double result = 1.0;
    for (std::size_t i = 0; i < iterations; ++i) {
        result = std::sin(result + static_cast<double>(i)) * std::cos(result);
        result = std::sqrt(std::abs(result) + 1.0);
        result = std::exp(result * 0.1) - 1.0;
    }
    return result;
}

std::uint64_t CpuBenchmark::compute_memory_workload(std::size_t iterations) noexcept {
    const std::size_t array_size = 1024;
    std::vector<std::uint64_t> data(array_size);

    // Initialize with pattern
    for (std::size_t i = 0; i < array_size; ++i) {
        data[i] = i * 31 + 17;
    }

    std::uint64_t result = 0;
    for (std::size_t i = 0; i < iterations; ++i) {
        std::size_t index = (i * 7) % array_size;
        result += data[index];
        data[index] = (result * 13) % 1000000007ULL;
    }

    return result;
}

void CpuBenchmark::print_results(const Results& results) {
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "  CPU Benchmark Results\n";
    std::cout << "========================================\n";
    std::cout << "\n";

    std::cout << "Benchmark Type: " << results.benchmark_type << "\n";
    std::cout << "Iterations: " << results.iterations << "\n";
    std::cout << "\n";

    if (results.benchmark_successful) {
        std::cout << "Performance Metrics:\n";
        std::cout << "  " << std::left << std::setw(30) << "Total Time:"
                  << std::fixed << std::setprecision(6)
                  << results.timing.total_time_seconds << " seconds\n";

        std::cout << "  " << std::left << std::setw(30) << "Operations/Second:"
                  << std::fixed << std::setprecision(2)
                  << results.timing.operations_per_second << "\n";

        std::cout << "  " << std::left << std::setw(30) << "Time/Operation:"
                  << std::fixed << std::setprecision(2)
                  << results.timing.time_per_operation_ns << " ns\n";
    } else {
        std::cout << "Benchmark failed to complete successfully.\n";
    }

    std::cout << "\n";
    std::cout << "Note: CPU benchmarks measure computational throughput and may vary\n";
    std::cout << "      based on CPU frequency scaling, thermal throttling, and system load.\n";
    std::cout << "\n";
}