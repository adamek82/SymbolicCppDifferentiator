CXX ?= g++
APP := diff

SRC := \
	src/main.cpp \
	src/lexer.cpp \
	src/parser.cpp \
	src/diff.cpp \
	src/simplify.cpp \
	src/print.cpp

HDR := \
	include/ast.hpp \
	include/lexer.hpp \
	include/parser.hpp \
	include/diff.hpp \
	include/simplify.hpp \
	include/print.hpp

FORMAT_FILES := $(SRC) $(HDR)

CLANG_FORMAT ?= clang-format

INCLUDES := -Iinclude

COMMON_FLAGS := -std=c++20 -Wall -Wextra -pedantic $(INCLUDES)

DEBUG_FLAGS := -g -O0
RELEASE_FLAGS := -O2 -DNDEBUG

ifeq ($(OS),Windows_NT)
	EXE := .exe
else
	EXE :=
endif

DEBUG_BIN := build/debug/$(APP)$(EXE)
RELEASE_BIN := build/release/$(APP)$(EXE)

.PHONY: all debug release run clean

all: debug

debug: $(DEBUG_BIN)

release: $(RELEASE_BIN)

$(DEBUG_BIN): $(SRC)
	mkdir -p build/debug
	$(CXX) $(COMMON_FLAGS) $(DEBUG_FLAGS) $(SRC) -o $(DEBUG_BIN)

$(RELEASE_BIN): $(SRC)
	mkdir -p build/release
	$(CXX) $(COMMON_FLAGS) $(RELEASE_FLAGS) $(SRC) -o $(RELEASE_BIN)

run: debug
	./$(DEBUG_BIN)

clean:
	rm -rf build

format:
	$(CLANG_FORMAT) -i $(FORMAT_FILES)

format-check:
	$(CLANG_FORMAT) --dry-run --Werror $(FORMAT_FILES)
