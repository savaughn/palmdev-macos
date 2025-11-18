#!/bin/bash

##############################################################################
# Palm OS Toolchain Uninstall Script
# Removes all Palm OS development tools and SDKs
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

print_header "Palm OS Toolchain Uninstall"

echo "This will remove:"
echo "  • Local toolchain directory: $TOOLCHAIN_DIR"
echo "  • Palm OS SDK: /usr/local/palmdev"
echo "  • Target directories: /usr/local/{arm,m68k}-palmos"
echo "  • GCC specs files: /usr/local/lib/gcc-lib/{m68k,arm}-palmos/"
echo "  • Trap numbers: /usr/local/share/prc-tools/"
echo "  • Homebrew packages: pilrc, pilot-link"
echo "  • Homebrew tap: jichu4n/palm-os"
echo ""
print_warning "This action cannot be undone!"
echo ""
read -p "Are you sure you want to continue? (yes/no): " -r
echo ""

if [[ ! $REPLY =~ ^[Yy][Ee][Ss]$ ]]; then
    print_info "Uninstall cancelled"
    exit 0
fi

# Remove local toolchain directory
print_header "Removing Local Toolchain"

if [ -d "$TOOLCHAIN_DIR" ]; then
    print_info "Removing $TOOLCHAIN_DIR..."
    rm -rf "$TOOLCHAIN_DIR"
    print_success "Local toolchain removed"
else
    print_info "Local toolchain directory not found"
fi

# Remove system-wide installations (requires sudo)
print_header "Removing System-Wide Installations"

NEED_SUDO=false

# Check what needs to be removed
if [ -d "/usr/local/palmdev" ]; then
    NEED_SUDO=true
fi

if [ -d "/usr/local/arm-palmos" ] || [ -d "/usr/local/m68k-palmos" ]; then
    NEED_SUDO=true
fi

if [ -d "/usr/local/lib/gcc-lib/m68k-palmos" ] || [ -d "/usr/local/lib/gcc-lib/arm-palmos" ]; then
    NEED_SUDO=true
fi

if [ -d "/usr/local/share/prc-tools" ]; then
    NEED_SUDO=true
fi

if [ "$NEED_SUDO" = true ]; then
    print_info "Removing system directories (requires sudo)..."
    
    if [ -d "/usr/local/palmdev" ]; then
        sudo rm -rf /usr/local/palmdev
        print_success "Removed /usr/local/palmdev (SDK)"
    fi
    
    if [ -d "/usr/local/arm-palmos" ]; then
        sudo rm -rf /usr/local/arm-palmos
        print_success "Removed /usr/local/arm-palmos (target directory)"
    fi
    
    if [ -d "/usr/local/m68k-palmos" ]; then
        sudo rm -rf /usr/local/m68k-palmos
        print_success "Removed /usr/local/m68k-palmos (target directory)"
    fi
    
    if [ -d "/usr/local/lib/gcc-lib/m68k-palmos" ]; then
        sudo rm -rf /usr/local/lib/gcc-lib/m68k-palmos
        print_success "Removed /usr/local/lib/gcc-lib/m68k-palmos (GCC specs)"
    fi
    
    if [ -d "/usr/local/lib/gcc-lib/arm-palmos" ]; then
        sudo rm -rf /usr/local/lib/gcc-lib/arm-palmos
        print_success "Removed /usr/local/lib/gcc-lib/arm-palmos (GCC specs)"
    fi
    
    if [ -d "/usr/local/share/prc-tools" ]; then
        sudo rm -rf /usr/local/share/prc-tools
        print_success "Removed /usr/local/share/prc-tools (trap numbers)"
    fi
else
    print_info "No system-wide installations found"
fi

# Remove Homebrew packages
print_header "Removing Homebrew Packages"

if command -v brew &> /dev/null; then
    if brew list pilrc &> /dev/null; then
        print_info "Uninstalling pilrc..."
        brew uninstall pilrc
        print_success "Removed pilrc"
    else
        print_info "pilrc not installed"
    fi
    
    if brew list pilot-link &> /dev/null; then
        print_info "Uninstalling pilot-link..."
        brew uninstall pilot-link
        print_success "Removed pilot-link"
    else
        print_info "pilot-link not installed"
    fi
    
    # Remove tap if no other packages from it are installed
    if brew tap | grep -q "jichu4n/palm-os"; then
        print_info "Removing tap jichu4n/palm-os..."
        brew untap jichu4n/palm-os
        print_success "Removed tap"
    fi
else
    print_info "Homebrew not found, skipping package removal"
fi

# Final message
print_header "Uninstall Complete!"

print_success "All Palm OS toolchain files have been removed"
echo ""
print_info "To reinstall, run: ./setup-toolchain.sh"
