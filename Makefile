# PipeNc Makefile for MinGW Cross Compilation
# Supports both advanced version (with getopt) and simple version

# Compiler settings
MINGW_PREFIX = x86_64-w64-mingw32
CC = $(MINGW_PREFIX)-gcc
WINDRES = $(MINGW_PREFIX)-windres
STRIP = $(MINGW_PREFIX)-strip

# Compiler flags
CFLAGS = -Wall -Wextra -O2 -std=c99
LDFLAGS = -static-libgcc -static
LIBS = 

# Target architecture
ARCH = x86_64

# Output directory
BUILD_DIR = build
DIST_DIR = dist

# Source files
SOURCES_SIMPLE = pipenc_simple.c
SOURCES_ADVANCED = pipenc.c

# Output binaries
TARGET_ADVANCED = $(BUILD_DIR)/pipenc.exe
TARGETS = $(TARGET_ADVANCED)

# Default target
.PHONY: all clean debug release

all: $(TARGETS)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(DIST_DIR):
	mkdir -p $(DIST_DIR)


$(TARGET_ADVANCED): $(SOURCES_ADVANCED) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LIBS)
	$(STRIP) $@

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(DIST_DIR)

# Debug target
debug: CFLAGS += -DDEBUG -g
debug: all

# Release target (extra optimizations)
release: CFLAGS += -O3 -DNDEBUG
release: all

