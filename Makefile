.PHONY: config-debug config-release build build-lib build-demo build-tests run-tests run-demo format format-check lint clean

# Configuration
config-debug:
	cmake -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

config-release:
	cmake -B build -DCMAKE_BUILD_TYPE=Release

# Compilation
build:
	cmake --build build -j

build-lib:
	cmake --build build --target kep-alloc -j

build-demo:
	cmake --build build --target demo -j

build-tests:
	cmake --build build --target tests -j

# Quality & Tooling
format-check:
	cmake --build build --target format-check

format:
	cmake --build build --target format

lint:
	cmake --build build --target lint

# Execution
run-tests: build-tests
	ctest --test-dir build --output-on-failure

run-demo: build-demo
	./build/examples/demo

docs:
	doxygen Doxyfile

# Cleanup
clean:
	rm -rf build