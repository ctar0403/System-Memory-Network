/**
 * process_priority.h - Process priority adjustment (Linux)
 * 
 * Best-effort attempt to raise process priority for more consistent
 * benchmark timing. Requires elevated privileges for full effect.
 */

#ifndef PROCESS_PRIORITY_H
#define PROCESS_PRIORITY_H

#include <cstdint>

/**
 * Process Priority Management Module
 * 
 * Provides best-effort process priority adjustment for benchmarking.
 * Attempts to raise process priority where permitted by the system.
 * Gracefully handles cases where privileges are insufficient.
 * 
 * Example usage:
 *   ProcessPriority priority;
 *   bool success = priority.attempt_raise();
 *   if (success) {
 *       std::cout << "Priority raised successfully\n";
 *   }
 */
class ProcessPriority {
public:
    /**
     * Priority adjustment result.
     */
    enum class Result {
        Success,           // Priority was successfully raised
        InsufficientPrivs, // Insufficient privileges (normal user)
        NotSupported,      // Platform does not support priority adjustment
        Error              // Other error occurred
    };

    /**
     * Constructs a process priority manager.
     */
    ProcessPriority() noexcept;

    /**
     * Attempts to raise the current process priority.
     * This is a best-effort operation and will not fail if privileges
     * are insufficient - the process will continue with default priority.
     * 
     * @return Result indicating success or reason for failure
     */
    Result attempt_raise() noexcept;

    /**
     * Gets the current process priority (nice value).
     * 
     * @return Current nice value, or 0 if not available
     */
    std::int32_t get_current_priority() const noexcept;

    /**
     * Converts a Result to a human-readable string.
     * 
     * @param result The result to convert
     * @return String description of the result
     */
    static const char* result_to_string(Result result) noexcept;

private:
    /**
     * Platform-specific implementation to raise priority.
     * 
     * @return Result indicating success or failure
     */
    Result raise_priority_impl() noexcept;

    /**
     * Platform-specific implementation to get current priority.
     * 
     * @return Current nice value, or 0 if not available
     */
    std::int32_t get_current_priority_impl() const noexcept;
};

#endif // PROCESS_PRIORITY_H

