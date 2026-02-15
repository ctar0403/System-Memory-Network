cmake_minimum_required(VERSION 3.10)
project(SystemBenchmarkSuite VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build core library
add_subdirectory(core)

# Build CLI platform
add_subdirectory(platform/cli)

# Build instructions:
#   mkdir build && cd build
#   cmake ..
#   cmake --build .
#   ./platform/cli/SystemBenchmark
#
# For iOS integration:
#   Use core/ directory as a static library in Xcode
#   Link BenchmarkCore.a with your iOS app
