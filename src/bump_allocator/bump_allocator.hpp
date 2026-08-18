#pragma once

#include <cstddef>

namespace kep_alloc {

class BumpAllocator {
  public:
    BumpAllocator(size_t size);
    ~BumpAllocator();

    void* allocate(size_t size);
    void free(void* ptr);
    void reset();
};

} // namespace kep_alloc