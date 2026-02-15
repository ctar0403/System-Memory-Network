cmake_minimum_required(VERSION 3.10)
project(SystemBenchmark VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Platform-specific sources
set(PLATFORM_SOURCES
    main.cpp
    network_benchmark.cpp
    process_priority.cpp
)

# Platform-specific headers
set(PLATFORM_HEADERS
    network_benchmark.h
    process_priority.h
)

# Include core library
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../core ${CMAKE_BINARY_DIR}/core)

# Create executable
add_executable(${PROJECT_NAME} ${PLATFORM_SOURCES} ${PLATFORM_HEADERS})

# Link with core library
target_link_libraries(${PROJECT_NAME} PRIVATE BenchmarkCore)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Strict compilation flags
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Werror)
