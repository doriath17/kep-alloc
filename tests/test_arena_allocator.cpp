#include <kep_alloc/kep_alloc.hpp>

#include <gtest/gtest.h>

using namespace kep_alloc;
using namespace kep_alloc::internal;

namespace kep_alloc::testing {

// 1 Point2D is 12-Bytes
struct Point2D {
    float x, y;

    Point2D(float x_val, float y_val) : x(x_val), y(y_val) {}
};

Point2D random_point() {
    return Point2D(static_cast<float>(rand()) / RAND_MAX, static_cast<float>(rand()) / RAND_MAX);
}

Point2D*
allocate_point(kep_alloc::ArenaAllocator<kep_alloc::internal::SystemPageBackingStorage>& arena,
               float x = 0, float y = 0) {
    Point2D* p = static_cast<Point2D*>(arena.allocate(sizeof(Point2D), alignof(Point2D)));
    EXPECT_NE(p, nullptr);
    new (p) Point2D(x, y); // Placement new to construct
    return p;
}

TEST(ArenaAllocatorTest, AllocateAndReset) {
    kep_alloc::internal::SystemPageBackingStorage backing_policy;
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
    kep_alloc::internal::SystemPageBackingStorage backing_policy;
    kep_alloc::ArenaAllocator allocator(backing_policy, KB * 4); // 4KB arena

    auto p0 = allocate_point(allocator);
    auto marker_after_p0 = allocator.get_marker();

    EXPECT_EQ(marker_after_p0, sizeof(Point2D));

    auto p1 = allocate_point(allocator);
    auto marker_after_p1 = allocator.get_marker();
    EXPECT_EQ(marker_after_p1, sizeof(Point2D) * 2);

    // for a 4KB arena and with sizeof(Point2D) =  8 bytes, you can allocate, in addition to the two
    // points already allocated, 510 more points (for a total of 512 points) before running out of
    // space.
    auto marker_before_last_allocation = allocator.get_marker();
    for (int i = 0; i < 510; ++i) {
        allocate_point(allocator);
    }

    auto marker_after_last_allocation = allocator.get_marker();
    EXPECT_EQ(marker_after_last_allocation, marker_before_last_allocation + sizeof(Point2D) * 510);

    // allocate after running out of space should return nullptr
    void* ptr_out_of_space = allocator.allocate(sizeof(Point2D));
    EXPECT_EQ(ptr_out_of_space, nullptr);
}

TEST(ArenaAllocatorTest, RewindToMarker) {
    kep_alloc::internal::SystemPageBackingStorage backing_policy;
    kep_alloc::ArenaAllocator allocator(backing_policy, KB * 4); // 4KB arena

    auto p0 = allocate_point(allocator);
    auto marker_after_p0 = allocator.get_marker();

    auto p1 = allocate_point(allocator);
    auto marker_after_p1 = allocator.get_marker();

    allocator.rewind_to(marker_after_p0);

    auto marker_after_rewind = allocator.get_marker();
    EXPECT_EQ(marker_after_rewind, marker_after_p0);

    // After rewinding, we should be able to allocate again
    auto p2 = allocate_point(allocator);
    EXPECT_NE(p2, nullptr);
}

} // namespace kep_alloc::testing