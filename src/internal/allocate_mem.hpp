#pragma once
#include "system_info.hpp"

#include <cstddef>

namespace kep_alloc::internal {

/**
 * Allocates memory of the given normalized size.
 *
 * @param norm_size The normalized size of memory to allocate.
 * @return A pointer to the allocated memory, or nullptr if allocation fails.maximum port
 */
void* allocate_mem(size_t norm_size);

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
    static const size_t offset_mask = get_page_size() - 1;
    static const size_t page_boundary_mask = ~offset_mask;
    return (req_size + offset_mask) & page_boundary_mask;
}

} // namespace kep_alloc::internal