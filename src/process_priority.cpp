#include "process_priority.h"

#ifdef __linux__
#include <unistd.h>
#include <sys/resource.h>
#include <cerrno>
#include <cstring>
#endif

ProcessPriority::ProcessPriority() noexcept {
}

ProcessPriority::Result ProcessPriority::attempt_raise() noexcept {
    return raise_priority_impl();
}

std::int32_t ProcessPriority::get_current_priority() const noexcept {
    return get_current_priority_impl();
}

const char* ProcessPriority::result_to_string(Result result) noexcept {
    switch (result) {
        case Result::Success:
            return "Success";
        case Result::InsufficientPrivs:
            return "Insufficient Privileges";
        case Result::NotSupported:
            return "Not Supported";
        case Result::Error:
            return "Error";
        default:
            return "Unknown";
    }
}

ProcessPriority::Result ProcessPriority::raise_priority_impl() noexcept {
#ifdef __linux__
    // Get current priority first
    errno = 0;
    int current_nice = getpriority(PRIO_PROCESS, 0);
    
    if (errno != 0) {
        // Error getting current priority
        return Result::Error;
    }

    // Attempt to set a higher priority (lower nice value)
    // We try to set nice value to -10, but this typically requires root
    // For normal users, we can only increase nice value (lower priority)
    // This is a best-effort operation - we try but don't require success
    
    // Try to set to -10 (high priority, requires root)
    errno = 0;
    int result = setpriority(PRIO_PROCESS, 0, -10);
    
    if (result == 0) {
        // Successfully set high priority (requires root privileges)
        return Result::Success;
    }
    
    // If that failed, check if it's a permission error
    if (errno == EPERM || errno == EACCES) {
        // Insufficient privileges - this is normal for non-root users
        // Normal users cannot decrease nice value (raise priority)
        // They can only increase nice value (lower priority)
        // If we're already at nice 0 or better, that's the best we can do
        if (current_nice <= 0) {
            // Already at best priority for non-root user
            return Result::Success;
        }
        
        // Try to set to 0 (default priority) - may fail if we can't decrease nice
        errno = 0;
        result = setpriority(PRIO_PROCESS, 0, 0);
        
        if (result == 0) {
            // Successfully set to default priority
            return Result::Success;
        }
        
        // Could not adjust priority - insufficient privileges
        return Result::InsufficientPrivs;
    }
    
    // Other error (e.g., invalid argument)
    return Result::Error;
#else
    // Platform not supported
    return Result::NotSupported;
#endif
}

std::int32_t ProcessPriority::get_current_priority_impl() const noexcept {
#ifdef __linux__
    errno = 0;
    int nice_value = getpriority(PRIO_PROCESS, 0);
    
    if (errno != 0) {
        // Error getting priority, return 0 as default
        return 0;
    }
    
    return static_cast<std::int32_t>(nice_value);
#else
    // Platform not supported, return 0 as default
    return 0;
#endif
}

