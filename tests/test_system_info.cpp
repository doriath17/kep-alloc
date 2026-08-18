#include <gtest/gtest.h>
#include <internal/system_info.hpp>

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

}