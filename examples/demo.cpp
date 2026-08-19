#include <kep_alloc/kep_alloc.hpp>
#include <iostream>

using namespace kep_alloc;

int main() {
    std::cout << "Demo running..." << std::endl;

    kep_alloc::internal::SystemPageBacking backing_policy;
    kep_alloc::ArenaAllocator allocator(backing_policy, 1024);

    void* ptr1 = allocator.allocate(128);
    void* ptr2 = allocator.allocate(256);

    allocator.deallocate(ptr1);
    allocator.deallocate(ptr2);

    allocator.reset();

    return 0;
}