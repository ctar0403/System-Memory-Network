cmake_minimum_required(VERSION 3.10)
project(SystemBenchmark VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Strict compilation flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/main.cpp
    src/timer.cpp
)

# Executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Build instructions:
#   mkdir build && cd build
#   cmake ..
#   cmake --build .
#   ./SystemBenchmark
#
# For cross-compilation (e.g., Android):
#   cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/android.toolchain.cmake ..
#   cmake --build .

