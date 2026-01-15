# System Memory & Network Performance Measurement Engine

A user-space C/C++ performance measurement tool designed to benchmark RAM behavior and compare device-side processing time with real network latency. This tool focuses on measurement, timing, and validation, not hardware modification.

The tool is intended as a proof-of-concept / demo engine that can be extended for further experimentation.

## Table of Contents

- [Features](#features)
- [Non-Goals](#non-goals)
- [Supported Platforms](#supported-platforms)
- [Build Requirements](#build-requirements)
- [Build Instructions](#build-instructions)
- [Usage](#usage)
- [Project Structure](#project-structure)
- [Design Principles](#design-principles)
- [Status](#status)
- [License](#license)

## Features

- **RAM read / write / read verification** - Comprehensive memory benchmarking
- **High-resolution timing and throughput measurement** - Nanosecond-precision timing
- **Structured result tables** - Latency, size, and iteration count metrics
- **Best-effort process prioritization** - Within OS limits
- **Network loop measurements** - Connection open and round-trip latency
- **Clean, portable, standards-compliant C++** - C++17 standard

## Non-Goals

**Important:** This project does not:

- ❌ Modify CPU frequency
- ❌ Increase physical RAM
- ❌ Access hardware pins or chips directly
- ❌ Modify bootloader or kernel
- ❌ Perform compression-based memory expansion

All measurements are performed in user space, within OS and API constraints.

## Supported Platforms

- ✅ **Linux** (Ubuntu recommended)
- ✅ **Android** (user-space only)
- ⚠️ **macOS / Windows** (development only; Linux recommended for testing)

## Build Requirements

- CMake ≥ 3.10
- GCC or Clang (with C++17 support)
- POSIX-compatible system

### Install Dependencies on Ubuntu

```bash
sudo apt update
sudo apt install -y build-essential cmake clang
```

## Build Instructions

```bash
git clone <repository-url>
cd <project-directory>

mkdir build
cd build
cmake ..
make -j
```

The build should complete without warnings or errors.

**Alternative build method:**

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

**For cross-compilation (e.g., Android):**

```bash
cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/android.toolchain.cmake ..
cmake --build .
```

## Usage

### Basic Run

```bash
./SystemBenchmark
```

### Command-Line Options

```bash
./SystemBenchmark [OPTIONS]
```

**Available Options:**

- `--buffer-size SIZE` - Buffer size in bytes (default: 1048576 = 1MB)
- `--iterations COUNT` - Number of iterations (default: 1000)
- `--network-host HOST` - Run network benchmark (hostname or IP)
- `--network-port PORT` - Network benchmark port (default: 80)
- `--help` - Show help message

### Examples

```bash
# Basic memory benchmark
./SystemBenchmark

# Custom buffer size and iterations
./SystemBenchmark --buffer-size 1048576 --iterations 10000

# Large buffer test
./SystemBenchmark --buffer-size 10485760 --iterations 1000000

# Network benchmark
./SystemBenchmark --network-host 127.0.0.1 --network-port 80

# Combined memory and network benchmark
./SystemBenchmark --buffer-size 1048576 --iterations 10000 --network-host example.com --network-port 80
```

### Output

The tool provides:

- RAM timing statistics
- Verification results
- Network latency comparison (if enabled)
- Platform information
- Timer resolution measurements

## Project Structure

```
.
├── CMakeLists.txt
├── README.md
├── include/
│   ├── timer.h
│   ├── memory_benchmark.h
│   ├── network_benchmark.h
│   └── process_priority.h
├── src/
│   ├── main.cpp
│   ├── timer.cpp
│   ├── memory_benchmark.cpp
│   ├── network_benchmark.cpp
│   └── process_priority.cpp
└── build/          # Build directory (created during build)
```

## Design Principles

- **Clarity over cleverness** - Readable and maintainable code
- **Portable, standard-compliant C++** - C++17 standard
- **No undefined behavior** - Strict compiler warnings enabled
- **Repeatable and explainable measurements** - Consistent benchmarking methodology
- **Clean handoff to other engineers** - Well-documented and structured

## Status

- ✅ **Milestone 1:** Project setup and timing utilities
- ✅ **Milestone 2:** Memory benchmarking
- ✅ **Milestone 3:** Result tables and stability metrics
- ✅ **Milestone 4:** Resource prioritization
- ✅ **Milestone 5:** Network timing loop

## License

To be defined.

## Notes

This project is intended for analysis and demonstration purposes. Any performance improvements observed are the result of measurement and optimization, not hardware alteration.
