#!/bin/bash

##############################################################################
# Creates a self-contained ZIP file with the complete Palm OS toolchain.
# The ZIP contains everything needed to build Palm OS apps right away.
##############################################################################

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
VERSION="1"
ZIP_NAME="palmdev-macos-arm-v${VERSION}"
STAGING_DIR="/tmp/${ZIP_NAME}"

echo "=== Building ${ZIP_NAME}.zip ==="

# Clean up old staging
rm -rf "$STAGING_DIR"
mkdir -p "$STAGING_DIR"

echo "Copying toolchain..."
cp -R "$SCRIPT_DIR/toolchain" "$STAGING_DIR/toolchain"

echo "Copying SDK..."
cp -R "$SCRIPT_DIR/sdk" "$STAGING_DIR/sdk"

echo "Copying PEAL..."
cp -R "$SCRIPT_DIR/peal" "$STAGING_DIR/peal"
# Do not ship debug symbols
rm -rf "$STAGING_DIR/peal/postlink/peal-postlink.dSYM"

echo "Copying templates..."
cp -R "$SCRIPT_DIR/templates" "$STAGING_DIR/templates"

echo "Copying HelloWorld example..."
cp -R "$SCRIPT_DIR/helloworld" "$STAGING_DIR/helloworld"
# Remove build artifacts
rm -rf "$STAGING_DIR/helloworld/build"

echo "Bundling pilrc..."
mkdir -p "$STAGING_DIR/tools"
PILRC_BIN=""
if [ -x "/opt/homebrew/opt/pilrc/bin/pilrc" ]; then
    PILRC_BIN="/opt/homebrew/opt/pilrc/bin/pilrc"
elif [ -x "/opt/homebrew/Cellar/pilrc/3.2/bin/pilrc" ]; then
    PILRC_BIN="/opt/homebrew/Cellar/pilrc/3.2/bin/pilrc"
elif command -v pilrc &> /dev/null; then
    PILRC_BIN="$(command -v pilrc)"
fi

if [ -n "$PILRC_BIN" ]; then
    cp "$PILRC_BIN" "$STAGING_DIR/tools/pilrc"
    chmod +x "$STAGING_DIR/tools/pilrc"
    echo "  pilrc bundled from: $PILRC_BIN"
else
    echo "  WARNING: pilrc not found - ZIP will require Homebrew installation"
fi

echo "Copying docs..."
cp "$SCRIPT_DIR/README.md" "$STAGING_DIR/"
cp "$SCRIPT_DIR/LICENSE" "$STAGING_DIR/" 2>/dev/null || true

echo "Creating quickstart README..."
cat > "$STAGING_DIR/QUICKSTART.md" << 'QUICKSTART_EOF'
# Palm OS Toolchain for macOS (Apple Silicon) - Quick Start

## Build the Hello World Example

    cd helloworld
    make build

Output: `build/HelloWorld.prc`

## Create Your Own Application

    mkdir myapp
    cd myapp

    # Copy template files
    cp ../templates/Makefile .
    cp ../templates/app.rcp .

    # For Palm OS 4 (68K):
    cp ../templates/palmos4/main.c .
    make build

    # For Palm OS 5 (ARM native):
    cp ../templates/palmos5/main_arm.c .
    cp ../templates/palmos5/stub_m68k.c .
    cp ../templates/palmos5/pace.h .
    make build TARGET=palmos5

## Configure Your App

Edit the top of the Makefile:

    APP_NAME    = MyApp        # Application name
    CREATOR_ID  = MyAp         # Unique 4-character ID
    RESOURCES   = app.rcp      # Resource file
    SOURCES     = main.c       # Source files (Palm OS 4)
    ARM_SOURCES = main_arm.c   # Source files (Palm OS 5)

## What's Inside

    toolchain/   - Cross-compilers (m68k-palmos-gcc, arm-palmos-gcc, build-prc)
    sdk/         - Palm OS SDK 4.0 and 5r3 (headers and libraries)
    peal/        - Palm Executive ARM Loader (for Palm OS 5 ARM apps)
    tools/       - pilrc (resource compiler)
    templates/   - Makefile and source templates for Palm OS 4 and 5
    helloworld/  - Working example application

## Requirements

- macOS 11+ (Big Sur or later) on Apple Silicon
- No installation required - everything runs from this directory

## First-Time macOS Security Note

macOS may block the toolchain binaries on first run ("cannot be opened because
the developer cannot be verified"). To allow them:

    System Settings > Privacy & Security > scroll down > "Allow Anyway"

Or remove the quarantine flag from all binaries at once:

    xattr -r -d com.apple.quarantine toolchain/ tools/ peal/

## Installing on a Palm Device

Via USB (requires pilot-link: `brew tap jichu4n/palm-os && brew install pilot-link`):

    pilot-xfer -i build/MyApp.prc

Or drag the .prc file onto POSE / CloudpilotEmu.
QUICKSTART_EOF

# Create ZIP
echo "Creating ZIP..."
cd /tmp
rm -f "${ZIP_NAME}.zip"
zip -r -q "${ZIP_NAME}.zip" "${ZIP_NAME}"

# Result
ZIP_SIZE=$(du -h "/tmp/${ZIP_NAME}.zip" | cut -f1)
echo ""
echo "=== Done ==="
echo "ZIP: /tmp/${ZIP_NAME}.zip (${ZIP_SIZE})"
echo ""
echo "Contents:"
du -sh "$STAGING_DIR"/{toolchain,sdk,peal,tools,templates,helloworld} 2>/dev/null | sed 's|/tmp/[^/]*/||'

# Clean up staging
rm -rf "$STAGING_DIR"
