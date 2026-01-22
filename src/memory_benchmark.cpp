#include "memory_benchmark.h"
#include "timer.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <cmath>

MemoryBenchmark::MemoryBenchmark() noexcept {
}

MemoryBenchmark::Results MemoryBenchmark::run(
    std::size_t buffer_size_bytes,
    std::size_t iterations
) {
    // Validate inputs
    if (buffer_size_bytes == 0) {
        Results results{};
        std::cerr << "Error: Buffer size must be greater than 0\n";
        return results;
    }
    if (iterations == 0) {
        Results results{};
        std::cerr << "Error: Iterations must be greater than 0\n";
        return results;
    }

    // Allocate buffer using vector for automatic cleanup
    std::vector<std::uint8_t> buffer(buffer_size_bytes);
    
    // Use helper method with pre-allocated buffer
    return run_with_buffer(buffer.data(), buffer_size_bytes, iterations);
}

MemoryBenchmark::Results MemoryBenchmark::run_with_buffer(
    std::uint8_t* buffer,
    std::size_t buffer_size_bytes,
    std::size_t iterations
) {
    Results results{};
    results.buffer_size_bytes = buffer_size_bytes;
    results.iterations = iterations;
    results.verification_passed = true;
    results.verification_errors = 0;
    results.timing.min_latency_ns = 0.0;
    results.timing.max_latency_ns = 0.0;
    results.timing.avg_latency_ns = 0.0;
    results.timing.total_time_seconds = 0.0;
    results.timing.variance_ns = 0.0;
    results.timing.std_deviation_ns = 0.0;
    results.timing.sample_count = 0;

    // Run benchmark with per-iteration timing
    Timer total_timer;
    total_timer.start();

    std::size_t total_errors = 0;
    std::int64_t min_latency_ns = 0;
    std::int64_t max_latency_ns = 0;
    std::int64_t sum_latency_ns = 0;
    bool first_iteration = true;
    
    // Store latencies for variance calculation
    std::vector<double> latencies;
    latencies.reserve(iterations);

    for (std::size_t i = 0; i < iterations; ++i) {
        std::int64_t cycle_latency_ns = 0;
        std::size_t errors = verify_cycle(buffer.data(), buffer_size_bytes, 
                                          cycle_latency_ns);
        total_errors += errors;

        // Track min/max/average latency incrementally
        if (first_iteration) {
            min_latency_ns = cycle_latency_ns;
            max_latency_ns = cycle_latency_ns;
            first_iteration = false;
        } else {
            if (cycle_latency_ns < min_latency_ns) {
                min_latency_ns = cycle_latency_ns;
            }
            if (cycle_latency_ns > max_latency_ns) {
                max_latency_ns = cycle_latency_ns;
            }
        }
        sum_latency_ns += cycle_latency_ns;
        latencies.push_back(static_cast<double>(cycle_latency_ns));
    }

    double elapsed_seconds = total_timer.elapsed_seconds();

    // Calculate timing statistics
    results.timing.total_time_seconds = elapsed_seconds;
    results.timing.min_latency_ns = static_cast<double>(min_latency_ns);
    results.timing.max_latency_ns = static_cast<double>(max_latency_ns);
    results.timing.avg_latency_ns = static_cast<double>(sum_latency_ns) 
                                    / static_cast<double>(iterations);
    results.timing.sample_count = iterations;
    
    // Calculate variance and standard deviation
    calculate_statistics(latencies, results.timing.avg_latency_ns,
                        results.timing.variance_ns, results.timing.std_deviation_ns);
    
    // Calculate throughput: (buffer_size * iterations * 3) / time
    // Factor of 3 because we do read-write-read (three operations per cycle)
    double total_bytes_processed = static_cast<double>(buffer_size_bytes) 
                                   * static_cast<double>(iterations) * 3.0;
    results.throughput_mbps = (total_bytes_processed / elapsed_seconds) / (1024.0 * 1024.0);
    
    results.verification_errors = total_errors;
    results.verification_passed = (total_errors == 0);

    return results;
}

MemoryBenchmark::Results MemoryBenchmark::run_continuous(
    std::size_t buffer_size_bytes,
    std::size_t iterations_per_run,
    std::size_t max_runs,
    double max_duration_seconds
) {
    Results aggregated_results{};
    aggregated_results.buffer_size_bytes = buffer_size_bytes;
    aggregated_results.iterations = iterations_per_run;
    aggregated_results.verification_passed = true;
    aggregated_results.verification_errors = 0;
    aggregated_results.timing.min_latency_ns = 0.0;
    aggregated_results.timing.max_latency_ns = 0.0;
    aggregated_results.timing.avg_latency_ns = 0.0;
    aggregated_results.timing.total_time_seconds = 0.0;
    aggregated_results.timing.variance_ns = 0.0;
    aggregated_results.timing.std_deviation_ns = 0.0;
    aggregated_results.timing.sample_count = 0;

    // Validate inputs
    if (buffer_size_bytes == 0) {
        std::cerr << "Error: Buffer size must be greater than 0\n";
        return aggregated_results;
    }
    if (iterations_per_run == 0) {
        std::cerr << "Error: Iterations per run must be greater than 0\n";
        return aggregated_results;
    }
    if (max_runs == 0 && max_duration_seconds <= 0.0) {
        std::cerr << "Error: Either max_runs or max_duration_seconds must be specified\n";
        return aggregated_results;
    }

    // Allocate buffer once - reuse across runs for deterministic behavior
    std::vector<std::uint8_t> buffer(buffer_size_bytes);

    // Track results across runs
    std::vector<double> run_avg_latencies;
    std::vector<double> run_total_times;
    std::size_t total_errors = 0;
    std::size_t completed_runs = 0;
    
    Timer continuous_timer;
    continuous_timer.start();

    // Run continuous benchmark loop
    bool should_continue = true;
    while (should_continue) {
        // Check duration limit
        if (max_duration_seconds > 0.0) {
            double elapsed = continuous_timer.elapsed_seconds();
            if (elapsed >= max_duration_seconds) {
                should_continue = false;
                break;
            }
        }

        // Check run count limit
        if (max_runs > 0 && completed_runs >= max_runs) {
            should_continue = false;
            break;
        }

        // Run a single benchmark with pre-allocated buffer (deterministic, no allocation in loop)
        Results run_results = run_with_buffer(buffer.data(), buffer_size_bytes, iterations_per_run);
        
        // Aggregate results
        if (run_results.timing.sample_count > 0) {
            run_avg_latencies.push_back(run_results.timing.avg_latency_ns);
            run_total_times.push_back(run_results.timing.total_time_seconds);
            total_errors += run_results.verification_errors;
            
            // Update min/max across runs
            if (completed_runs == 0) {
                aggregated_results.timing.min_latency_ns = run_results.timing.min_latency_ns;
                aggregated_results.timing.max_latency_ns = run_results.timing.max_latency_ns;
            } else {
                if (run_results.timing.min_latency_ns < aggregated_results.timing.min_latency_ns) {
                    aggregated_results.timing.min_latency_ns = run_results.timing.min_latency_ns;
                }
                if (run_results.timing.max_latency_ns > aggregated_results.timing.max_latency_ns) {
                    aggregated_results.timing.max_latency_ns = run_results.timing.max_latency_ns;
                }
            }
            
            aggregated_results.timing.total_time_seconds += run_results.timing.total_time_seconds;
            completed_runs++;
        } else {
            // Run failed, break to avoid infinite loop
            break;
        }
    }

    // Calculate aggregated statistics
    if (completed_runs > 0) {
        // Calculate average latency across runs
        double sum_avg_latency = 0.0;
        for (double avg : run_avg_latencies) {
            sum_avg_latency += avg;
        }
        aggregated_results.timing.avg_latency_ns = sum_avg_latency / static_cast<double>(completed_runs);
        aggregated_results.timing.sample_count = completed_runs;
        
        // Calculate variance and standard deviation of run averages
        calculate_statistics(run_avg_latencies, aggregated_results.timing.avg_latency_ns,
                            aggregated_results.timing.variance_ns, 
                            aggregated_results.timing.std_deviation_ns);
        
        // Calculate total throughput
        double total_bytes_processed = static_cast<double>(buffer_size_bytes) 
                                      * static_cast<double>(iterations_per_run) 
                                      * static_cast<double>(completed_runs) * 3.0;
        aggregated_results.throughput_mbps = (total_bytes_processed / aggregated_results.timing.total_time_seconds) 
                                            / (1024.0 * 1024.0);
        
        aggregated_results.iterations = iterations_per_run * completed_runs;
    }

    aggregated_results.verification_errors = total_errors;
    aggregated_results.verification_passed = (total_errors == 0);

    return aggregated_results;
}

void MemoryBenchmark::calculate_statistics(
    const std::vector<double>& latencies,
    double mean,
    double& variance,
    double& std_deviation
) noexcept {
    variance = 0.0;
    std_deviation = 0.0;
    
    if (latencies.empty() || latencies.size() == 1) {
        return;
    }
    
    // Calculate variance: sum of squared differences from mean
    double sum_squared_diff = 0.0;
    for (double latency : latencies) {
        double diff = latency - mean;
        sum_squared_diff += diff * diff;
    }
    
    // Sample variance (divide by n-1 for sample, n for population)
    // Using sample variance for better statistical properties
    std::size_t n = latencies.size();
    if (n > 1) {
        variance = sum_squared_diff / static_cast<double>(n - 1);
        
        // Standard deviation is square root of variance
        if (variance > 0.0) {
            std_deviation = std::sqrt(variance);
        }
    }
}

std::size_t MemoryBenchmark::verify_cycle(
    std::uint8_t* buffer,
    std::size_t size,
    std::int64_t& cycle_latency_ns
) noexcept {
    // Measure latency of this cycle using monotonic clock
    Timer cycle_timer;
    cycle_timer.start();

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
    
    // Record cycle latency
    cycle_latency_ns = cycle_timer.elapsed_nanoseconds();
    
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

    // Configuration Table
    std::cout << "Configuration:\n";
    std::cout << "  " << std::left << std::setw(25) << "Buffer Size:";
    if (results.buffer_size_bytes < 1024) {
        std::cout << results.buffer_size_bytes << " bytes\n";
    } else if (results.buffer_size_bytes < 1024 * 1024) {
        std::cout << std::fixed << std::setprecision(2) 
                  << (results.buffer_size_bytes / 1024.0) << " KB\n";
    } else {
        std::cout << std::fixed << std::setprecision(2) 
                  << (results.buffer_size_bytes / (1024.0 * 1024.0)) << " MB\n";
    }
    
    std::cout << "  " << std::left << std::setw(25) << "Iterations:" 
              << results.iterations << "\n";
    std::cout << "\n";

    // Timing Statistics Table
    std::cout << "Timing Statistics:\n";
    std::cout << "  " << std::left << std::setw(25) << "Total Time:" 
              << std::fixed << std::setprecision(6) 
              << results.timing.total_time_seconds << " seconds\n";
    
    std::cout << "  " << std::left << std::setw(25) << "Min Latency:" 
              << std::fixed << std::setprecision(2) 
              << results.timing.min_latency_ns << " ns\n";
    
    std::cout << "  " << std::left << std::setw(25) << "Max Latency:" 
              << std::fixed << std::setprecision(2) 
              << results.timing.max_latency_ns << " ns\n";
    
    std::cout << "  " << std::left << std::setw(25) << "Average Latency:" 
              << std::fixed << std::setprecision(2) 
              << results.timing.avg_latency_ns << " ns\n";
    
    // Calculate latency spread
    double latency_spread_ns = results.timing.max_latency_ns - results.timing.min_latency_ns;
    std::cout << "  " << std::left << std::setw(25) << "Latency Spread:" 
              << std::fixed << std::setprecision(2) 
              << latency_spread_ns << " ns\n";
    
    // Show variance and standard deviation if available
    if (results.timing.sample_count > 1) {
        std::cout << "  " << std::left << std::setw(25) << "Variance:" 
                  << std::fixed << std::setprecision(2) 
                  << results.timing.variance_ns << " ns²\n";
        
        std::cout << "  " << std::left << std::setw(25) << "Std Deviation:" 
                  << std::fixed << std::setprecision(2) 
                  << results.timing.std_deviation_ns << " ns\n";
        
        // Coefficient of variation (relative standard deviation)
        if (results.timing.avg_latency_ns > 0.0) {
            double cv = (results.timing.std_deviation_ns / results.timing.avg_latency_ns) * 100.0;
            std::cout << "  " << std::left << std::setw(25) << "Coefficient of Variation:" 
                      << std::fixed << std::setprecision(2) 
                      << cv << " %\n";
        }
    }
    
    if (results.timing.sample_count > 0) {
        std::cout << "  " << std::left << std::setw(25) << "Sample Count:" 
                  << results.timing.sample_count << "\n";
    }
    std::cout << "\n";

    // Performance Metrics Table
    std::cout << "Performance Metrics:\n";
    std::cout << "  " << std::left << std::setw(25) << "Throughput:" 
              << std::fixed << std::setprecision(2) 
              << results.throughput_mbps << " MB/s\n";
    
    // Calculate operations per second
    double ops_per_second = static_cast<double>(results.iterations) 
                           / results.timing.total_time_seconds;
    std::cout << "  " << std::left << std::setw(25) << "Operations/sec:" 
              << std::fixed << std::setprecision(2) 
              << ops_per_second;
    if (ops_per_second >= 1000000.0) {
        std::cout << " (" << std::fixed << std::setprecision(2) 
                  << (ops_per_second / 1000000.0) << " M ops/s)";
    } else if (ops_per_second >= 1000.0) {
        std::cout << " (" << std::fixed << std::setprecision(2) 
                  << (ops_per_second / 1000.0) << " K ops/s)";
    }
    std::cout << "\n";
    std::cout << "\n";

    // Verification Table
    std::cout << "Verification:\n";
    std::cout << "  " << std::left << std::setw(25) << "Status:" 
              << (results.verification_passed ? "PASSED" : "FAILED");
    if (!results.verification_passed) {
        std::cout << " (" << results.verification_errors << " errors)";
    }
    std::cout << "\n";
    std::cout << "\n";

    // Summary Table (compact view)
    std::cout << "Summary Table:\n";
    std::cout << "  " << std::string(60, '-') << "\n";
    std::cout << "  " << std::left << std::setw(20) << "Metric" 
              << std::right << std::setw(20) << "Value" 
              << std::right << std::setw(20) << "Unit" << "\n";
    std::cout << "  " << std::string(60, '-') << "\n";
    
    // Format buffer size for summary
    std::string buffer_size_str;
    if (results.buffer_size_bytes < 1024) {
        buffer_size_str = std::to_string(results.buffer_size_bytes) + " B";
    } else if (results.buffer_size_bytes < 1024 * 1024) {
        buffer_size_str = std::to_string(results.buffer_size_bytes / 1024) + " KB";
    } else {
        buffer_size_str = std::to_string(results.buffer_size_bytes / (1024 * 1024)) + " MB";
    }
    
    std::cout << "  " << std::left << std::setw(20) << "Buffer Size" 
              << std::right << std::setw(20) << buffer_size_str 
              << std::right << std::setw(20) << "" << "\n";
    
    std::cout << "  " << std::left << std::setw(20) << "Iterations" 
              << std::right << std::setw(20) << results.iterations 
              << std::right << std::setw(20) << "" << "\n";
    
    std::cout << "  " << std::left << std::setw(20) << "Total Time" 
              << std::right << std::setw(20) << std::fixed << std::setprecision(6) 
              << results.timing.total_time_seconds 
              << std::right << std::setw(20) << "seconds" << "\n";
    
    std::cout << "  " << std::left << std::setw(20) << "Min Latency" 
              << std::right << std::setw(20) << std::fixed << std::setprecision(2) 
              << results.timing.min_latency_ns 
              << std::right << std::setw(20) << "ns" << "\n";
    
    std::cout << "  " << std::left << std::setw(20) << "Max Latency" 
              << std::right << std::setw(20) << std::fixed << std::setprecision(2) 
              << results.timing.max_latency_ns 
              << std::right << std::setw(20) << "ns" << "\n";
    
    std::cout << "  " << std::left << std::setw(20) << "Avg Latency" 
              << std::right << std::setw(20) << std::fixed << std::setprecision(2) 
              << results.timing.avg_latency_ns 
              << std::right << std::setw(20) << "ns" << "\n";
    
    if (results.timing.sample_count > 1) {
        std::cout << "  " << std::left << std::setw(20) << "Std Deviation" 
                  << std::right << std::setw(20) << std::fixed << std::setprecision(2) 
                  << results.timing.std_deviation_ns 
                  << std::right << std::setw(20) << "ns" << "\n";
    }
    
    std::cout << "  " << std::left << std::setw(20) << "Throughput" 
              << std::right << std::setw(20) << std::fixed << std::setprecision(2) 
              << results.throughput_mbps 
              << std::right << std::setw(20) << "MB/s" << "\n";
    
    std::cout << "  " << std::left << std::setw(20) << "Verification" 
              << std::right << std::setw(20) << (results.verification_passed ? "PASSED" : "FAILED")
              << std::right << std::setw(20) << "" << "\n";
    
    std::cout << "  " << std::string(60, '-') << "\n";
    std::cout << "\n";
}

