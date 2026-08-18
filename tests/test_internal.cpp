#include "internal/backing/backing_policy.hpp"

#include <gtest/gtest.h>

namespace kep_alloc::internal::testing {

TEST(SystemInfoTest, GetPageSizeReturnsValidSize) {
    size_t page_size = get_page_size();
    EXPECT_GT(page_size, 0);
    EXPECT_GE(page_size, DEFAULT_PAGE_SIZE);
    EXPECT_EQ(page_size & (page_size - 1), 0); // Page size should be a multiple of two
}

TEST(SystemInfoTest, GetPageSizeIsConsistent) {
    size_t first_call = get_page_size();
    size_t second_call = get_page_size();
    EXPECT_EQ(first_call, second_call);
}

TEST(NormalizeSizeTest, ReturnsZeroForZeroInput) {
    size_t result = normalize_size(0);
    EXPECT_EQ(result, 0);
}

TEST(NormalizeSizeTest, ReturnsPageSizeForInputLessThanPageSize) {
    size_t page_size = get_page_size();
    size_t result = normalize_size(page_size - 1);
    EXPECT_EQ(result, page_size);
}

TEST(NormalizeSizeTest, ReturnsSufficientMemory) {
    size_t page_size = get_page_size();
    size_t result_lower_boundary = normalize_size(page_size + 1);
    size_t result_upper_boundary = normalize_size(page_size + page_size - 1);
    EXPECT_EQ(result_lower_boundary, page_size + page_size);
    EXPECT_EQ(result_upper_boundary, page_size + page_size);
}

TEST(SystemPageBackingTest, AllocateChunkReturnsNonNullForValidSize) {
    SystemPageBacking backing;
    size_t page_size = get_page_size();
    void* ptr = backing.allocate_chunk(page_size);
    EXPECT_NE(ptr, nullptr);
    backing.deallocate_chunk(ptr, page_size);   
}

TEST(SystemPageBackingTest, AllocateChunkReturnsNullForZeroSize) {
    SystemPageBacking backing;
    void* ptr = backing.allocate_chunk(0);
    EXPECT_EQ(ptr, nullptr);
}

TEST(SystemPageBackingTest, DeallocateChunkHandlesNullPointer) {
    SystemPageBacking backing;
    backing.deallocate_chunk(nullptr, 1024); // Should not crash or throw
}

TEST(SystemPageBackingTest, DeallocateChunkHandlesZeroSize) {
    SystemPageBacking backing;
    void* ptr = backing.allocate_chunk(1024);
    backing.deallocate_chunk(ptr, 0); // Should not crash or throw
    backing.deallocate_chunk(ptr, 1024); // Clean up
}

void test_allocate_and_deallocate(SystemPageBacking& backing, size_t size, std::string_view test_case_msg = "") {
    void* ptr = backing.allocate_chunk(size);
    EXPECT_NE(ptr, nullptr) << "Allocation failed for size: " << size << " bytes. " << test_case_msg;
    backing.deallocate_chunk(ptr, size);
}

TEST(SystemPageBackingTest, AllocateAndDeallocateMultipleChunks) {
    SystemPageBacking backing;
    size_t page_size = get_page_size();

    test_allocate_and_deallocate(backing, page_size, "Testing allocation of 1 page.");
    test_allocate_and_deallocate(backing, page_size * 2, "Testing allocation of 2 pages.");
    test_allocate_and_deallocate(backing, page_size + 1, "Testing allocation of page_size + 1 bytes -- page lower boundary.");
    test_allocate_and_deallocate(backing, page_size + (page_size >> 2), "Testing allocation of page_size + (page_size >> 2) bytes -- in the middle.");
    test_allocate_and_deallocate(backing, page_size * 2 - 1, "Testing allocation of page_size * 2 - 1 bytes -- page upper boundary.");
    test_allocate_and_deallocate(backing, page_size * 10, "Testing allocation of 10 pages.");
}   

} // namespace kep_alloc::internal::testing