# =============================================================================
# Build rules for Palm OS 68K applications with the modern gcc (m68k-gcc/).
#
# Include this file at the end of a project Makefile that sets:
#   APP_NAME    database name of the application
#   CREATOR_ID  4-character creator ID
#   SOURCES     .c and .cpp files
#   RESOURCES   .rcp file (pilrc)
# and optionally HEADERS, BUILD_DIR, CFLAGS_EXTRA, CXXFLAGS_EXTRA, LDFLAGS_EXTRA.
#
# Targets: build (default), clean, rebuild, install
# =============================================================================

M68K_GCC_DIR := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
PALMDEV_DIR  := $(abspath $(M68K_GCC_DIR)/..)
RUNTIME_DIR  := $(M68K_GCC_DIR)/runtime
GCC_BIN      := $(M68K_GCC_DIR)/toolchain/bin
SDK_DIR      := $(PALMDEV_DIR)/sdk/sdk-5r3/include

ifeq ($(wildcard $(GCC_BIN)/m68k-none-elf-gcc),)
  $(error Modern gcc not found! Please run: $(M68K_GCC_DIR)/build-toolchain.sh)
endif

BUILD_DIR ?= build
HEADERS   ?=

CC      = $(GCC_BIN)/m68k-none-elf-gcc
CXX     = $(GCC_BIN)/m68k-none-elf-g++
PYTHON ?= python3
POSTLINK  = $(PYTHON) $(RUNTIME_DIR)/palm-postlink.py
BUILD_PRC ?= $(PALMDEV_DIR)/toolchain/bin/build-prc
PILOT_XFER ?= pilot-xfer
# pilrc: look locally in the repo first, then Homebrew, then PATH
PILRC := $(shell \
    if [ -x "$(PALMDEV_DIR)/tools/pilrc" ]; then echo "$(PALMDEV_DIR)/tools/pilrc"; \
    elif [ -x "/opt/homebrew/opt/pilrc/bin/pilrc" ]; then echo "/opt/homebrew/opt/pilrc/bin/pilrc"; \
    else echo "pilrc"; fi)

SDK_INCLUDES = $(addprefix -isystem $(SDK_DIR),/ /Core /Core/Hardware /Core/System \
               /Core/UI /Dynamic /Libraries /Libraries/PalmOSGlue)

# Code generation as in Dmitry Grinberg's palmdev package (2020), except that
# -msep-data replaces -mpcrel -fpic: code and constants stay in the code
# resource, writable globals are reached through A5 (see runtime/startup.c).
ARCH_FLAGS = -m68000 -mshort -mno-align-int -fshort-enums -msep-data
OPT_FLAGS  = -Os -funsafe-math-optimizations -ffunction-sections -fdata-sections
WARN_FLAGS = -Wall -Wextra -Wno-multichar -Wno-unused-parameter

CFLAGS   = -std=gnu17 $(ARCH_FLAGS) $(OPT_FLAGS) $(WARN_FLAGS) $(SDK_INCLUDES) $(CFLAGS_EXTRA)
CXXFLAGS = -std=gnu++17 -fno-exceptions -fno-rtti -fno-threadsafe-statics \
           $(ARCH_FLAGS) $(OPT_FLAGS) $(WARN_FLAGS) $(SDK_INCLUDES) $(CXXFLAGS_EXTRA)
# The start code runs before A5 is set up: pc-relative only.
STARTUP_CFLAGS = -std=gnu17 -m68000 -mshort -mno-align-int -fshort-enums -mpcrel -fpic \
                 -Os -fno-tree-loop-distribute-patterns $(WARN_FLAGS) $(SDK_INCLUDES)
LDFLAGS  = -nostdlib -pie -Wl,--no-dynamic-linker -Wl,--gc-sections \
           -Wl,-T,$(RUNTIME_DIR)/palmos.ld $(LDFLAGS_EXTRA)
LIBS     = -lgcc

APP_OBJS     = $(patsubst %,$(BUILD_DIR)/%.o,$(basename $(SOURCES)))
RUNTIME_OBJS = $(BUILD_DIR)/runtime/startup.o $(BUILD_DIR)/runtime/crt.o \
               $(if $(filter %.cpp,$(SOURCES)),$(BUILD_DIR)/runtime/cxxabi.o)
ELF      = $(BUILD_DIR)/$(APP_NAME).elf
CODE_BIN = $(BUILD_DIR)/code0001.bin
RES_DIR  = $(BUILD_DIR)/res
PRC_NAME = $(BUILD_DIR)/$(APP_NAME).prc

.PHONY: build clean rebuild install
.DEFAULT_GOAL := build

build: $(PRC_NAME)
	@echo "Built $(PRC_NAME)"
	@ls -l $(PRC_NAME)

# build-prc collects the code resource and pilrc's resources (all *.bin)
$(PRC_NAME): $(CODE_BIN) $(RES_DIR)/.stamp
	$(BUILD_PRC) --no-check-resources -o $@ -t appl -c $(CREATOR_ID) -n "$(APP_NAME)" \
		$(CODE_BIN) $(wildcard $(RES_DIR)/*.bin)

$(CODE_BIN): $(ELF) $(RUNTIME_DIR)/palm-postlink.py
	$(POSTLINK) $< $@

# The runtime objects come first: __Startup__ must be at offset 0.
$(ELF): $(RUNTIME_OBJS) $(APP_OBJS) $(RUNTIME_DIR)/palmos.ld
	$(CXX) $(ARCH_FLAGS) $(LDFLAGS) -o $@ $(RUNTIME_OBJS) $(APP_OBJS) $(LIBS)

$(BUILD_DIR)/%.o: %.c $(HEADERS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.cpp $(HEADERS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/runtime/startup.o: $(RUNTIME_DIR)/startup.c $(RUNTIME_DIR)/palm-image.h
	@mkdir -p $(dir $@)
	$(CC) $(STARTUP_CFLAGS) -c $< -o $@

$(BUILD_DIR)/runtime/crt.o: $(RUNTIME_DIR)/crt.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/runtime/cxxabi.o: $(RUNTIME_DIR)/cxxabi.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(RES_DIR)/.stamp: $(RESOURCES) $(HEADERS)
	@rm -rf $(RES_DIR) && mkdir -p $(RES_DIR)
	$(PILRC) -q $(RESOURCES) $(RES_DIR)
	@touch $@

clean:
	rm -rf $(BUILD_DIR)

rebuild: clean build

install: $(PRC_NAME)
	$(PILOT_XFER) -i $(PRC_NAME)
