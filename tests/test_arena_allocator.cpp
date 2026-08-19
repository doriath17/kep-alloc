#include <kep_alloc/kep_alloc.hpp>

#include <gtest/gtest.h>

using namespace kep_alloc;
using namespace kep_alloc::internal;

namespace kep_alloc::testing {

// 1 Point3D is 12-Bytes
struct Point3D {
    float x, y, z;

    Point3D(float x_val, float y_val, float z_val) : x(x_val), y(y_val), z(z_val) {}
};

Point3D random_point() {
    return Point3D(static_cast<float>(rand()) / RAND_MAX, static_cast<float>(rand()) / RAND_MAX,
                   static_cast<float>(rand()) / RAND_MAX);
}

Point3D* allocate_point(kep_alloc::ArenaAllocator<kep_alloc::internal::SystemPageBacking> &arena, float x = 0, float y = 0, float z = 0) {
    Point3D* p = static_cast<Point3D*>(arena.allocate(sizeof(Point3D), alignof(Point3D)));
    EXPECT_NE(p, nullptr);
    new (p) Point3D(x, y, z); // Placement new to construct
    return p;
}

TEST(ArenaAllocatorTest, AllocateAndReset) {
    kep_alloc::internal::SystemPageBacking backing_policy;
    kep_alloc::ArenaAllocator allocator(backing_policy, KB * 4); // 4KB arena

    void* ptr1 = allocator.allocate(128);
    EXPECT_NE(ptr1, nullptr);

    void* ptr2 = allocator.allocate(256);
    EXPECT_NE(ptr2, nullptr);

    allocator.reset();

    void* ptr3 = allocator.allocate(512);
    EXPECT_NE(ptr3, nullptr);
}

TEST(ArenaAllocatorTest, MarkerIsCorrect) {
    kep_alloc::internal::SystemPageBacking backing_policy;
    kep_alloc::ArenaAllocator allocator(backing_policy, KB * 4); // 4KB arena

    auto p0 = allocate_point(allocator);
    auto marker_after_p0 = allocator.get_marker();

    EXPECT_EQ(marker_after_p0, sizeof(Point3D));
}

} // namespace kep_alloc::testing