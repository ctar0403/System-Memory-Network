/**
 * demo_config.h - Preset configurations for benchmarks
 * 
 * Provides safe defaults for mobile/demo environments and
 * full-power settings for desktop/server testing.
 */

#ifndef DEMO_CONFIG_H
#define DEMO_CONFIG_H

#include <cstddef>

/**
 * Demo Mode Configuration
 * 
 * Provides safe defaults for mobile/demo environments.
 * Use these values when running on resource-constrained devices.
 */
namespace DemoConfig {
    // Memory benchmark defaults
    constexpr std::size_t DEMO_BUFFER_SIZE = 1024 * 1024;      // 1 MB (safe for mobile)
    constexpr std::size_t DEMO_ITERATIONS = 100;                // Shorter for quick demo
    constexpr std::size_t DEMO_CONTINUOUS_RUNS = 5;             // Limited runs
    
    // CPU benchmark defaults
    constexpr std::size_t DEMO_CPU_ITERATIONS = 10000;          // Reduced for mobile
    
    // Network benchmark defaults (if applicable)
    constexpr std::size_t DEMO_NETWORK_ITERATIONS = 3;          // Minimal network tests
    constexpr std::size_t DEMO_NETWORK_PAYLOAD = 512;           // Small payload
    
    // Full benchmark defaults (for desktop/server)
    constexpr std::size_t FULL_BUFFER_SIZE = 10 * 1024 * 1024; // 10 MB
    constexpr std::size_t FULL_ITERATIONS = 1000;               // More iterations
    constexpr std::size_t FULL_CPU_ITERATIONS = 1000000;        // Full CPU test
}

#endif // DEMO_CONFIG_H
