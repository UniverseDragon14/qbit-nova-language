CC ?= cc

CFLAGS ?= \
	-std=c11 \
	-O2 \
	-Wall \
	-Wextra \
	-Werror \
	-pedantic \
	-Iinclude

BUILD_DIR := build
ERROR_TEST := $(BUILD_DIR)/test_error

.PHONY: all test clean

all: test

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(ERROR_TEST): src/error.c tests/test_error.c include/qnova/error.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) src/error.c tests/test_error.c -o $(ERROR_TEST)

test: $(ERROR_TEST)
	./$(ERROR_TEST)

clean:
	rm -rf $(BUILD_DIR)
