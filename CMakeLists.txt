cmake_minimum_required(VERSION 3.20)
project(
  kep-alloc
  VERSION 1.0.0
  LANGUAGES CXX
)

# Prevent building inside the source directory
include(cmake/PreventInSourceBuilds.cmake)

# Enforce C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build Options
option(KEP_ALLOC_BUILD_EXAMPLES "Build library demo examples" ON)
option(KEP_ALLOC_BUILD_TESTS "Build unit tests" ON)

# ------------------------------------------------------------------------------
# Library Target
# ------------------------------------------------------------------------------

# Define implementation source files
# These files are compiled into the library target but they are NOT exposed to the library consumers
set(KEP_ALLOC_SOURCES
    src/bump_allocator/bump_allocator.cpp
)

# 
set(KEP_ALLOC_HEADERS
    src/bump_allocator/bump_allocator.hpp
#   include/kep_alloc/arena_allocator.hpp
#   include/kep_alloc/utils.hpp
)

# Create library target (controlled by standard CMake BUILD_SHARED_LIBS)
add_library(kep-alloc ${KEP_ALLOC_SOURCES} ${KEP_ALLOC_HEADERS})

# Alias for modern CMake target consumption (e.g., target_link_libraries(... kep-alloc::kep-alloc))
add_library(kep-alloc::kep-alloc ALIAS kep-alloc)

# Include Directories
target_include_directories(kep-alloc
  PUBLIC    # expose to consumers
    # Note: BUILD_INTERFACE is used to include this directory only when building the project from source code but ignore it after the library is installed. 
    # It is a generator expression
    # It allows to not break the library consumers after the library is installed (because otherwise CMake would look for the include directory in the source code after the library is installed)
    # TODO: study the difference between BUILD_INTERFACE and INSTALL_INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Set strict compiler warnings
if(MSVC)
  target_compile_options(kep-alloc PRIVATE /W4 /permissive-)
else()
  target_compile_options(kep-alloc PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ------------------------------------------------------------------------------
# Subdirectories
# ------------------------------------------------------------------------------

if(KEP_ALLOC_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

# if(KEP_ALLOC_BUILD_TESTS)
#   enable_testing()
#   add_subdirectory(tests)
# endif()