# Add demo executables and link them to the library target
add_executable(demo demo.cpp)
target_link_libraries(demo PRIVATE kep-alloc::kep-alloc)