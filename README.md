# Requirements

- Install CMake (version 3.20 and up)
- Install GCC (version 10 and up)

# Build Instructions

## Building commands

- Configure the build system:

  `cmake -B build -DCMAKE_BUILD_TYPE=Debug`

  Note: To build in release mode use `-DCMAKE_BUILD_TYPE=Release`

- To build all targets in parallel:

  `cmake --build build -j`

- To build the library target only:

  `cmake --build build --target kep-alloc`

- To build the demo target only:

  `cmake --build build --target demo`

- To build the unit tests target only:

  `cmake --build build --target tests`

## Running commands

- To run unit tests using CTest:

  `ctest --test-dir build --output-on-failure`

  or run the tests executable directly:

  `./build/tests/tests`

- To run the demo executable:

  `./build/examples/demo` (use the specific demo you need to run)
