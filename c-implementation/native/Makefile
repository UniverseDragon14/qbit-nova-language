CC ?= gcc
CFLAGS ?= -O2 -g -std=c11 -Wall -Wextra -Wpedantic
CPPFLAGS ?= -Iinclude
LDLIBS ?= -lm

BUILD := build
BIN := $(BUILD)/qnova
SRC := src/qnova.c src/main.c
OBJ := $(SRC:src/%.c=$(BUILD)/%.o)

.PHONY: all clean test check-spec

all: $(BIN)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.o: src/%.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN): $(OBJ)
	$(CC) $(OBJ) $(LDLIBS) -o $@

test: $(BIN)
	QNOVA_BIN="$(abspath $(BIN))" bash tests/run_tests.sh

check-spec:
	cd ../specification/v0.1 && sha256sum -c SPEC_MANIFEST.sha256

clean:
	@echo "clean disabled: build artifacts are preserved; use a fresh build directory instead"
