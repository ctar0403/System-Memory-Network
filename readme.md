# System Performance Benchmark

A portable C++17 benchmarking suite for measuring memory, CPU, and network performance in user space.

## Features

- **Memory Benchmark**: RAM read/write/verify with latency statistics
- **CPU Benchmark**: Computational performance testing (integer, float, memory ops)
- **Network Benchmark**: Connection timing and round-trip latency (Linux only)
- **High-Resolution Timing**: Nanosecond-precision measurements
- **Cross-Platform**: Linux, macOS, iOS (core library)

## Project Structure

```
core/                   # Portable benchmark library (iOS-compatible)
├── include/           # Public headers
└── src/               # Implementation

platform/cli/          # Linux CLI application
├── main.cpp          # Entry point
├── network_benchmark.* # POSIX network timing
└── process_priority.*  # Linux process priority

docs/                  # Documentation
└── iOS_INTEGRATION.md # iOS integration guide
```

## Build Instructions

### Linux/Ubuntu

```bash
# Install dependencies
sudo apt install build-essential cmake

# Build
mkdir build && cd build
cmake ..
make -j

# Run
./platform/cli/SystemBenchmark --help
```

### macOS

```bash
brew install cmake
mkdir build && cd build
cmake ..
make -j
./platform/cli/SystemBenchmark
```

### iOS

See `docs/iOS_INTEGRATION.md` for Xcode integration instructions.

## Usage

```bash
# Memory benchmark (1MB buffer, 1000 iterations)
./SystemBenchmark --buffer-size 1048576 --iterations 1000

# CPU benchmark
./SystemBenchmark --cpu-iterations 100000

# Network benchmark (Linux only)
./SystemBenchmark --network-host 127.0.0.1 --network-port 80 --network-iterations 10

# Combined test
./SystemBenchmark --buffer-size 1048576 --iterations 1000 --cpu-iterations 100000
```

## Demo Mode

For mobile or quick testing, use demo defaults:

```cpp
#include "demo_config.h"
#include "memory_benchmark.h"

MemoryBenchmark bench;
auto results = bench.run(
    DemoConfig::DEMO_BUFFER_SIZE,    // 1 MB
    DemoConfig::DEMO_ITERATIONS       // 100 iterations
);
```

## API Example

```cpp
#include "memory_benchmark.h"
#include "cpu_benchmark.h"

// Memory benchmark
MemoryBenchmark mem_bench;
auto mem_results = mem_bench.run(1024 * 1024, 1000);

std::cout << "Avg Latency: " << mem_results.timing.avg_latency_ns << " ns\n";
std::cout << "Throughput: " << mem_results.throughput_mbps << " MB/s\n";

// CPU benchmark
CpuBenchmark cpu_bench;
auto cpu_results = cpu_bench.run(100000);

std::cout << "Ops/Second: " << cpu_results.timing.operations_per_second << "\n";
```

## Platform Support

| Feature | Linux | macOS | iOS |
|---------|-------|-------|-----|
| Memory Benchmark | ✓ | ✓ | ✓ |
| CPU Benchmark | ✓ | ✓ | ✓ |
| Network Benchmark | ✓ | Limited | ✗ |
| Process Priority | ✓ | Limited | ✗ |

## Requirements

- C++17 compiler (GCC 7+, Clang 5+, Apple Clang 10+)
- CMake 3.10+
- POSIX system (for CLI platform)

## Limitations

This tool performs measurements in user space only:
- Does not modify hardware settings
- Does not require root/elevated privileges
- Does not bypass OS security
- Results vary by system load and hardware

## License

To be defined.
