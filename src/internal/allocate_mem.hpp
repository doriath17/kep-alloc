#pragma once
#include <cstddef>
#include "system_info.hpp"

namespace kep_alloc::internal {

/**
 * Allocates memory of the given normalized size.
 * 
 * @param norm_size The normalized size of memory to allocate.
 * @return A pointer to the allocated memory, or nullptr if allocation fails.maximum port
 */
void * allocate_mem(size_t norm_size);

/**
 * Normalizes the given size to a suitable allocation size (a multiple of the page size).
 * 
 * This function returns the closest multiple of the page size that is greater than or equal to the given size.
 * This is important to ensure that sufficient memory is allocated for the requested size and also
 * to comply with the requirements of the mmap system call:
 * From the man page of mmap:
 * > offset must be a multiple of the page size as returned by
 * > sysconf(_SC_PAGE_SIZE)
 * 
 * Formula used: (requested_size + page_size - 1) & ~(page_size - 1)
 * 
 * To understand the formula used to normalize the size, think about the
 * binary representation of req_size and page_size.
 *
 * Since page_size is always a power of two (2^N), its binary representation
 * has a single '1' bit at position N, with all lower bits set to '0' (these represent the offset within a page).
 *
 * Example: req_size = 12841 B, page_size = 4096 B (4 KB, N = 12)
 *
 * 1. Inputs:
 *    - req_size        = 12841 B -> 0011 0010 0010 1001
 *    - page_size - 1   =  4095 B -> 0000 1111 1111 1111 (Offset mask)
 *    - ~(page_size - 1)          -> 1111 0000 0000 0000 (Page boundary mask)
 *
 * 2. Add (page_size - 1) to push non-exact sizes into the next page:
 *    - 12841 + 4095 = 16936      -> 0100 0010 0010 1008
 *
 * 3. Bitwise AND with alignment mask to clear offset bits:
 *    - 16936 & ~(4095)           -> 0100 0000 0000 0000 = 16384 B (16 KB = 4 pages)
 * 
 * @param req_size The requested size of memory to normalize.
 * @return The normalized size, which is the closest multiple of the page size that is greater than or equal to the requested size.
 */
[[nodiscard]] inline size_t normalize_size(size_t req_size) noexcept {
    static const size_t offset_mask = get_page_size() - 1;
    static const size_t page_boundary_mask = ~offset_mask;
    return (req_size + offset_mask) & page_boundary_mask;
}

}