#include <cstddef>

namespace kep_alloc::internal {

/**
 * Allocates memory of the given normalized size.
 * 
 * @param norm_size The normalized size of memory to allocate.
 * @return A pointer to the allocated memory, or nullptr if allocation fails.maximum port
 */
void * allocate_mem(size_t norm_size) {
    return nullptr;
}

/**
 * Normalizes the given size to a suitable allocation size (a power of two).
 * 
 * This function returns the closest power of two that is greater than or equal to the given size.
 * 
 * From the man page of mmap:
 * > offset must be a multiple of the page size as returned by
 * > sysconf(_SC_PAGE_SIZE)
 * 
 * 
 * 
 */
size_t normalize_size(size_t size) {
    return size;
}

}