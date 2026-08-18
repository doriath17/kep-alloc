#pragma once

#include <cstddef>
#include <unistd.h>

namespace kep_alloc::internal {

constexpr size_t DEFAULT_PAGE_SIZE = 4096; // Default to 4KB if sysconf fails

/**
 * Fetches the system's page size using sysconf(_SC_PAGE_SIZE).
 * This function is executed only once the first time it is called, and the result is cached for subsequent calls.
 * 
 * @return The system's page size in bytes. If sysconf fails, it defaults to 4096 bytes (4KB).
 */
[[nodiscard]] inline size_t get_page_size() noexcept {
    // This pattern is called an IIFE (Immediately Invoked Function Expression) or an IILE (Immediately Invoked Lambda Expression).
    // the variable page_size is thread safe
    // Note that without a lambda the expression would lose the constness of the variable and would not be thread safe. 
    // TODO: understand why the lambda is needed to make the variable thread safe.
    // Since C++11 static local static variables are guaranteed to be thread-safe.
    static size_t page_size = []() -> size_t {
        long size = sysconf(_SC_PAGE_SIZE);
        if (size == -1) {
            return DEFAULT_PAGE_SIZE; // Fallback to default if sysconf fails
        }
        return static_cast<size_t>(size);
    }();
    return page_size;
}

} // namespace kep_alloc::internal