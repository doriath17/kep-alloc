#pragma once
#include "../utils.hpp"

#include <concepts>
#include <cstddef>
#include <sys/mman.h>
#include <type_traits>
#include <unistd.h>

namespace kep_alloc::internal {

/**
 * @brief Backing system policy to allocate memory.
 *
 * # Notes
 * Allocators can define a policy to allocate/request memory. This can be achieved in
 * two main ways:
 * - inheritance (dynamic polymorphism):
 * you can implement an interface like this:
 * @code{.cpp}
 * class IBackingPolicy {
 * public:
 *      virtual ~IBackingPolicy() = default;
 *      [[nodiscard]] virtual void* allocate_chunk(std::size_t bytes) = 0;
 *      virtual void deallocate_chunk(void* ptr, std::size_t bytes) = 0;
 * };
 * @endcode
 * However this will add some runtime overhead due to the vtable lookup when calling the virtual
 * functions.
 *
 * - templates (static polymorphism):
 * this is the actual approach i choose. Templates are resolved at compile time so there is no
 * overhead at runtime, so you can achieve the same benefits of the virtual inheritance but with
 * actual zero costs. Furthermore, in modern C++ (C++ 20) there is also the possibility to create
 * `concepts` and actually enforce that a certain template type parameter implement that concept:
 * @code{.cpp}
 * template <BackingPolicy Backing = SystemPageBacking>
 * @endcode
 *
 * ### List of backing systems:
 * - SystemPageBacking (default)
 * - ArenaBacking
 * - StaticBufferBacking
 * - MallocBacking
 * - HugePageBacking
 */
template <typename T> concept BackingPolicy = requires(T backing, size_t bytes, void* ptr) {
    { backing.allocate_chunk(bytes) } noexcept -> std::convertible_to<void*>;
    { backing.deallocate_chunk(ptr, bytes) } noexcept -> std::same_as<void>;
};

constexpr size_t DEFAULT_PAGE_SIZE = 4096; // Default to 4KB if sysconf fails

/**
 * Fetches the system's page size using sysconf(_SC_PAGE_SIZE).
 * This function is executed only once the first time it is called, and the result is cached for
 * subsequent calls.
 *
 * @return The system's page size in bytes. If sysconf fails, it defaults to 4096 bytes (4KB).
 */
[[nodiscard]] inline size_t get_page_size() noexcept {
    // This pattern is called an IIFE (Immediately Invoked Function Expression) or an IILE
    // (Immediately Invoked Lambda Expression). the variable page_size is thread safe Note that
    // without a lambda the expression would lose the constness of the variable and would not be
    // thread safe.
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

/**
 * @brief Normalizes a requested size up to the nearest multiple of the OS page size.
 *
 * Returns the closest multiple of the page size that is greater than or equal to the
 * requested size. This ensures sufficient memory is allocated and complies with
 * POSIX system call alignment constraints.
 *
 * @note From the POSIX `mmap` specification:
 * > *offset must be a multiple of the page size as returned by sysconf(_SC_PAGE_SIZE)*
 *
 * ### Mathematical Formula
 * @code{.cpp}
 * (requested_size + page_size - 1) & ~(page_size - 1)
 * @endcode
 *
 * ### Bitwise Formula Explanation
 * Since `page_size` is always a power of two (\f$2^N\f$), its binary representation
 * has a single '1' bit at position \f$N\f$, with all lower bits set to '0' (representing
 * the offset within a page).
 *
 * **Example:** `req_size` = 12,841 B, `page_size` = 4,096 B (4 KB, \f$N = 12\f$)
 *
 * 1. **Inputs:**
 * - `req_size`               = 12841 B  -> `0011 0010 0010 1001`
 * - `page_size - 1`          =  4095 B  -> `0000 1111 1111 1111` (Offset mask)
 * - `~(page_size - 1)`                  -> `1111 0000 0000 0000` (Page boundary mask)
 *
 * 2. **Add `(page_size - 1)`** to push non-exact sizes into the next page:
 * - `12841 + 4095`           = 16936 B  -> `0100 0010 0010 1000`
 *
 * 3. **Bitwise AND** with alignment mask to clear offset bits:
 * - `16936 & ~(4095)`        = 16384 B  -> `0100 0000 0000 0000` (16 KB = 4 pages)
 *
 * @param[in] req_size The requested allocation size in bytes.
 * @return The page-aligned size (greater than or equal to `req_size`).
 */
[[nodiscard]] inline size_t normalize_size(size_t req_size) noexcept {
    return align(req_size, get_page_size());
}

class SystemPageBacking {
  public:
    [[nodiscard]] inline void* allocate_chunk(size_t bytes) noexcept {
        if (bytes == 0) [[unlikely]] {
            return nullptr;
        }

        // TODO: maybe this is done automatically by mmap
        auto norm_size = normalize_size(bytes);

        void* chunk_ptr =
            mmap(NULL, norm_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (chunk_ptr == MAP_FAILED) [[unlikely]] {
            return nullptr;
        }

        return chunk_ptr;
    }

    void inline deallocate_chunk(void* ptr, size_t bytes) noexcept {
        if (ptr == nullptr || bytes == 0) [[unlikely]] {
            return;
        }

        auto norm_size = normalize_size(bytes);
        munmap(ptr, norm_size);
    }
};
} // namespace kep_alloc::internal
