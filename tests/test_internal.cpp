#include "internal/allocate_mem.hpp"

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

} // namespace kep_alloc::internal::testing