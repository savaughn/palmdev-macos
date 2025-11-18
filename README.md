# Palm OS Development for macOS

A complete Palm OS development environment for macOS ARM (Apple Silicon) with pre-built toolchain, SDK, and hello world template.

## Quick Start

```bash
# Clone this repository
git clone https://github.com/savaughn/palmdev-macos.git palmdev-macos
cd palmdev-macos

# Run setup (downloads toolchain and SDK)
./setup-toolchain.sh

# Build hello world
cd helloworld
make build

# Output: build/HelloWorld.prc ready to install on Palm device!
```

## What's Included

- **Pre-built prc-tools** - m68k-palmos-gcc compiler for macOS ARM
- **Palm OS SDK** - Headers and libraries for Palm OS
- **Build tools** - pilrc, build-prc, pilot-link
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
# Build the hello world app
cd helloworld
make build
```

All build output goes to the `build/` directory.

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
3. **Study Palm OS APIs** - Check SDK headers in `/usr/local/palmdev/sdk-3.5/include/`
4. **Build something cool** - Games, utilities, network apps!

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
