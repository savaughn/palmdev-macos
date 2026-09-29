# Makefile for HelloWorld Palm OS Application

APP_NAME = HelloWorld
BUILD_DIR = build
PRC_NAME = $(BUILD_DIR)/$(APP_NAME).prc
CREATOR_ID = HlWd

# Source files
SOURCES = $(APP_NAME).c
RESOURCES = $(APP_NAME).rcp

# Get script directory for toolchain
TOOLCHAIN_DIR := $(shell cd .. && pwd)/toolchain

# Check if toolchain is installed
ifeq ($(wildcard $(TOOLCHAIN_DIR)/bin/m68k-palmos-gcc),)
  $(error Toolchain not found! Please run: cd .. && ./setup-toolchain.sh)
endif

# Set GCC environment (required for prc-tools to find libraries)
export GCC_EXEC_PREFIX = $(TOOLCHAIN_DIR)/lib/gcc-lib/

# SDK path (local, no palmdev-prep required)
SDK_DIR := $(shell cd .. && pwd)/sdk/sdk-4

# Compiler and tools (-B tells GCC where to find as/ld/cc1 locally, no sudo needed)
CC = $(TOOLCHAIN_DIR)/bin/m68k-palmos-gcc
CFLAGS = -B$(TOOLCHAIN_DIR)/lib/gcc-lib/m68k-palmos/2.95.3-kgpd/ -B$(TOOLCHAIN_DIR)/m68k-palmos/bin/ -B$(TOOLCHAIN_DIR)/m68k-palmos/lib/ -nostdinc -I$(SDK_DIR)/include -I$(SDK_DIR)/include/Core -I$(SDK_DIR)/include/Core/Hardware -I$(SDK_DIR)/include/Core/System -I$(SDK_DIR)/include/Core/UI -I$(SDK_DIR)/include/Dynamic -I$(SDK_DIR)/include/Libraries -O2 -Wall
LDFLAGS = -L$(SDK_DIR)/lib
# pilrc: look locally in the repo first, then Homebrew, then PATH
REPO_DIR_HW := $(shell cd .. && pwd)
PILRC := $(shell \
    if [ -x "$(REPO_DIR_HW)/tools/pilrc" ]; then echo "$(REPO_DIR_HW)/tools/pilrc"; \
    elif [ -x "/opt/homebrew/opt/pilrc/bin/pilrc" ]; then echo "/opt/homebrew/opt/pilrc/bin/pilrc"; \
    else echo "pilrc"; fi)
BUILD_PRC = $(TOOLCHAIN_DIR)/bin/build-prc
PILOT_XFER = $(HOME)/.local/bin/pilot-xfer

# Build targets
OBJS = $(BUILD_DIR)/$(APP_NAME).o
APP_BINARY = $(BUILD_DIR)/$(APP_NAME)
RESOURCE_FILE = $(APP_NAME).ro

.PHONY: all clean install info

all: info

info:
	@echo "╔════════════════════════════════════════════════════╗"
	@echo "║        HelloWorld - Palm OS Application            ║"
	@echo "╚════════════════════════════════════════════════════╝"
	@echo ""
	@echo " A simple 'Hello World' demo for Palm OS"
	@echo ""
	@echo " Available targets:"
	@echo "  make build     - Build $(PRC_NAME)"
	@echo "  make clean     - Remove build artifacts"
	@echo "  make install   - Install to Palm device via USB"
	@echo "  make rebuild   - Clean and build"
	@echo ""
	@echo " Status:"
	@if [ -f "$(TOOLCHAIN_DIR)/bin/m68k-palmos-gcc" ]; then \
		echo "  ✓ Toolchain ready"; \
	else \
		echo "  ✗ Toolchain not found - run: cd .. && ./setup-toolchain.sh"; \
	fi

build: $(PRC_NAME)
	@echo "✓ Built $(PRC_NAME)"
	@ls -lh $(PRC_NAME)

$(PRC_NAME): $(APP_BINARY) $(RESOURCE_FILE)
	@mkdir -p $(BUILD_DIR)
	$(BUILD_PRC) -o $@ \
		-t appl \
		-c $(CREATOR_ID) \
		-n "$(APP_NAME)" \
		$(APP_BINARY) $(BUILD_DIR)/*.bin

$(APP_BINARY): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(OBJS) -o $@

$(BUILD_DIR)/%.o: %.c $(HEADERS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(RESOURCE_FILE): $(RESOURCES) $(HEADERS)
	@mkdir -p $(BUILD_DIR)
	$(PILRC) $(RESOURCES) $(BUILD_DIR)

clean:
	/bin/rm -rf $(BUILD_DIR)
	/bin/rm -f *.grc *~

install: $(PRC_NAME)
	@echo "Installing $(PRC_NAME) to Palm device..."
	$(PILOT_XFER) -i $(PRC_NAME)

rebuild: clean build
