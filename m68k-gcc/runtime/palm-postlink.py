#!/usr/bin/env python3
"""
palm-postlink: turns the ELF file of a Palm OS 68K application (modern gcc,
-msep-data, linked -pie with palmos.ld) into the 'code' 1 resource.

The resource is the memory region "rom" of the link (start code, image
header, code, read-only data, initial contents of the RAM image), followed by
the relocation table that startup.c applies at launch:

    UInt16 count
    UInt16 offset[count]   32-bit pointers in the RAM image, relative to its
                           start; each holds a link-time address in rom or ram

The table comes from the R_68K_RELATIVE entries of .rela.dyn. Any other
dynamic relocation (or one outside the RAM image) is an error: it would be an
absolute address in the read-only code, or a reference to an undefined symbol.

usage: palm-postlink.py app.elf code0001.bin
"""

import struct
import sys

R_68K_RELATIVE = 22
PT_LOAD = 1
SHT_RELA = 4
HEADER_MAGIC = 0x50616C6D  # 'Palm'
HEADER_FIELDS = 8          # see palm-image.h
MAX_RESOURCE = 65505


def fail(msg):
    sys.exit(f"palm-postlink: error: {msg}")


class Elf:
    def __init__(self, path):
        with open(path, "rb") as f:
            self.data = f.read()
        d = self.data
        if d[:4] != b"\x7fELF" or d[4] != 1 or d[5] != 2:
            fail(f"{path}: not a 32-bit big-endian ELF file")
        (e_type, e_machine, _, _, e_phoff, e_shoff, _, _, e_phentsize, e_phnum,
         e_shentsize, e_shnum, e_shstrndx) = struct.unpack_from(">HHIIIIIHHHHHH", d, 16)
        if e_machine != 4:
            fail(f"{path}: not an m68k ELF file")
        self.phdrs = [struct.unpack_from(">IIIIIIII", d, e_phoff + i * e_phentsize)
                      for i in range(e_phnum)]
        self.shdrs = [struct.unpack_from(">IIIIIIIIII", d, e_shoff + i * e_shentsize)
                      for i in range(e_shnum)]
        strtab = self.shdrs[e_shstrndx]
        self.names = [self._str(strtab[4], sh[0]) for sh in self.shdrs]

    def _str(self, off, idx):
        end = self.data.index(b"\0", off + idx)
        return self.data[off + idx:end].decode()

    def symbol(self, name):
        for sh in self.shdrs:
            if sh[1] != 2:  # SHT_SYMTAB
                continue
            strtab = self.shdrs[sh[6]]
            for i in range(sh[5] // 16):
                st_name, st_value = struct.unpack_from(">II", self.data, sh[4] + 16 * i)
                if self._str(strtab[4], st_name) == name:
                    return st_value
        return None

    def section(self, name):
        for sh, n in zip(self.shdrs, self.names):
            if n == name:
                return sh
        return None


def main():
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    elf_path, out_path = sys.argv[1:]
    elf = Elf(elf_path)

    for sh, name in zip(elf.shdrs, elf.names):
        if name == ".ctors" and sh[5] > 0:
            fail("unsupported .ctors/.dtors sections (old-style constructors); "
                 "the runtime runs .init_array/.fini_array only")
        if name in (".got", ".got.plt", ".text.startup", ".plt"):
            fail(f"section {name} outside the layout of palmos.ld")

    vec = elf.section(".vec")
    if vec is None:
        fail("no .vec section - not linked with palmos.ld?")
    rom_origin = vec[3]  # sh_addr

    code = bytearray()
    for p_type, p_offset, p_vaddr, p_paddr, p_filesz, _, _, _ in elf.phdrs:
        if p_type != PT_LOAD or p_filesz == 0:
            continue
        if not (rom_origin <= p_paddr < rom_origin + 0x10000):
            continue  # dynamic sections for this tool, not part of the resource
        pos = p_paddr - rom_origin
        if len(code) < pos:
            code.extend(b"\0" * (pos - len(code)))
        code[pos:pos + p_filesz] = elf.data[p_offset:p_offset + p_filesz]

    hdr_addr = elf.symbol("__palm_image_header")
    if hdr_addr is None:
        fail("symbol __palm_image_header not found - not linked with palmos.ld?")
    hdr_pos = hdr_addr - rom_origin
    (magic, image_offset, init_size, ram_size, a5_offset, hdr_rom, ram_origin,
     reloc_offset) = struct.unpack_from(">" + "I" * HEADER_FIELDS, code, hdr_pos)
    if magic != HEADER_MAGIC or hdr_rom != rom_origin or reloc_offset != 0:
        fail("unexpected image header (already post-linked?)")
    if image_offset + init_size != len(code):
        fail(f"RAM image ({image_offset:#x}+{init_size:#x}) is not at the end "
             f"of the code ({len(code):#x})")
    if a5_offset + 4 > init_size:
        fail("no global offset table at A5 (compile with -msep-data)")
    if ram_size > 0xFFFF:
        fail(f"globals too large: {ram_size} bytes (max. 64 KB)")

    rom_end = rom_origin + image_offset
    ram_end = ram_origin + ram_size
    offsets = []
    for sh, name in zip(elf.shdrs, elf.names):
        if sh[1] != SHT_RELA:
            continue
        for i in range(sh[5] // 12):
            r_offset, r_info, r_addend = struct.unpack_from(">IIi", elf.data, sh[4] + 12 * i)
            r_type = r_info & 0xFF
            if r_type != R_68K_RELATIVE:
                fail(f"{name}: unsupported dynamic relocation type {r_type} at "
                     f"{r_offset:#x} (symbol #{r_info >> 8}) - undefined symbol?")
            if not (ram_origin <= r_offset and r_offset + 4 <= ram_origin + init_size):
                fail(f"{name}: relocation at {r_offset:#x} outside the RAM image - "
                     "absolute address in code? (compile everything with -msep-data)")
            if r_offset == ram_origin + a5_offset:
                fail(f"{name}: relocation of the word at A5 (reserved for Palm OS)")
            if r_offset & 1:
                fail(f"{name}: misaligned relocation at {r_offset:#x}")
            target = r_addend & 0xFFFFFFFF
            if not (rom_origin <= target <= rom_end or ram_origin <= target <= ram_end):
                fail(f"{name}: relocation at {r_offset:#x} points to {target:#x}, "
                     "neither code nor globals")
            # RELA: the addend is authoritative, store it in the image
            struct.pack_into(">I", code, image_offset + (r_offset - ram_origin), target)
            offsets.append(r_offset - ram_origin)

    if len(code) & 1:
        code.append(0)
    reloc_pos = len(code)
    code += struct.pack(">H", len(offsets))
    for off in sorted(offsets):
        code += struct.pack(">H", off)
    struct.pack_into(">I", code, hdr_pos + 4 * (HEADER_FIELDS - 1), reloc_pos)

    # Largest Memory Manager chunk, and so the largest resource, on Palm OS
    if len(code) > MAX_RESOURCE:
        fail(f"code resource too large: {len(code)} bytes (max. {MAX_RESOURCE})")
    with open(out_path, "wb") as f:
        f.write(code)
    print(f"palm-postlink: {out_path}: {len(code)} bytes "
          f"(code {image_offset}, initialized globals {init_size}, "
          f"globals total {ram_size}, relocations {len(offsets)}), A5 offset {a5_offset}")


if __name__ == "__main__":
    main()
