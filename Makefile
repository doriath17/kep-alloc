.PHONY: config-debug config-release build build-lib build-demo build-tests run-tests run-test-suite run-test-unit run-demo format format-check lint clean

# Configuration
config-debug:
	cmake -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

config-release:
	cmake -B build -DCMAKE_BUILD_TYPE=Release

# Compilation
build:
	cmake --build build -j

build-debug: config-debug
	cmake --build build -j

build-lib:
	cmake --build build --target kep-alloc -j

build-demo:
	cmake --build build --target demo -j

build-tests: config-debug
	cmake --build build --target kep_alloc_tests -j

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

# example: make run-test-suite SUITE=TestSuiteName
run-test-suite: build-tests
	@if [ -z "$(SUITE)" ]; then echo "Usage: make run-test-suite SUITE=TestSuiteName"; exit 1; fi
	ctest --test-dir build --output-on-failure -R '^$(SUITE)\.'

# example: make run-test-unit TEST=TestSuiteName.TestName
run-test-unit: build-tests
	@if [ -z "$(TEST)" ]; then echo "Usage: make run-test-unit TEST=TestSuiteName.TestName"; exit 1; fi
	ctest --test-dir build --output-on-failure -R '^$(TEST)$$'

run-demo: build-demo
	./build/examples/demo

docs:
	doxygen Doxyfile

# Cleanup
clean:
	rm -rf build