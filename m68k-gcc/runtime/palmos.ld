/*
 * Linker script for Palm OS 68K applications built with the modern gcc
 * (m68k-none-elf, -msep-data), linked with -pie.
 *
 * rom: the 'code' 1 resource. Start code first (Palm OS enters at offset 0),
 *      then the image header, code, read-only data and the initial contents
 *      of the RAM image. palm-postlink appends the relocation table.
 * ram: the globals block that startup.c allocates at launch, with A5 pointing
 *      at the global offset table: GOT, .data, .bss.
 * The dynamic sections of the -pie link only serve palm-postlink, which reads
 * the R_68K_RELATIVE entries of .rela.dyn; they are not part of the resource.
 *
 * Based on the linker script of Dmitry Grinberg's palmdev package (2020).
 */

MEMORY
{
	rom   : ORIGIN = 0x10000000, LENGTH = 64K
	ram   : ORIGIN = 0x20000000, LENGTH = 64K
	other : ORIGIN = 0x40000000, LENGTH = 16M
}

ENTRY(__Startup__)

SECTIONS
{
	.vec : {
		__palm_code_start = .;
		KEEP(*(.vectors))
		*(.vectors.*)
		. = ALIGN(4);
		__palm_image_header = .;
		LONG(0x50616C6D)                                /* magic 'Palm' */
		LONG(LOADADDR(.data) - ORIGIN(rom))             /* imageOffset */
		LONG(SIZEOF(.data))                             /* initSize */
		LONG(__palm_ram_end - ORIGIN(ram))              /* ramSize */
		LONG(_GLOBAL_OFFSET_TABLE_ - ORIGIN(ram))       /* a5Offset */
		LONG(ORIGIN(rom))                               /* romOrigin */
		LONG(ORIGIN(ram))                               /* ramOrigin */
		LONG(0)                                         /* relocOffset: palm-postlink */
	} > rom

	/* MacsBug function names emitted by Retro68's gcc: not needed on Palm OS */
	/DISCARD/ : { *(*.macsbug) *(.init) *(.fini) *(.eh_frame) *(.comment) }

	.text : { *(.text .text.*) } > rom

	.rodata : { *(.rodata .rodata.*) . = ALIGN(4); } > rom

	.data : {
		*(.got.plt) *(.got)
		*(.data .data.*)
		. = ALIGN(4);
		__preinit_array_start = .;
		KEEP(*(.preinit_array))
		__preinit_array_end = .;
		__init_array_start = .;
		KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*)))
		KEEP(*(.init_array))
		__init_array_end = .;
		__fini_array_start = .;
		KEEP(*(SORT_BY_INIT_PRIORITY(.fini_array.*)))
		KEEP(*(.fini_array))
		__fini_array_end = .;
		. = ALIGN(4);
	} > ram AT> rom

	.bss (NOLOAD) : {
		*(.bss .bss.*) *(COMMON)
		. = ALIGN(4);
		__palm_ram_end = .;
	} > ram

	/* Old-style constructor tables: not supported (gcc is configured with
	   --enable-initfini-array); kept here so that palm-postlink rejects them
	   instead of --gc-sections dropping them silently. */
	.ctors    : { KEEP(*(.ctors .ctors.* .dtors .dtors.*)) } > other

	.dynamic  : { *(.dynamic) }  > other
	.dynsym   : { *(.dynsym) }   > other
	.dynstr   : { *(.dynstr) }   > other
	.hash     : { *(.hash) }     > other
	.gnu.hash : { *(.gnu.hash) } > other
	.rela.dyn : { *(.rela.*) }   > other
	.interp   : { *(.interp) }   > other
}
