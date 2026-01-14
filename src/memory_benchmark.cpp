#include "memory_benchmark.h"
#include "timer.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <memory>
#include <cstring>
#include <algorithm>

MemoryBenchmark::MemoryBenchmark() noexcept {
}

MemoryBenchmark::Results MemoryBenchmark::run(
    std::size_t buffer_size_bytes,
    std::size_t iterations
) {
    Results results{};
    results.buffer_size_bytes = buffer_size_bytes;
    results.iterations = iterations;
    results.verification_passed = true;
    results.verification_errors = 0;

    // Validate inputs
    if (buffer_size_bytes == 0) {
        std::cerr << "Error: Buffer size must be greater than 0\n";
        return results;
    }
    if (iterations == 0) {
        std::cerr << "Error: Iterations must be greater than 0\n";
        return results;
    }

    // Allocate buffer using vector for automatic cleanup
    std::vector<std::uint8_t> buffer(buffer_size_bytes);

    // Run benchmark
    Timer timer;
    timer.start();

    std::size_t total_errors = 0;
    for (std::size_t i = 0; i < iterations; ++i) {
        std::size_t errors = verify_cycle(buffer.data(), buffer_size_bytes);
        total_errors += errors;
    }

    double elapsed_seconds = timer.elapsed_seconds();
    std::int64_t elapsed_nanoseconds = timer.elapsed_nanoseconds();

    // Calculate results
    results.total_time_seconds = elapsed_seconds;
    results.average_latency_nanoseconds = static_cast<double>(elapsed_nanoseconds) 
                                          / static_cast<double>(iterations);
    
    // Calculate throughput: (buffer_size * iterations * 3) / time
    // Factor of 3 because we do read-write-read (three operations per cycle)
    double total_bytes_processed = static_cast<double>(buffer_size_bytes) 
                                   * static_cast<double>(iterations) * 3.0;
    results.throughput_mbps = (total_bytes_processed / elapsed_seconds) / (1024.0 * 1024.0);
    
    results.verification_errors = total_errors;
    results.verification_passed = (total_errors == 0);

    return results;
}

std::size_t MemoryBenchmark::verify_cycle(
    std::uint8_t* buffer,
    std::size_t size
) noexcept {
    // Read: Touch all bytes to test read performance
    volatile std::uint8_t dummy = 0;
    for (std::size_t i = 0; i < size; ++i) {
        dummy ^= buffer[i];  // Read and accumulate to prevent optimization
    }
    (void)dummy;  // Suppress unused variable warning
    
    // Write: Write pattern to test write performance
    write_pattern(buffer, size);
    
    // Read: Read and verify pattern to test read performance and correctness
    std::size_t errors = verify_pattern(buffer, size);
    
    return errors;
}

void MemoryBenchmark::write_pattern(
    std::uint8_t* buffer,
    std::size_t size
) noexcept {
    // Write a simple pattern: each byte gets its position modulo 256
    // This ensures we can verify correctness byte-by-byte
    for (std::size_t i = 0; i < size; ++i) {
        buffer[i] = static_cast<std::uint8_t>(i & 0xFF);
    }
}

std::size_t MemoryBenchmark::verify_pattern(
    const std::uint8_t* buffer,
    std::size_t size
) const noexcept {
    std::size_t errors = 0;
    
    // Verify each byte matches the expected pattern byte-by-byte
    for (std::size_t i = 0; i < size; ++i) {
        std::uint8_t expected = static_cast<std::uint8_t>(i & 0xFF);
        if (buffer[i] != expected) {
            ++errors;
        }
    }
    
    return errors;
}

void MemoryBenchmark::print_results(const Results& results) {
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "  RAM Benchmark Results\n";
    std::cout << "========================================\n";
    std::cout << "\n";
    
    // Format buffer size
    std::cout << std::left << std::setw(30) << "Buffer Size:";
    if (results.buffer_size_bytes < 1024) {
        std::cout << results.buffer_size_bytes << " bytes\n";
    } else if (results.buffer_size_bytes < 1024 * 1024) {
        std::cout << std::fixed << std::setprecision(2) 
                  << (results.buffer_size_bytes / 1024.0) << " KB\n";
    } else {
        std::cout << std::fixed << std::setprecision(2) 
                  << (results.buffer_size_bytes / (1024.0 * 1024.0)) << " MB\n";
    }
    
    std::cout << std::left << std::setw(30) << "Iterations:" 
              << results.iterations << "\n";
    
    std::cout << std::left << std::setw(30) << "Total Time:" 
              << std::fixed << std::setprecision(6) 
              << results.total_time_seconds << " seconds\n";
    
    std::cout << std::left << std::setw(30) << "Average Latency:" 
              << std::fixed << std::setprecision(2) 
              << results.average_latency_nanoseconds << " ns\n";
    
    std::cout << std::left << std::setw(30) << "Throughput:" 
              << std::fixed << std::setprecision(2) 
              << results.throughput_mbps << " MB/s\n";
    
    std::cout << std::left << std::setw(30) << "Verification:" 
              << (results.verification_passed ? "PASSED" : "FAILED");
    if (!results.verification_passed) {
        std::cout << " (" << results.verification_errors << " errors)";
    }
    std::cout << "\n";
    
    std::cout << "\n";
}

