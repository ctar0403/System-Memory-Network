/**
 * memory_benchmark.h - RAM performance measurement
 * 
 * Measures memory read/write/verify performance with statistical analysis.
 * Provides latency, throughput, and stability metrics.
 */

#ifndef MEMORY_BENCHMARK_H
#define MEMORY_BENCHMARK_H

#include <cstdint>
#include <cstddef>
#include <vector>

/**
 * RAM Benchmarking Module
 * 
 * Performs read-write-read verification loops on allocated memory buffers.
 * Measures total time and average latency per operation.
 * 
 * Example usage:
 *   MemoryBenchmark benchmark;
 *   benchmark.run(buffer_size_bytes, iterations);
 */
class MemoryBenchmark {
public:
    /**
     * Timing statistics structure for latency measurements.
     */
    struct TimingStats {
        double min_latency_ns;
        double max_latency_ns;
        double avg_latency_ns;
        double total_time_seconds;
        double variance_ns;          // Variance of latency measurements
        double std_deviation_ns;      // Standard deviation of latency measurements
        std::size_t sample_count;     // Number of samples used for statistics
    };

    /**
     * Results structure containing benchmark metrics.
     */
    struct Results {
        std::size_t buffer_size_bytes;
        std::size_t iterations;
        TimingStats timing;
        double throughput_mbps;
        bool verification_passed;
        std::size_t verification_errors;
    };

    /**
     * Constructs a memory benchmark instance.
     */
    MemoryBenchmark() noexcept;

    /**
     * Runs the memory benchmark.
     * 
     * @param buffer_size_bytes Size of the buffer to allocate (in bytes)
     * @param iterations Number of read-write-read cycles to perform
     * @return Results structure with benchmark metrics
     */
    Results run(std::size_t buffer_size_bytes, std::size_t iterations);

    /**
     * Runs the memory benchmark in continuous/stability mode.
     * Executes multiple benchmark runs and tracks consistency across runs.
     * 
     * @param buffer_size_bytes Size of the buffer to allocate (in bytes)
     * @param iterations_per_run Number of read-write-read cycles per run
     * @param max_runs Maximum number of runs to execute (0 = unlimited, use duration)
     * @param max_duration_seconds Maximum duration in seconds (0 = unlimited, use max_runs)
     * @return Results structure with aggregated benchmark metrics and stability statistics
     */
    Results run_continuous(std::size_t buffer_size_bytes, 
                          std::size_t iterations_per_run,
                          std::size_t max_runs,
                          double max_duration_seconds);

    /**
     * Prints benchmark results in a clear table format.
     * 
     * @param results The benchmark results to print
     */
    static void print_results(const Results& results);

private:
    /**
     * Performs a single read-write-read verification cycle and measures its latency.
     * 
     * @param buffer Pointer to the buffer
     * @param size Size of the buffer in bytes
     * @param cycle_latency_ns Output parameter for cycle latency in nanoseconds
     * @return Number of verification errors (0 if all correct)
     */
    std::size_t verify_cycle(std::uint8_t* buffer, std::size_t size, 
                             std::int64_t& cycle_latency_ns) noexcept;

    /**
     * Writes a pattern to the buffer.
     * 
     * @param buffer Pointer to the buffer
     * @param size Size of the buffer in bytes
     */
    void write_pattern(std::uint8_t* buffer, std::size_t size) noexcept;

    /**
     * Verifies the pattern in the buffer.
     * 
     * @param buffer Pointer to the buffer
     * @param size Size of the buffer in bytes
     * @return Number of verification errors
     */
    std::size_t verify_pattern(const std::uint8_t* buffer, std::size_t size) const noexcept;

    /**
     * Runs the benchmark with a pre-allocated buffer (for deterministic execution).
     * 
     * @param buffer Pointer to pre-allocated buffer
     * @param buffer_size_bytes Size of the buffer in bytes
     * @param iterations Number of read-write-read cycles to perform
     * @return Results structure with benchmark metrics
     */
    Results run_with_buffer(std::uint8_t* buffer, 
                           std::size_t buffer_size_bytes, 
                           std::size_t iterations);

    /**
     * Calculates variance and standard deviation from a vector of latency values.
     * 
     * @param latencies Vector of latency measurements in nanoseconds
     * @param mean The mean value of latencies
     * @param variance Output parameter for variance
     * @param std_deviation Output parameter for standard deviation
     */
    static void calculate_statistics(const std::vector<double>& latencies,
                                    double mean,
                                    double& variance,
                                    double& std_deviation) noexcept;
};

#endif // MEMORY_BENCHMARK_H

