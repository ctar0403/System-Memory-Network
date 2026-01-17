#ifndef CPU_BENCHMARK_H
#define CPU_BENCHMARK_H

#include <cstdint>
#include <cstddef>

/**
 * CPU Benchmarking Module
 *
 * Performs CPU performance measurements using computational workloads.
 * Measures execution time for various CPU-intensive operations.
 *
 * Example usage:
 *   CpuBenchmark benchmark;
 *   benchmark.run(iterations);
 */
class CpuBenchmark {
public:
    /**
     * Timing statistics structure for CPU measurements.
     */
    struct TimingStats {
        double total_time_seconds;
        double operations_per_second;
        double time_per_operation_ns;
    };

    /**
     * Results structure containing CPU benchmark metrics.
     */
    struct Results {
        std::size_t iterations;
        TimingStats timing;
        std::string benchmark_type;
        bool benchmark_successful;
    };

    /**
     * Constructs a CPU benchmark instance.
     */
    CpuBenchmark() noexcept;

    /**
     * Runs the CPU benchmark.
     *
     * @param iterations Number of computational iterations to perform
     * @return Results structure with benchmark metrics
     */
    Results run(std::size_t iterations);

    /**
     * Prints CPU benchmark results in a clear table format.
     *
     * @param results The benchmark results to print
     */
    static void print_results(const Results& results);

private:
    /**
     * Performs CPU-intensive computation (integer operations).
     *
     * @param iterations Number of iterations
     * @return Computation result (to prevent optimization)
     */
    std::uint64_t compute_integer_workload(std::size_t iterations) noexcept;

    /**
     * Performs CPU-intensive computation (floating-point operations).
     *
     * @param iterations Number of iterations
     * @return Computation result (to prevent optimization)
     */
    double compute_float_workload(std::size_t iterations) noexcept;

    /**
     * Performs CPU-intensive computation (memory-bound operations).
     *
     * @param iterations Number of iterations
     * @return Computation result (to prevent optimization)
     */
    std::uint64_t compute_memory_workload(std::size_t iterations) noexcept;
};

#endif // CPU_BENCHMARK_H