#ifndef TIMER_H
#define TIMER_H

#include <cstdint>
#include <chrono>

/**
 * High-resolution monotonic timer utility for benchmarking.
 * 
 * Uses std::chrono::steady_clock for portable, monotonic time measurement
 * across Unix-like systems (Linux/Android).
 * 
 * Example usage:
 *   Timer timer;
 *   timer.start();
 *   // ... code to measure ...
 *   double elapsed = timer.elapsed_seconds();
 */
class Timer {
public:
    /**
     * Constructs a timer. Does not start automatically.
     */
    Timer() noexcept;

    /**
     * Starts the timer. Can be called multiple times to reset.
     */
    void start() noexcept;

    /**
     * Returns elapsed time in seconds since start() was called.
     * Returns 0.0 if start() has not been called.
     */
    double elapsed_seconds() const noexcept;

    /**
     * Returns elapsed time in nanoseconds since start() was called.
     * Returns 0 if start() has not been called.
     */
    std::int64_t elapsed_nanoseconds() const noexcept;

    /**
     * Returns elapsed time in milliseconds since start() was called.
     * Returns 0.0 if start() has not been called.
     */
    double elapsed_milliseconds() const noexcept;

private:
    std::chrono::steady_clock::time_point start_time_;
    bool started_;
};

#endif // TIMER_H

