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



} // namespace kep_alloc::internal