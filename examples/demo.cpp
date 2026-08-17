#include <kep_alloc/kep_alloc.hpp>
#include <iostream>

int main() {
    std::cout << "Demo running..." << std::endl;

    kep_alloc::BumpAllocator allocator(1024);

    void* ptr1 = allocator.allocate(128);
    void* ptr2 = allocator.allocate(256);

    allocator.free(ptr1);
    allocator.free(ptr2);

    allocator.reset();

    return 0;
}