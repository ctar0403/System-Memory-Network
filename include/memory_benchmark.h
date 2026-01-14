#ifndef MEMORY_BENCHMARK_H
#define MEMORY_BENCHMARK_H

#include <cstdint>
#include <cstddef>

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
};

#endif // MEMORY_BENCHMARK_H

