# Building

config-debug: 
	cmake -B build -DCMAKE_BUILD_TYPE=Debug

config-release: 
	cmake -B build -DCMAKE_BUILD_TYPE=Release

build: 
	cmake --build build -j

build-lib: 
	cmake --build build --target kep-alloc

build-demo: 
	cmake --build build --target demo

build-tests: 
	cmake --build build --target tests

run-tests: build-tests
	ctest --test-dir build --output-on-failure

run-demo: build-demo
	./build/examples/demo


