# PEAL - Palm Executive ARM Loader

PEAL runs native ARM applications on Palm OS 5 devices.
The ARM application is wrapped in a 68K stub that loads the ARM code at runtime.

## Source

Original author: Greg Parker (2004)
Taken from the TCPMP project: https://github.com/Erikhht/TCPMP (`common/palmos/peal/`)

## Directory structure

```
peal/
  postlink/       Host tool (C++), compiled with Apple Clang
    peal-postlink  Turns an ARM ELF file into Palm OS resources
  m68k/            68K loader (compiled into the stub)
    peal.c/h       Loads ARM code from 'armc' resources
  arm/             ARM entry point
    pealstub.c/h   Must be linked as the first object
```

## Build (host tool)

```bash
cd postlink
c++ -O2 -W -Wall image.cc postlinker.cc relocation.cc section.cc \
    symbol.cc symboltable.cc complain.cc -o peal-postlink
```

The compiled binary is located at `toolchain/toolchain/bin/peal-postlink`.

## Usage

```bash
# Convert an ARM ELF file into a Palm OS resource
peal-postlink -s 1000 -o output.ro input.elf
```

Options:
- `-s <resID>`: create .ro format with resource type 'armc', starting at the given ID
- `-t <resType>`: set the resource type (default: 'armc')
- `-o <output>`: output file
- `-v`: verbose output

## License

BSD 2-Clause License (see the source files)
