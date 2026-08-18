include(FetchContent)

# Download GoogleTest automatically
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0 # Stable gtest release tag
)

# For Windows build compatibility
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

# ====================================================
# Prepare executable for unit tests
# ====================================================

add_executable(kep_alloc_tests
  test_system_info.cpp
)

# Link against GoogleTest and kep_alloc
target_link_libraries(kep_alloc_tests
  PRIVATE
    kep-alloc::kep-alloc
    gtest_main
)

# Allow unit tests to see private headers under src/
target_include_directories(kep_alloc_tests
  PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# 4. Discover tests for CTest integration
include(GoogleTest)
gtest_discover_tests(kep_alloc_tests)