cmake_minimum_required(VERSION 3.10)
project(BenchmarkCore VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Core library sources (platform-independent)
set(CORE_SOURCES
    src/timer.cpp
    src/memory_benchmark.cpp
    src/cpu_benchmark.cpp
)

# Core library headers
set(CORE_HEADERS
    include/timer.h
    include/memory_benchmark.h
    include/cpu_benchmark.h
)

# Create static library for core functionality
add_library(BenchmarkCore STATIC ${CORE_SOURCES} ${CORE_HEADERS})

# Include directories
target_include_directories(BenchmarkCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Strict compiler flags
target_compile_options(BenchmarkCore PRIVATE
    -Wall
    -Wextra
    $<$<NOT:$<PLATFORM_ID:Darwin>>:-Werror>
)

# Optimization flags
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(BenchmarkCore PRIVATE -O3)
endif()
