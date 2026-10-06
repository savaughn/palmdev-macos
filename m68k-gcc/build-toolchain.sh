#!/bin/bash

##############################################################################
# Builds a modern gcc (C, C++) for Palm OS 68K apps, natively on macOS.
#
# Source: Retro68 (https://github.com/autc04/Retro68), pinned to one commit.
# Retro68's gcc knows __attribute__((__raw_inline__(...))), which the Palm OS
# SDK 5r3 headers in ../sdk use for the system traps with this compiler.
#
# Only binutils and gcc (with libgcc and the freestanding part of libstdc++)
# are built, for the target m68k-none-elf and the 68000. There is no C library:
# Palm OS apps use the Palm OS API, the few C functions the compiler itself
# calls (memcpy, memset, ...) come from runtime/ in this directory.
#
# Result:  m68k-gcc/toolchain/bin/m68k-none-elf-{gcc,g++,ld,objcopy,objdump,...}
# Needs:   Xcode Command Line Tools, Homebrew: gmp mpfr libmpc
##############################################################################

set -euo pipefail

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PREFIX="$SCRIPT_DIR/toolchain"
BUILD_DIR="$SCRIPT_DIR/build"
SRC_DIR="$BUILD_DIR/src/Retro68"
TARGET=m68k-none-elf

RETRO68_REPO="https://github.com/autc04/Retro68.git"
# gcc 16.1.0, binutils 2.46
RETRO68_COMMIT="a4f38da117006a6e4a3e74c8eeeaad9ae736e6f6"

JOBS="$(sysctl -n hw.ncpu 2>/dev/null || nproc)"

die() { echo "error: $*" >&2; exit 1; }

[ "$(uname)" = Darwin ] || echo "note: this script is written for macOS; continuing anyway"

# --- Dependencies ------------------------------------------------------------
command -v brew >/dev/null || die "Homebrew not found"
for pkg in gmp mpfr libmpc; do
    brew list --versions "$pkg" >/dev/null || die "missing Homebrew package '$pkg' (brew install gmp mpfr libmpc)"
done
GMP="$(brew --prefix gmp)"
MPFR="$(brew --prefix mpfr)"
MPC="$(brew --prefix libmpc)"

# --- Sources -----------------------------------------------------------------
mkdir -p "$BUILD_DIR/src"
if [ ! -d "$SRC_DIR/.git" ]; then
    echo "=== Cloning Retro68 ==="
    git clone --filter=blob:none --no-checkout "$RETRO68_REPO" "$SRC_DIR"
    git -C "$SRC_DIR" checkout --detach "$RETRO68_COMMIT"
elif [ "$(git -C "$SRC_DIR" rev-parse HEAD)" != "$RETRO68_COMMIT" ]; then
    die "$SRC_DIR is not at $RETRO68_COMMIT; remove $BUILD_DIR/src to fetch the pinned version"
fi

# Local fixes on top of Retro68 (patches/*.patch). They are applied to the
# clean checkout; a stamp file records the applied set (by content hash).
PATCH_STAMP="$BUILD_DIR/src/retro68-patches.applied"
PATCH_SUM="$(cat "$SCRIPT_DIR"/patches/*.patch | shasum | cut -d' ' -f1)"
if [ "$(cat "$PATCH_STAMP" 2>/dev/null)" != "$PATCH_SUM" ]; then
    git -C "$SRC_DIR" diff --quiet ||
        die "$SRC_DIR has changes from other patches; reset it with: git -C '$SRC_DIR' checkout -- ."
    for patch in "$SCRIPT_DIR"/patches/*.patch; do
        echo "applying $(basename "$patch")"
        git -C "$SRC_DIR" apply "$patch"
    done
    echo "$PATCH_SUM" > "$PATCH_STAMP"
fi

# --- binutils ----------------------------------------------------------------
echo "=== binutils ==="
rm -rf "$BUILD_DIR/binutils" && mkdir -p "$BUILD_DIR/binutils"
(
    cd "$BUILD_DIR/binutils"
    "$SRC_DIR/binutils/configure" \
        --target=$TARGET --prefix="$PREFIX" \
        --disable-nls --disable-werror --disable-doc \
        --disable-gdb --disable-gdbserver --disable-sim --disable-gprofng \
        MAKEINFO=true
    make -j"$JOBS" MAKEINFO=true
    make install MAKEINFO=true
)

# --- gcc ---------------------------------------------------------------------
# --without-headers --without-newlib: no C library; gcc then installs its own
# stdint.h for hosted compiles too. --disable-hosted-libstdcxx: only the
# freestanding C++ headers and libsupc++ (no exceptions/RTTI are used on Palm).
# --enable-initfini-array: constructors in .init_array, which runtime/crt.c runs.
# The target libraries (libgcc) are built with -msep-data like the apps: code
# without absolute addresses, data through A5 (see runtime/startup.c).
echo "=== gcc ==="
export PATH="$PREFIX/bin:$PATH"
rm -rf "$BUILD_DIR/gcc" && mkdir -p "$BUILD_DIR/gcc"
(
    cd "$BUILD_DIR/gcc"
    "$SRC_DIR/gcc/configure" \
        --target=$TARGET --prefix="$PREFIX" \
        --enable-languages=c,c++ \
        --with-cpu=m68000 --disable-multilib \
        --without-headers --without-newlib \
        --disable-hosted-libstdcxx --disable-libstdcxx-verbose \
        --disable-shared --disable-threads --disable-tls --enable-initfini-array \
        --disable-nls --disable-libssp --disable-libquadmath \
        --disable-libgomp --disable-libatomic --disable-libcc1 \
        --with-gmp="$GMP" --with-mpfr="$MPFR" --with-mpc="$MPC" \
        CFLAGS_FOR_TARGET="-g -O2 -msep-data" CXXFLAGS_FOR_TARGET="-g -O2 -msep-data" \
        MAKEINFO=true
    make -j"$JOBS" all-gcc MAKEINFO=true
    make -j"$JOBS" all-target-libgcc MAKEINFO=true
    make -j"$JOBS" all-target-libstdc++-v3 MAKEINFO=true
    make install-gcc install-target-libgcc install-target-libstdc++-v3 MAKEINFO=true
)

echo ""
echo "=== Done ==="
"$PREFIX/bin/$TARGET-gcc" --version | head -1
"$PREFIX/bin/$TARGET-ld" --version | head -1
echo "Toolchain: $PREFIX"
