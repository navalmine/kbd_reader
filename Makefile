BUILD_DIR ?= build

.PHONY: configure build test clean

configure:
	@cmake -S . -B $(BUILD_DIR)

build: configure
	@cmake --build $(BUILD_DIR)

test: configure
	@cmake --build $(BUILD_DIR) --target test_scancode test_stats
	@ctest --test-dir $(BUILD_DIR) --output-on-failure
	@cmake --build $(BUILD_DIR)

clean:
	@cmake --build $(BUILD_DIR) --target clean || true
	@rm -rf $(BUILD_DIR)
