#!/bin/bash

##############################################################################
# Palm OS Toolchain Setup Script
# Installs prc-tools, pilrc, and pilot-link via Homebrew (system-wide)
##############################################################################

set -e  # Exit on error

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_header() {
    echo -e "\n${BLUE}===================================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}===================================================${NC}\n"
}

print_success() {
    echo -e "${GREEN}✓${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}⚠${NC} $1"
}

print_error() {
    echo -e "${RED}✗${NC} $1"
}

print_info() {
    echo -e "${BLUE}ℹ${NC} $1"
}

# Get script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
TOOLCHAIN_DIR="$SCRIPT_DIR/toolchain"
PRC_TOOLS_DIR="$TOOLCHAIN_DIR"

# Pre-built binaries URL
PRC_TOOLS_RELEASE_URL="https://github.com/savaughn/prc-tools-remix/releases/download/macos-arm/prc-tools-remix-macos-arm64.tar.gz"

print_header "Palm OS Toolchain Setup"

# Detect architecture
ARCH=$(uname -m)
if [ "$ARCH" != "arm64" ]; then
    print_warning "This script downloads pre-built binaries for macOS ARM (Apple Silicon)"
    print_warning "Your architecture is: $ARCH"
    echo ""
fi

# Check requirements
print_header "Checking Requirements"

if ! command -v curl &> /dev/null; then
    print_error "curl not found (required for downloading)"
    exit 1
fi

if ! command -v tar &> /dev/null; then
    print_error "tar not found (required for extracting)"
    exit 1
fi

# Install or check prc-tools
print_header "Installing prc-tools"

if [ -d "$PRC_TOOLS_DIR" ] && [ -f "$PRC_TOOLS_DIR/bin/m68k-palmos-gcc" ]; then
    print_info "prc-tools already installed at $PRC_TOOLS_DIR"
else
    print_info "Downloading pre-built prc-tools for macOS ARM..."
    
    # Create toolchain directory
    mkdir -p "$TOOLCHAIN_DIR"
    
    # Download
    TEMP_ARCHIVE="/tmp/prc-tools-$$.tar.gz"
    if curl -L -o "$TEMP_ARCHIVE" "$PRC_TOOLS_RELEASE_URL"; then
        print_success "Downloaded prc-tools"
        
        # Extract
        print_info "Extracting to $TOOLCHAIN_DIR..."
        tar -xzf "$TEMP_ARCHIVE" -C "$TOOLCHAIN_DIR"
        
        # Move files from extracted subdirectory if it exists
        if [ -d "$TOOLCHAIN_DIR/prc-tools-remix-macos-arm64" ]; then
            print_info "Moving files from subdirectory..."
            # Move all contents up to toolchain dir
            mv "$TOOLCHAIN_DIR/prc-tools-remix-macos-arm64"/* "$TOOLCHAIN_DIR/"
            rm -rf "$TOOLCHAIN_DIR/prc-tools-remix-macos-arm64"
        fi
        
        # Install target directories to /usr/local (required by palmdev-prep)
        print_info "Installing target directories to /usr/local (requires sudo)..."
        if [ -d "$TOOLCHAIN_DIR/arm-palmos" ]; then
            sudo cp -R "$TOOLCHAIN_DIR/arm-palmos" /usr/local/
            print_success "Installed /usr/local/arm-palmos"
        else
            print_warning "arm-palmos directory not found in toolchain"
        fi
        if [ -d "$TOOLCHAIN_DIR/m68k-palmos" ]; then
            sudo cp -R "$TOOLCHAIN_DIR/m68k-palmos" /usr/local/
            print_success "Installed /usr/local/m68k-palmos"
        else
            print_warning "m68k-palmos directory not found in toolchain"
        fi
        
        # Create directory for prc-tools shared data
        sudo mkdir -p /usr/local/share/prc-tools
        
        # Create GCC lib directories for specs files
        sudo mkdir -p /usr/local/lib/gcc-lib/m68k-palmos/2.95.3-kgpd
        sudo mkdir -p /usr/local/lib/gcc-lib/arm-palmos/2.95.3-kgpd
        
        # Cleanup
        rm -f "$TEMP_ARCHIVE"
        
        if [ -f "$PRC_TOOLS_DIR/bin/m68k-palmos-gcc" ]; then
            print_success "prc-tools installed successfully"
        else
            print_error "Installation failed - compiler not found"
            exit 1
        fi
    else
        print_error "Failed to download prc-tools"
        print_info "You can manually download from: $PRC_TOOLS_RELEASE_URL"
        exit 1
    fi
fi

# Install pilrc and pilot-link via Homebrew
print_header "Installing Support Tools"

if ! command -v brew &> /dev/null; then
    print_error "Homebrew not found"
    exit 1
fi

print_success "Homebrew found: $(brew --version | head -n1)"

# Add tap
if ! brew tap | grep -q "jichu4n/palm-os"; then
    print_info "Adding tap jichu4n/palm-os..."
    brew tap jichu4n/palm-os
fi

# Install support tools
if brew list pilrc &> /dev/null; then
    print_info "pilrc already installed"
else
    brew install pilrc
    print_success "pilrc installed"
fi

if brew list pilot-link &> /dev/null; then
    print_info "pilot-link already installed"
else
    brew install pilot-link
    print_success "pilot-link installed"
fi

# Install Palm OS SDK
print_header "Installing Palm OS SDK"

SDK_INSTALL_DIR="/usr/local/palmdev"

if [ -d "$SDK_INSTALL_DIR/sdk-3.5" ]; then
    print_info "SDK already installed at $SDK_INSTALL_DIR"
else
    print_info "Downloading Palm OS SDK from GitHub..."
    
    # Clone SDK repo to temp location
    TEMP_SDK="/tmp/palm-os-sdk-$$"
    git clone --depth 1 https://github.com/jichu4n/palm-os-sdk.git "$TEMP_SDK"
    
    # Create target directory (requires sudo)
    print_info "Installing SDK to $SDK_INSTALL_DIR (requires sudo)..."
    sudo mkdir -p "$SDK_INSTALL_DIR"
    
    # Copy SDK files
    sudo cp -R "$TEMP_SDK"/* "$SDK_INSTALL_DIR/"
    
    # Cleanup
    rm -rf "$TEMP_SDK"
    
    print_success "SDK installed"
fi

# Run palmdev-prep if available
print_header "Configuring SDK with palmdev-prep"

# Add prc-tools to PATH temporarily for palmdev-prep
export PATH="$PRC_TOOLS_DIR/bin:$PATH"

if command -v palmdev-prep &> /dev/null; then
    print_info "Running palmdev-prep (requires sudo)..."
    
    # palmdev-prep needs to run as root to write to /usr/local/
    sudo -E env "PATH=$PATH" palmdev-prep
    
    print_success "SDK configured"
else
    print_warning "palmdev-prep not found in PATH, skipping SDK configuration"
    print_info "Make sure $PRC_TOOLS_DIR/bin is in your PATH"
fi

# Verify installations
print_header "Verifying Installation"

# Add toolchain to PATH temporarily for verification
export PATH="$PRC_TOOLS_DIR/bin:$PATH"

check_tool() {
    local TOOL=$1
    if command -v "$TOOL" &> /dev/null; then
        print_success "$TOOL found"
        return 0
    else
        print_error "$TOOL not found"
        return 1
    fi
}

ALL_OK=true
check_tool "m68k-palmos-gcc" || ALL_OK=false
check_tool "pilrc" || ALL_OK=false
check_tool "build-prc" || ALL_OK=false
check_tool "pilot-xfer" || ALL_OK=false

if [ "$ALL_OK" = true ]; then
    print_success "All tools installed!"
else
    print_warning "Some tools missing"
fi

# Final instructions
print_header "Setup Complete!"

echo ""
print_success "Palm OS development environment is ready!"
echo ""
echo "To build the HelloWorld sample application:"
echo "  cd helloworld"
echo "  make build"
echo ""
echo "Your application will be in: helloworld/build/HelloWorld.prc"
echo ""
