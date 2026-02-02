#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <string>
#include <cstdint>

#ifdef __linux__
#include <unistd.h>
#include <sys/utsname.h>
#endif

#include "timer.h"
#include "memory_benchmark.h"
#include "process_priority.h"
#include "network_benchmark.h"
#include "cpu_benchmark.h"

/**
 * System Benchmarking Tool
 * 
 * A portable user-space benchmarking utility for Unix-like systems.
 * 
 * Build Instructions:
 *   mkdir build && cd build
 *   cmake ..
 *   cmake --build .
 * 
 * Run Instructions:
 *   ./SystemBenchmark [--buffer-size SIZE] [--iterations COUNT]
 * 
 * Options:
 *   --buffer-size SIZE    Buffer size in bytes (default: 1048576 = 1MB)
 *   --iterations COUNT    Number of iterations (default: 1000)
 *   --cpu-iterations COUNT Run CPU benchmark with COUNT iterations
 *   --network-host HOST   Run network benchmark (hostname or IP)
 *   --network-port PORT   Network benchmark port (default: 80)
 *   --network-iterations COUNT Network benchmark iterations (default: 1)
 *   --continuous-runs COUNT Run benchmark in continuous mode for COUNT runs
 *   --continuous-duration SEC Run benchmark in continuous mode for SEC seconds
 *   --help                Show this help message
 * 
 * Examples:
 *   ./SystemBenchmark --buffer-size 1048576 --iterations 10000
 *   ./SystemBenchmark --buffer-size 10485760 --iterations 1000000
 *   ./SystemBenchmark --cpu-iterations 1000000
 *   ./SystemBenchmark --network-host 127.0.0.1 --network-port 80
 *   ./SystemBenchmark --network-host example.com --network-iterations 10
 * 
 * For Android (cross-compilation):
 *   cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/android.toolchain.cmake ..
 *   cmake --build .
 */

namespace {
    const char* VERSION = "1.0.0";
    
    void print_banner() {
        std::cout << "========================================\n";
        std::cout << "  System Benchmarking Tool v" << VERSION << "\n";
        std::cout << "  Portable C++ User-Space Benchmark\n";
        std::cout << "========================================\n";
        std::cout << "\n";
    }
    
    void print_environment_info() {
        std::cout << "Environment Information:\n";
        std::cout << "------------------------\n";
        
        // Compiler info
        #if defined(__GNUC__)
            std::cout << "Compiler: GCC " << __GNUC__ << "." << __GNUC_MINOR__ 
                      << "." << __GNUC_PATCHLEVEL__ << "\n";
        #elif defined(__clang__)
            std::cout << "Compiler: Clang " << __clang_major__ << "." 
                      << __clang_minor__ << "." << __clang_patchlevel__ << "\n";
        #else
            std::cout << "Compiler: Unknown\n";
        #endif
        
        // C++ standard
        std::cout << "C++ Standard: " << __cplusplus << "\n";
        
        // Platform info
        #ifdef __linux__
            struct utsname sys_info;
            if (uname(&sys_info) == 0) {
                std::cout << "System: " << sys_info.sysname << "\n";
                std::cout << "Release: " << sys_info.release << "\n";
                std::cout << "Machine: " << sys_info.machine << "\n";
            }
        #else
            std::cout << "Platform: Unix-like\n";
        #endif
        
        // Timer resolution test
        Timer timer;
        timer.start();
        // Small delay to test timer
        volatile int dummy = 0;
        for (int i = 0; i < 1000; ++i) {
            dummy += i;
        }
        (void)dummy;  // Suppress unused variable warning
        double elapsed = timer.elapsed_nanoseconds();
        std::cout << "Timer Resolution: ~" << elapsed << " ns (test measurement)\n";
        
        std::cout << "\n";
    }
    
    void print_usage(const char* program_name) {
        std::cout << "Usage: " << program_name 
                  << " [--buffer-size SIZE] [--iterations COUNT] "
                  << "[--network-host HOST] [--network-port PORT] [--help]\n";
        std::cout << "\n";
        std::cout << "Options:\n";
        std::cout << "  --buffer-size SIZE    Buffer size in bytes (default: 1048576 = 1MB)\n";
        std::cout << "  --iterations COUNT    Number of iterations (default: 1000)\n";
        std::cout << "  --network-host HOST   Run network benchmark (hostname or IP)\n";
        std::cout << "  --network-port PORT   Network benchmark port (default: 80)\n";
        std::cout << "  --network-iterations COUNT Network benchmark iterations (default: 1)\n";
        std::cout << "  --continuous-runs COUNT Run benchmark in continuous mode for COUNT runs\n";
        std::cout << "  --continuous-duration SEC Run benchmark in continuous mode for SEC seconds\n";
        std::cout << "  --help                Show this help message\n";
        std::cout << "\n";
        std::cout << "Examples:\n";
        std::cout << "  " << program_name << " --buffer-size 1048576 --iterations 10000\n";
        std::cout << "  " << program_name << " --buffer-size 10485760 --iterations 1000000\n";
        std::cout << "  " << program_name << " --network-host 127.0.0.1 --network-port 80\n";
        std::cout << "  " << program_name << " --network-host example.com --network-iterations 10\n";
        std::cout << "  " << program_name << " --buffer-size 1048576 --iterations 1000 --network-host 127.0.0.1\n";
        std::cout << "\n";
    }
    
    std::size_t parse_size_t(const char* str, const char* option_name) {
        try {
            unsigned long long value = std::stoull(str);
            if (value == 0) {
                std::cerr << "Error: " << option_name 
                          << " must be greater than 0\n";
                return 0;
            }
            return static_cast<std::size_t>(value);
        } catch (const std::exception& e) {
            std::cerr << "Error: Invalid value for " << option_name 
                      << ": " << str << "\n";
            return 0;
        }
    }
}

int main(int argc, char* argv[]) {
    // Default values
    std::size_t buffer_size = 1048576;  // 1 MB
    std::size_t iterations = 1000;
    bool run_benchmark = false;
    bool run_cpu_benchmark = false;
    std::size_t cpu_iterations = 100000;
    bool run_network_benchmark = false;
    std::string network_host;
    std::uint16_t network_port = 80;
    std::size_t network_iterations = 1;
    bool continuous_mode = false;
    std::size_t continuous_runs = 0;
    double continuous_duration = 0.0;
    
    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            print_banner();
            print_usage(argv[0]);
            return EXIT_SUCCESS;
        } else if (arg == "--buffer-size" && i + 1 < argc) {
            buffer_size = parse_size_t(argv[++i], "--buffer-size");
            if (buffer_size == 0) {
                return EXIT_FAILURE;
            }
            run_benchmark = true;
        } else if (arg == "--iterations" && i + 1 < argc) {
            iterations = parse_size_t(argv[++i], "--iterations");
            if (iterations == 0) {
                return EXIT_FAILURE;
            }
            run_benchmark = true;
        } else if (arg == "--cpu-iterations" && i + 1 < argc) {
            cpu_iterations = parse_size_t(argv[++i], "--cpu-iterations");
            if (cpu_iterations == 0) {
                return EXIT_FAILURE;
            }
            run_cpu_benchmark = true;
        } else if (arg == "--network-host" && i + 1 < argc) {
            network_host = argv[++i];
            run_network_benchmark = true;
        } else if (arg == "--network-port" && i + 1 < argc) {
            try {
                unsigned long port_value = std::stoul(argv[++i]);
                if (port_value == 0 || port_value > 65535) {
                    std::cerr << "Error: Port must be between 1 and 65535\n";
                    return EXIT_FAILURE;
                }
                network_port = static_cast<std::uint16_t>(port_value);
            } catch (const std::exception& e) {
                std::cerr << "Error: Invalid port number: " << argv[i] << "\n";
                return EXIT_FAILURE;
            }
        } else if (arg == "--network-iterations" && i + 1 < argc) {
            network_iterations = parse_size_t(argv[++i], "--network-iterations");
            if (network_iterations == 0) {
                return EXIT_FAILURE;
            }
        } else if (arg == "--continuous-runs" && i + 1 < argc) {
            continuous_runs = parse_size_t(argv[++i], "--continuous-runs");
            if (continuous_runs == 0) {
                return EXIT_FAILURE;
            }
            continuous_mode = true;
            run_benchmark = true;
        } else if (arg == "--continuous-duration" && i + 1 < argc) {
            try {
                continuous_duration = std::stod(argv[++i]);
                if (continuous_duration <= 0.0) {
                    std::cerr << "Error: Continuous duration must be greater than 0\n";
                    return EXIT_FAILURE;
                }
                continuous_mode = true;
                run_benchmark = true;
            } catch (const std::exception& e) {
                std::cerr << "Error: Invalid duration value: " << argv[i] << "\n";
                return EXIT_FAILURE;
            }
        } else {
            std::cerr << "Error: Unknown option: " << arg << "\n";
            std::cerr << "Use --help for usage information.\n";
            return EXIT_FAILURE;
        }
    }
    
    print_banner();
    print_environment_info();
    
    // Attempt to raise process priority (best-effort, non-blocking)
    ProcessPriority priority;
    std::int32_t initial_priority = priority.get_current_priority();
    ProcessPriority::Result priority_result = priority.attempt_raise();
    std::int32_t final_priority = priority.get_current_priority();
    
    std::cout << "Process Priority (Best-Effort):\n";
    std::cout << "  Initial Priority: " << initial_priority << " (nice value)\n";
    std::cout << "  Priority Adjustment: " 
              << ProcessPriority::result_to_string(priority_result) << "\n";
    std::cout << "  Final Priority: " << final_priority << " (nice value)\n";
    
    if (priority_result == ProcessPriority::Result::Success) {
        std::cout << "  Status: SUCCESS - Process priority raised for better benchmark accuracy.\n";
    } else if (priority_result == ProcessPriority::Result::InsufficientPrivs) {
        std::cout << "  Status: INSUFFICIENT PRIVILEGES - Running with default priority.\n";
        std::cout << "  Note: Root privileges required for higher priority (this is normal).\n";
    } else if (priority_result == ProcessPriority::Result::NotSupported) {
        std::cout << "  Status: NOT SUPPORTED - Priority adjustment not available on this platform.\n";
    } else {
        std::cout << "  Status: ERROR - Priority adjustment failed.\n";
    }
    std::cout << "\n";
    
    // Run memory benchmark if parameters provided
    double memory_latency_ns = 0.0;
    if (run_benchmark) {
        MemoryBenchmark benchmark;
        MemoryBenchmark::Results results;
        
        if (continuous_mode) {
            std::cout << "Running RAM Benchmark (Continuous/Stability Mode)...\n";
            std::cout << "Buffer Size: " << buffer_size << " bytes\n";
            std::cout << "Iterations per Run: " << iterations << "\n";
            if (continuous_runs > 0) {
                std::cout << "Mode: Continuous for " << continuous_runs << " runs\n";
            } else if (continuous_duration > 0.0) {
                std::cout << "Mode: Continuous for " << std::fixed << std::setprecision(2) 
                          << continuous_duration << " seconds\n";
            }
            std::cout << "Note: Tracking consistency and variance across runs.\n";
            std::cout << "\n";
            
            results = benchmark.run_continuous(buffer_size, iterations, 
                                              continuous_runs, continuous_duration);
        } else {
            std::cout << "Running RAM Benchmark (Single Run Mode)...\n";
            std::cout << "Buffer Size: " << buffer_size << " bytes\n";
            std::cout << "Iterations: " << iterations << "\n";
            std::cout << "\n";
            
            results = benchmark.run(buffer_size, iterations);
        }
        
        MemoryBenchmark::print_results(results);
        
        memory_latency_ns = results.timing.avg_latency_ns;
        
        if (!results.verification_passed) {
            return EXIT_FAILURE;
        }
    }

    // Run network benchmark if requested
    double cpu_time_per_op_ns = 0.0;
    if (run_cpu_benchmark) {
        std::cout << "Running CPU Benchmark...\n";
        std::cout << "Iterations: " << cpu_iterations << "\n";
        std::cout << "\n";

        CpuBenchmark cpu_benchmark;
        CpuBenchmark::Results cpu_results = cpu_benchmark.run(cpu_iterations);
        CpuBenchmark::print_results(cpu_results);
        
        cpu_time_per_op_ns = cpu_results.timing.time_per_operation_ns;

        if (!cpu_results.benchmark_successful) {
            std::cerr << "Warning: CPU benchmark failed to complete.\n";
        }
    }

    // Run network benchmark if requested
    if (run_network_benchmark) {
        if (network_host.empty()) {
            std::cerr << "Error: --network-host requires a hostname or IP address\n";
            return EXIT_FAILURE;
        }
        
        std::cout << "Running Network Benchmark...\n";
        std::cout << "Target: " << network_host << ":" << network_port << "\n";
        if (network_iterations > 1) {
            std::cout << "Iterations: " << network_iterations << " (call-like loop)\n";
        }
        std::cout << "\n";
        
        NetworkBenchmark network_benchmark;
        NetworkBenchmark::Results network_results;
        
        if (network_iterations > 1) {
            network_results = network_benchmark.run_call_loop(
                network_host, network_port, network_iterations, 1024);
        } else {
            network_results = network_benchmark.run(
                network_host, network_port, 1024);
        }
        
        NetworkBenchmark::print_results(network_results);
        
        // Print comparisons if other benchmarks were also run
        if (run_benchmark && memory_latency_ns > 0.0) {
            NetworkBenchmark::print_comparison(network_results, memory_latency_ns);
        }
        
        if (run_cpu_benchmark && cpu_time_per_op_ns > 0.0) {
            NetworkBenchmark::print_cpu_comparison(network_results, cpu_time_per_op_ns);
        }
        
        if (!network_results.benchmark_successful) {
            std::cerr << "Warning: Network benchmark failed. "
                      << "This may be due to network connectivity issues, "
                      << "firewall rules, or the target server not accepting connections.\n";
        }
    }
    
    if (!run_benchmark && !run_cpu_benchmark && !run_network_benchmark) {
        std::cout << "Benchmarking framework initialized.\n";
        std::cout << "Use --help to see usage information.\n";
        std::cout << "\n";
    }
    
    return EXIT_SUCCESS;
}

