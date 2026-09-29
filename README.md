# Palm OS Development for macOS

[![Build Palm OS App](https://github.com/User7142/palmdev-macos/actions/workflows/build.yml/badge.svg)](https://github.com/User7142/palmdev-macos/actions/workflows/build.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![GitHub issues](https://img.shields.io/github/issues/User7142/palmdev-macos)](https://github.com/User7142/palmdev-macos/issues)

A complete Palm OS development environment for macOS ARM (Apple Silicon) with pre-built toolchain, SDK, and hello world template. Supports both Palm OS 4 (68K) and Palm OS 5 (native ARM via PEAL).

> This is a fork of [savaughn/palmdev-macos](https://github.com/savaughn/palmdev-macos). It adds Palm OS 5 (ARM) support via PEAL, the Palm OS SDK 5r3, a dual-target Makefile and a portable ZIP build. Background and a walkthrough: [palm2000.com](https://palm2000.com/articles/46)

## Quick Start

```bash
# Clone this repository
git clone https://github.com/User7142/palmdev-macos.git palmdev-macos
cd palmdev-macos

# Run setup (downloads toolchain and SDK)
./setup-toolchain.sh

# Build hello world
cd helloworld
make build

# Output: build/HelloWorld.prc ready to install on Palm device!
```

## What's Included

- **Pre-built prc-tools** - m68k-palmos-gcc (68K) and arm-palmos-gcc (ARM) compilers for macOS ARM
- **Palm OS SDK** - Headers and libraries for Palm OS 4 and 5r3
- **PEAL** - Palm Executive ARM Loader for native ARM apps on Palm OS 5
- **Build tools** - pilrc, build-prc, peal-postlink, pilot-link
- **Dual-target Makefile template** - Build for Palm OS 4 or 5 with one command
- **Hello World template** - Ready-to-build sample application
- **Automated setup** - One-command installation

## Requirements

- macOS 11+ (Big Sur or later)
- Apple Silicon
- Homebrew installed
- ~100MB disk space

## What Gets Installed

The setup script will:

1. **Download prc-tools** from [GitHub releases](https://github.com/savaughn/prc-tools-remix/releases/tag/macos-arm)
   - Installs to local `toolchain/` directory
   - Includes m68k-palmos-gcc 2.95.3
   - Installs target directories to `/usr/local/{arm,m68k}-palmos/`
   
2. **Install support tools** via Homebrew
   - pilrc (resource compiler)
   - pilot-link (device communication)
   
3. **Download Palm OS SDK** from [jichu4n/palm-os-sdk](https://github.com/jichu4n/palm-os-sdk)
   - Installs to `/usr/local/palmdev/`
   - Includes SDKs for Palm OS 1.0 through 5r4
   
4. **Configure toolchain** with palmdev-prep
   - Writes GCC specs files to `/usr/local/lib/gcc-lib/`
   - Writes trap numbers to `/usr/local/share/prc-tools/`
   - Configures SDK paths for compilation


## Building Applications

The Makefile automatically finds the toolchain - no need to source any config files!

```bash
# Build the hello world app (Palm OS 4, 68K)
cd helloworld
make build
```

All build output goes to the `build/` directory.

## Palm OS 5 ARM Development

For native ARM performance on Palm OS 5 devices (e.g. Tungsten T3), use the dual-target Makefile template:

```bash
# Create a new project from templates
mkdir myapp && cd myapp
cp ../templates/Makefile .
cp ../templates/app.rcp .
cp ../templates/palmos5/main_arm.c .
cp ../templates/palmos5/stub_m68k.c .
cp ../templates/palmos5/pace.h .

# Edit Makefile: set APP_NAME, CREATOR_ID, ARM_SOURCES

# Build for Palm OS 5 (ARM native with 68K stub)
make build TARGET=palmos5

# Or build for Palm OS 4 (pure 68K) with the same Makefile
cp ../templates/palmos4/main.c .
make build TARGET=palmos4
```

### How ARM Apps Work

A Palm OS 5 ARM app consists of:
1. **68K stub** - loaded by Palm OS, detects ARM CPU, loads ARM code via PEAL
2. **ARM binary** - native ARM code packed as `armc` resources by `peal-postlink`
3. **PACE wrappers** (`pace.h`) - call Palm OS APIs from ARM via 68K emulator callbacks

### PEAL (Palm Executive ARM Loader)

PEAL loads and executes ARM code on Palm OS 5. Components:
- `peal/postlink/` - Host tool that converts ARM ELF to Palm OS resources
- `peal/m68k/` - 68K loader compiled into the stub
- `peal/arm/` - ARM entry point (`pealstub.c`, must be linked first)

## Installing on Palm Device

### Via HotSync
```bash
# Install to Palm via USB
pilot-xfer -i build/HelloWorld.prc
```

### Via POSE (Palm OS Emulator)
Drag and drop the `build/HelloWorld.prc` file onto the POSE window.

### Via Palm Desktop
Copy the `build/HelloWorld.prc` file to your HotSync install folder.

## Troubleshooting

### "curl: command not found"
Install Xcode Command Line Tools:
```bash
xcode-select --install
```

### "brew: command not found"
Install Homebrew:
```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

### "palmdev-prep: Permission denied"
The SDK installation requires sudo. You'll be prompted for your password.

### Compilation errors with system headers
Make sure you're using the flags in the Makefile:
```bash
m68k-palmos-gcc -palmos3.5 -nostdinc ...
```

The `-nostdinc` flag prevents macOS system headers from conflicting with Palm OS headers.

### "No genuine PalmOS.h found"
Run palmdev-prep to configure the SDK:
```bash
export PATH="$PWD/toolchain/usr/local/bin:$PATH"
sudo -E env "PATH=$PATH" palmdev-prep
```

## Documentation

- [Palm OS Programming Tutorial](http://www.palmos.com/dev/support/docs/)
- [prc-tools Documentation](http://prc-tools.sourceforge.net/)
- [Palm OS Reference](https://stuff.mit.edu/afs/sipb/project/palm/doc/)

## Next Steps

After building hello world:

1. **Read the source** - `helloworld/HelloWorld.c` is fully commented
2. **Modify the UI** - Edit `helloworld/HelloWorld.rcp` for resources
3. **Study Palm OS APIs** - Check SDK headers in `sdk/sdk-4/include/` or `sdk/sdk-5r3/include/`
4. **Try ARM development** - Use `templates/` for a Palm OS 5 ARM project
5. **Build something cool** - Games, utilities, audio apps, MIDI synths!

## 🔄 Updating

To update the toolchain:

```bash
# Re-run setup to get latest versions
./setup-toolchain.sh
```

## Contributing

Found a bug? Have a feature request? Want to share your Palm OS app?

- Open an issue
- Submit a pull request
- Share your creations!

## License

This template and setup scripts are provided as-is for Palm OS development.

- prc-tools: GPL
- Palm OS SDK: Palm OS License (educational/non-commercial use)
- Support tools: Various open source licenses

## Credits

- [jichu4n](https://github.com/jichu4n) - Palm OS SDK and Homebrew tap
- [savaughn](https://github.com/savaughn) - prc-tools macOS ARM builds
- Palm, Inc. - Original Palm OS SDK and documentation
- prc-tools community - Maintaining the GCC toolchain

## Architecture Note

These pre-built binaries are for **macOS ARM (Apple Silicon)** only.

For Intel Macs, you're already covered by existing homebrew packages.
---
