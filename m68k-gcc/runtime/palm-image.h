/*
 * Header that the linker script (palmos.ld) puts right after the start code
 * at the beginning of the code resource. It describes the RAM image (GOT,
 * .data, .bss) for startup.c. palm-postlink fills in relocOffset.
 */
#ifndef PALM_IMAGE_H
#define PALM_IMAGE_H

#define palmImageMagic 0x50616C6DUL /* 'Palm' */

typedef struct {
	UInt32 magic;        /* palmImageMagic */
	UInt32 imageOffset;  /* initial contents of the RAM image, offset in the code resource */
	UInt32 initSize;     /* bytes of it (GOT and .data) */
	UInt32 ramSize;      /* size of the RAM image including .bss */
	UInt32 a5Offset;     /* _GLOBAL_OFFSET_TABLE_ in the RAM image: the value of A5 */
	UInt32 romOrigin;    /* link address of the code resource */
	UInt32 ramOrigin;    /* link address of the RAM image */
	UInt32 relocOffset;  /* relocation table, offset in the code resource (palm-postlink) */
} PalmImageHeader;

#endif
