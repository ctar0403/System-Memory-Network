#include <iostream>
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
 *   --help                Show this help message
 * 
 * Examples:
 *   ./SystemBenchmark --buffer-size 1048576 --iterations 10000
 *   ./SystemBenchmark --buffer-size 10485760 --iterations 1000000
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
        double elapsed = timer.elapsed_nanoseconds();
        std::cout << "Timer Resolution: ~" << elapsed << " ns (test measurement)\n";
        
        std::cout << "\n";
    }
    
    void print_usage(const char* program_name) {
        std::cout << "Usage: " << program_name 
                  << " [--buffer-size SIZE] [--iterations COUNT] [--help]\n";
        std::cout << "\n";
        std::cout << "Options:\n";
        std::cout << "  --buffer-size SIZE    Buffer size in bytes (default: 1048576 = 1MB)\n";
        std::cout << "  --iterations COUNT    Number of iterations (default: 1000)\n";
        std::cout << "  --help                Show this help message\n";
        std::cout << "\n";
        std::cout << "Examples:\n";
        std::cout << "  " << program_name << " --buffer-size 1048576 --iterations 10000\n";
        std::cout << "  " << program_name << " --buffer-size 10485760 --iterations 1000000\n";
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
    
    std::cout << "Process Priority:\n";
    std::cout << "  Initial Priority: " << initial_priority << " (nice value)\n";
    std::cout << "  Priority Adjustment: " 
              << ProcessPriority::result_to_string(priority_result) << "\n";
    std::cout << "  Final Priority: " << final_priority << " (nice value)\n";
    
    if (priority_result == ProcessPriority::Result::Success) {
        std::cout << "  Note: Process priority raised for better benchmark accuracy.\n";
    } else if (priority_result == ProcessPriority::Result::InsufficientPrivs) {
        std::cout << "  Note: Running with default priority (requires root for higher priority).\n";
    } else if (priority_result == ProcessPriority::Result::NotSupported) {
        std::cout << "  Note: Priority adjustment not supported on this platform.\n";
    }
    std::cout << "\n";
    
    // Run memory benchmark if parameters provided, otherwise show help
    if (run_benchmark) {
        std::cout << "Running RAM Benchmark...\n";
        std::cout << "Buffer Size: " << buffer_size << " bytes\n";
        std::cout << "Iterations: " << iterations << "\n";
        std::cout << "\n";
        
        MemoryBenchmark benchmark;
        MemoryBenchmark::Results results = benchmark.run(buffer_size, iterations);
        MemoryBenchmark::print_results(results);
        
        return results.verification_passed ? EXIT_SUCCESS : EXIT_FAILURE;
    } else {
        std::cout << "Benchmarking framework initialized.\n";
        std::cout << "Use --help to see usage information.\n";
        std::cout << "\n";
        return EXIT_SUCCESS;
    }
}

