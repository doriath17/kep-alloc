#include "bump_allocator.hpp"
#include <iostream>

namespace kep_alloc {
    BumpAllocator::BumpAllocator(size_t size) {

    }

    BumpAllocator::~BumpAllocator() {

    }

    void* BumpAllocator::allocate(size_t size) {
        std::cout << "BumpAllocator::allocate called with size: " << size << std::endl;
        return nullptr;
    }

    void BumpAllocator::free(void* ptr) {

    }

    void BumpAllocator::reset() {

    }

}