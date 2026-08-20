#pragma once
#include <cstddef>

namespace kep_alloc::internal {

// common sizes
constexpr size_t KB = 1024;
constexpr size_t MB = 1024 * KB;
constexpr size_t GB = 1024 * MB;

/**
 * @brief Aligns a given value to the next boundary based on the specified alignment.
 *
 * Formula used:
 * `val + offset_mask) & boundary_mask`
 * where:
 * - `offset_mask` = `alignment - 1`
 * - `boundary_mask` = `~offset_mask``
 *
 * @param val The raw value to align.
 * @param alignment The alignment boundary.
 * @return The next boundary-aligned value.
 */
inline size_t align(size_t val, size_t alignment) noexcept {
    size_t offset_mask = alignment - 1;
    size_t boundary_mask = ~offset_mask;
    return (val + offset_mask) & boundary_mask;
}

/**
 * @brief Checks if a given raw value is aligned to a specified boundary.
 *
 * Alignment values are always powers of two, so this function checks efficiently if
 * the value is aligned to the specified boundary using a bitwise AND operation.
 *
 * @param val The raw value to check.
 * @param alignment The alignment boundary.
 * @return true if `val` is aligned to `alignment`, false otherwise.
 *
 */
inline bool is_aligned(size_t val, size_t alignment) noexcept {
    return (val & (alignment - 1)) == 0;
}

inline size_t check_and_align(size_t val, size_t alignment) noexcept {
    if (!is_aligned(val, alignment)) {
        val = internal::align(val, alignment);
    }
}

} // namespace kep_alloc::internal