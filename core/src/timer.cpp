/**
 * timer.cpp - High-resolution timer implementation
 */

#include "timer.h"
#include <chrono>

Timer::Timer() noexcept : started_(false) {
}

void Timer::start() noexcept {
    start_time_ = std::chrono::steady_clock::now();
    started_ = true;
}

double Timer::elapsed_seconds() const noexcept {
    if (!started_) {
        return 0.0;
    }
    
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
        now - start_time_
    );
    return static_cast<double>(duration.count()) / 1'000'000'000.0;
}

std::int64_t Timer::elapsed_nanoseconds() const noexcept {
    if (!started_) {
        return 0;
    }
    
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
        now - start_time_
    );
    return duration.count();
}

double Timer::elapsed_milliseconds() const noexcept {
    if (!started_) {
        return 0.0;
    }
    
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
        now - start_time_
    );
    return static_cast<double>(duration.count()) / 1'000'000.0;
}

