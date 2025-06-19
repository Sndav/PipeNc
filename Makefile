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
TARGET_SIMPLE = $(BUILD_DIR)/pipenc_simple.exe
TARGET_ADVANCED = $(BUILD_DIR)/pipenc.exe
TARGETS = $(TARGET_SIMPLE) $(TARGET_ADVANCED)

# Default target
.PHONY: all clean install simple advanced dist help test-simple test-advanced check-mingw debug release config

all: $(TARGETS)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(DIST_DIR):
	mkdir -p $(DIST_DIR)

# Build simple version (no external dependencies)
simple: $(TARGET_SIMPLE)

$(TARGET_SIMPLE): $(SOURCES_SIMPLE) | $(BUILD_DIR)
	@echo "Building simple version..."
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LIBS)
	$(STRIP) $@
	@echo "Simple version built: $@"

# Build advanced version (with getopt support)
advanced: $(TARGET_ADVANCED)

$(TARGET_ADVANCED): $(SOURCES_ADVANCED) | $(BUILD_DIR)
	@echo "Building advanced version..."
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LIBS)
	$(STRIP) $@
	@echo "Advanced version built: $@"

# Create distribution package
dist: all | $(DIST_DIR)
	@echo "Creating distribution package..."
	cp $(TARGET_SIMPLE) $(DIST_DIR)/
	cp $(TARGET_ADVANCED) $(DIST_DIR)/
	cp README.md $(DIST_DIR)/ 2>/dev/null || echo "README.md not found, skipping..."
	cp EXAMPLES.md $(DIST_DIR)/ 2>/dev/null || echo "EXAMPLES.md not found, skipping..."
	cp test_windows.bat $(DIST_DIR)/ 2>/dev/null || echo "test_windows.bat not found, skipping..."
	@echo "Distribution created in $(DIST_DIR)/"

# Install targets (copy to a system directory)
install: all
	@echo "Installing PipeNc..."
	@if [ -d "/usr/local/bin" ]; then \
		cp $(TARGET_SIMPLE) /usr/local/bin/pipenc_simple.exe; \
		cp $(TARGET_ADVANCED) /usr/local/bin/pipenc.exe; \
		echo "Installed to /usr/local/bin/"; \
	else \
		echo "Please manually copy the executables to your desired location"; \
	fi

# Test targets
test-simple: $(TARGET_SIMPLE)
	@echo "Testing simple version..."
	@echo "Run the following commands in separate terminals:"
	@echo "Terminal 1 (Server): $(TARGET_SIMPLE) -l \\\\.\\pipe\\test"
	@echo "Terminal 2 (Client): $(TARGET_SIMPLE) \\\\.\\pipe\\test"

test-advanced: $(TARGET_ADVANCED)
	@echo "Testing advanced version..."
	@echo "Run the following commands in separate terminals:"
	@echo "Terminal 1 (Server): $(TARGET_ADVANCED) -l -v \\\\.\\pipe\\test"
	@echo "Terminal 2 (Client): $(TARGET_ADVANCED) -v \\\\.\\pipe\\test"

# Cross-compilation check
check-mingw:
	@echo "Checking MinGW installation..."
	@which $(CC) > /dev/null || (echo "MinGW compiler not found: $(CC)" && exit 1)
	@$(CC) --version
	@echo "MinGW check passed!"

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILD_DIR)
	rm -rf $(DIST_DIR)
	@echo "Clean completed!"

# Debug target
debug: CFLAGS += -DDEBUG -g
debug: all

# Release target (extra optimizations)
release: CFLAGS += -O3 -DNDEBUG
release: all

# Print configuration
config:
	@echo "Build Configuration:"
	@echo "==================="
	@echo "MinGW Prefix: $(MINGW_PREFIX)"
	@echo "Compiler: $(CC)"
	@echo "CFLAGS: $(CFLAGS)"
	@echo "LDFLAGS: $(LDFLAGS)"
	@echo "Architecture: $(ARCH)"
	@echo "Build Dir: $(BUILD_DIR)"
	@echo "Dist Dir: $(DIST_DIR)"
	@echo ""

# Help target
help:
	@echo "PipeNc Makefile Help"
	@echo "===================="
	@echo ""
	@echo "Available targets:"
	@echo "  all          - Build both simple and advanced versions"
	@echo "  simple       - Build simple version (no external deps)"
	@echo "  advanced     - Build advanced version (with getopt)"
	@echo "  dist         - Create distribution package"
	@echo "  install      - Install to system directory"
	@echo "  test-simple  - Show test commands for simple version"
	@echo "  test-advanced- Show test commands for advanced version"
	@echo "  check-mingw  - Check MinGW installation"
	@echo "  clean        - Remove build artifacts"
	@echo "  help         - Show this help message"
	@echo ""
	@echo "Cross-compilation setup:"
	@echo "  Make sure MinGW-w64 is installed:"
	@echo "  - On macOS: brew install mingw-w64"
	@echo "  - On Ubuntu: apt-get install mingw-w64"
	@echo "  - On Fedora: dnf install mingw64-gcc"
	@echo ""
	@echo "Usage examples:"
	@echo "  make simple              # Build simple version"
	@echo "  make advanced            # Build advanced version"
	@echo "  make all                 # Build both versions"
	@echo "  make dist               # Create distribution"
	@echo ""
