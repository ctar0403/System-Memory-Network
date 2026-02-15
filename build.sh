#!/bin/bash
#
# Build script for System Performance Benchmark
# Usage: ./build.sh [clean|rebuild]

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"

echo "System Performance Benchmark - Build Script"
echo "==========================================="
echo ""

# Handle command line arguments
if [ "$1" == "clean" ]; then
    echo "Cleaning build directory..."
    rm -rf "${BUILD_DIR}"
    echo "Clean complete."
    exit 0
fi

if [ "$1" == "rebuild" ]; then
    echo "Rebuilding from scratch..."
    rm -rf "${BUILD_DIR}"
fi

# Check for required tools
echo "Checking build dependencies..."

if ! command -v cmake &> /dev/null; then
    echo "ERROR: cmake not found. Install with:"
    echo "  sudo apt install cmake"
    exit 1
fi

if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
    echo "ERROR: No C++ compiler found. Install with:"
    echo "  sudo apt install build-essential"
    exit 1
fi

echo "✓ Build dependencies found"
echo ""

# Create build directory
if [ ! -d "${BUILD_DIR}" ]; then
    echo "Creating build directory..."
    mkdir -p "${BUILD_DIR}"
fi

# Configure with CMake
echo "Configuring with CMake..."
cd "${BUILD_DIR}"
cmake ..

# Build
echo ""
echo "Building..."
make -j$(nproc)

echo ""
echo "Build Complete"
echo "==========================================="
echo ""
echo "Run: cd ${BUILD_DIR} && ./platform/cli/SystemBenchmark --help"
echo ""
