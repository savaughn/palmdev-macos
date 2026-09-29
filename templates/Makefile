# =============================================================================
# Palm OS Dual-Target Makefile Template
#
# Supports:
#   make build                  -> Palm OS 4 (pure 68K app)
#   make build TARGET=palmos5   -> Palm OS 5 (ARM-native with 68K stub)
#
# Instructions:
#   1. Adjust APP_NAME, CREATOR_ID, SOURCES
#   2. Create .c files and .rcp file
#   3. Run "make build"
# =============================================================================

# === Project configuration (to be filled in by the user) ===
APP_NAME    = MyApp
CREATOR_ID  = MyAp
RESOURCES   = app.rcp

# Source files:
# - palmos4: SOURCES is compiled directly (68K)
# - palmos5: ARM_SOURCES is compiled for ARM, SOURCES is ignored
SOURCES      = main.c
ARM_SOURCES  = main_arm.c
HEADERS      =

# === Target: palmos4 (default) or palmos5 ===
TARGET ?= palmos4

# === Common paths ===
# Auto-detect: project lives inside the cloned repo (e.g. palmdev-macos/myapp/)
REPO_DIR      := $(shell cd .. && pwd)
TOOLCHAIN_DIR = $(REPO_DIR)/toolchain
PEAL_DIR      = $(REPO_DIR)/peal
SDK4_DIR      = $(REPO_DIR)/sdk/sdk-4
SDK5_DIR      = $(REPO_DIR)/sdk/sdk-5r3
BUILD_DIR     = build/$(TARGET)

# === Common tools ===
# pilrc: look locally in the repo first, then Homebrew, then PATH
PILRC     := $(shell \
    if [ -x "$(REPO_DIR)/tools/pilrc" ]; then echo "$(REPO_DIR)/tools/pilrc"; \
    elif [ -x "/opt/homebrew/opt/pilrc/bin/pilrc" ]; then echo "/opt/homebrew/opt/pilrc/bin/pilrc"; \
    else echo "pilrc"; fi)
BUILD_PRC = $(TOOLCHAIN_DIR)/bin/build-prc
export GCC_EXEC_PREFIX = $(TOOLCHAIN_DIR)/lib/gcc-lib/

# =============================================================================
# Palm OS 4 (pure 68K app)
# =============================================================================
ifeq ($(TARGET),palmos4)

SDK_DIR = $(SDK4_DIR)

CC = $(TOOLCHAIN_DIR)/bin/m68k-palmos-gcc
CFLAGS = -B$(TOOLCHAIN_DIR)/lib/gcc-lib/m68k-palmos/2.95.3-kgpd/ \
         -B$(TOOLCHAIN_DIR)/m68k-palmos/bin/ \
         -B$(TOOLCHAIN_DIR)/m68k-palmos/lib/ \
         -nostdinc \
         -I$(SDK_DIR)/include \
         -I$(SDK_DIR)/include/Core \
         -I$(SDK_DIR)/include/Core/Hardware \
         -I$(SDK_DIR)/include/Core/System \
         -I$(SDK_DIR)/include/Core/UI \
         -I$(SDK_DIR)/include/Dynamic \
         -I$(SDK_DIR)/include/Libraries \
         -O2 -Wall
LDFLAGS = -L$(SDK_DIR)/lib

OBJS = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SOURCES))
APP_BINARY = $(BUILD_DIR)/$(APP_NAME)
PRC_NAME = $(BUILD_DIR)/$(APP_NAME).prc

.PHONY: all build clean rebuild info

all: info

info:
	@echo "=== $(APP_NAME) - Palm OS Application ==="
	@echo ""
	@echo "  make build                 - Build for Palm OS 4 (68K)"
	@echo "  make build TARGET=palmos5  - Build for Palm OS 5 (ARM)"
	@echo "  make clean                 - Delete build artifacts"
	@echo "  make rebuild               - Clean + Build"
	@echo ""
	@echo "  Current target: $(TARGET)"

build: $(PRC_NAME)
	@echo "=== Built $(PRC_NAME) ==="
	@ls -lh $(PRC_NAME)

$(PRC_NAME): $(APP_BINARY) $(BUILD_DIR)/.resources
	$(BUILD_PRC) -o $@ \
		-t appl \
		-c $(CREATOR_ID) \
		-n "$(APP_NAME)" \
		$(APP_BINARY) $(BUILD_DIR)/*.bin

$(APP_BINARY): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $@

$(BUILD_DIR)/%.o: %.c $(HEADERS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/.resources: $(RESOURCES) $(HEADERS)
	@mkdir -p $(BUILD_DIR)
	$(PILRC) $(RESOURCES) $(BUILD_DIR)
	@touch $@

clean:
	/bin/rm -rf build
	/bin/rm -f *.grc *~

rebuild: clean build

endif # palmos4

# =============================================================================
# Palm OS 5 (ARM-native with 68K stub via PEAL)
#
# Build-Pipeline:
#   1. ARM compile:      arm-palmos-gcc -> .o files
#   2. ARM link:         arm-palmos-gcc -> ARM ELF
#   3. Postlink:         peal-postlink -> .ro (armc resources)
#   4. 68K compile:      m68k-palmos-gcc -> stub + PEAL loader
#   5. 68K link:         m68k-palmos-gcc -> 68K ELF
#   6. PilRC:            pilrc -> .bin resources
#   7. build-prc:        68K ELF + .bin + .ro -> .prc
# =============================================================================
ifeq ($(TARGET),palmos5)

SDK_DIR = $(SDK5_DIR)

# 68K compiler (for stub + PEAL loader)
CC_M68K = $(TOOLCHAIN_DIR)/bin/m68k-palmos-gcc
CFLAGS_M68K = -B$(TOOLCHAIN_DIR)/lib/gcc-lib/m68k-palmos/2.95.3-kgpd/ \
              -B$(TOOLCHAIN_DIR)/m68k-palmos/bin/ \
              -B$(TOOLCHAIN_DIR)/m68k-palmos/lib/ \
              -nostdinc \
              -I$(SDK_DIR)/include \
              -I$(SDK_DIR)/include/Core \
              -I$(SDK_DIR)/include/Core/Hardware \
              -I$(SDK_DIR)/include/Core/System \
              -I$(SDK_DIR)/include/Core/UI \
              -I$(SDK_DIR)/include/Dynamic \
              -I$(SDK_DIR)/include/Libraries \
              -I$(SDK_DIR)/include/Libraries/PalmOSGlue \
              -I$(PEAL_DIR)/m68k \
              -O2 -Wall
LDFLAGS_M68K = -L$(SDK_DIR)/lib -L$(SDK_DIR)/lib/m68k-palmos-coff
LDLIBS_M68K  = -lPalmOSGlue

# ARM compiler (for main application)
CC_ARM = $(TOOLCHAIN_DIR)/bin/arm-palmos-gcc
CFLAGS_ARM = -B$(TOOLCHAIN_DIR)/lib/gcc-lib/arm-palmos/3.3.1/ \
             -B$(TOOLCHAIN_DIR)/arm-palmos/bin/ \
             -B$(TOOLCHAIN_DIR)/arm-palmos/lib/ \
             -nostdinc \
             -I$(SDK_DIR)/include \
             -I$(SDK_DIR)/include/Core \
             -I$(SDK_DIR)/include/Core/Hardware \
             -I$(SDK_DIR)/include/Core/System \
             -I$(SDK_DIR)/include/Core/UI \
             -I$(SDK_DIR)/include/Dynamic \
             -I$(SDK_DIR)/include/Libraries \
             -I$(PEAL_DIR)/arm \
             -march=armv4 -mtune=xscale \
             -msingle-pic-base \
             -O2 -Wall
LDFLAGS_ARM = -nostartfiles \
              -Wl,--emit-relocs \
              -Wl,--split-by-file=63000

PEAL_POSTLINK = $(TOOLCHAIN_DIR)/bin/peal-postlink

# ARM objects (pealstub.o must come first!)
ARM_OBJS = $(BUILD_DIR)/arm/pealstub.o \
           $(patsubst %.c,$(BUILD_DIR)/arm/%.o,$(ARM_SOURCES))
ARM_ELF = $(BUILD_DIR)/arm/$(APP_NAME)_arm.elf
ARM_RO  = $(BUILD_DIR)/arm/$(APP_NAME)_arm.ro

# 68K objects (stub + PEAL loader)
M68K_OBJS = $(BUILD_DIR)/m68k/stub_m68k.o \
            $(BUILD_DIR)/m68k/peal.o
M68K_ELF = $(BUILD_DIR)/m68k/$(APP_NAME)_stub

PRC_NAME = $(BUILD_DIR)/$(APP_NAME).prc

.PHONY: all build clean rebuild info

all: info

info:
	@echo "=== $(APP_NAME) - Palm OS Application ==="
	@echo ""
	@echo "  make build                 - Build for Palm OS 4 (68K)"
	@echo "  make build TARGET=palmos5  - Build for Palm OS 5 (ARM)"
	@echo "  make clean                 - Delete build artifacts"
	@echo "  make rebuild               - Clean + Build"
	@echo ""
	@echo "  Current target: $(TARGET)"

build: $(PRC_NAME)
	@echo "=== Built $(PRC_NAME) (ARM-native with 68K stub) ==="
	@ls -lh $(PRC_NAME)

# Step 7: Package everything
$(PRC_NAME): $(M68K_ELF) $(ARM_RO) $(BUILD_DIR)/.resources
	$(BUILD_PRC) -o $@ \
		-t appl \
		-c $(CREATOR_ID) \
		-n "$(APP_NAME)" \
		$(M68K_ELF) $(BUILD_DIR)/*.bin $(ARM_RO)

# Step 6: Compile resources
$(BUILD_DIR)/.resources: $(RESOURCES) $(HEADERS)
	@mkdir -p $(BUILD_DIR)
	$(PILRC) $(RESOURCES) $(BUILD_DIR)
	@touch $@

# Step 5: Link 68K
$(M68K_ELF): $(M68K_OBJS)
	$(CC_M68K) $(CFLAGS_M68K) $(LDFLAGS_M68K) $(M68K_OBJS) $(LDLIBS_M68K) -o $@

# Step 4: Compile 68K (stub)
$(BUILD_DIR)/m68k/stub_m68k.o: stub_m68k.c $(HEADERS)
	@mkdir -p $(BUILD_DIR)/m68k
	$(CC_M68K) $(CFLAGS_M68K) -c $< -o $@

# Step 4: Compile 68K (PEAL loader)
$(BUILD_DIR)/m68k/peal.o: $(PEAL_DIR)/m68k/peal.c
	@mkdir -p $(BUILD_DIR)/m68k
	$(CC_M68K) $(CFLAGS_M68K) -c $< -o $@

# Step 3: ARM ELF -> Palm OS resource
$(ARM_RO): $(ARM_ELF)
	$(PEAL_POSTLINK) -s 1000 -o $@ $(ARM_ELF)

# Step 2: Link ARM (pealstub.o must come first!)
$(ARM_ELF): $(ARM_OBJS)
	$(CC_ARM) $(CFLAGS_ARM) $(LDFLAGS_ARM) $(ARM_OBJS) -o $@

# Step 1: Compile ARM (pealstub)
$(BUILD_DIR)/arm/pealstub.o: $(PEAL_DIR)/arm/pealstub.c
	@mkdir -p $(BUILD_DIR)/arm
	$(CC_ARM) $(CFLAGS_ARM) -c $< -o $@

# Step 1: Compile ARM (app sources)
$(BUILD_DIR)/arm/%.o: %.c $(HEADERS)
	@mkdir -p $(BUILD_DIR)/arm
	$(CC_ARM) $(CFLAGS_ARM) -I. -c $< -o $@

clean:
	/bin/rm -rf build
	/bin/rm -f *.grc *~

rebuild: clean build

endif # palmos5
