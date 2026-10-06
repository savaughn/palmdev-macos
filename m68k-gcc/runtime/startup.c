/*
 * Palm OS 68K start code for the modern gcc (m68k-none-elf, -msep-data).
 *
 * Palm OS enters an application at offset 0 of its 'code' 1 resource. That
 * resource stays in the write-protected storage heap, so code and read-only
 * data run from there, while everything writable (the global offset table,
 * .data, .bss) lives in a block of the dynamic heap that this code sets up:
 *
 *   1. MemPtrNew() a block of the size of the RAM image (.data incl. GOT, .bss)
 *   2. copy the initial contents stored in the code resource, clear .bss
 *   3. apply the relocation table that palm-postlink appended: each entry is
 *      a 32-bit pointer in the RAM image that the linker filled with a link
 *      time address in the code (rom) or in the RAM image (ram)
 *   4. point A5 at the global offset table and call __palm_run() (crt.c),
 *      which runs the C++ constructors, PilotMain and the destructors
 *
 * Palm OS keeps the pointer to the current SysAppInfoType at 0(A5) (see
 * SysGetAppInfo in the ROM); startup copies it into the first GOT word,
 * which the ELF ABI reserves (for _DYNAMIC) and nothing here uses.
 *
 * Every launch gets its own globals, except a sub-call (the application
 * calling itself through SysAppLaunch), which keeps the caller's A5. With
 * -msep-data even calls of non-static functions and string constants go
 * through the GOT, so the GOT has to exist for every launch code, not only
 * for those with sysAppLaunchFlagNewGlobals.
 *
 * This file is compiled WITHOUT -msep-data (with -mpcrel -fpic), because it
 * runs before A5 is valid: everything it touches is addressed pc-relative.
 * All of it lives in the .vectors sections at the start of the code resource,
 * so the 16-bit pc-relative displacements of the 68000 always reach.
 */

#include <PalmOS.h>

#include "palm-image.h"

#define STARTUP __attribute__((section(".vectors.palm")))

extern const PalmImageHeader __palm_image_header;
extern UInt8 __palm_code_start[];

UInt32 __palm_run(const SysAppInfoType *appInfoP, UInt16 newGlobals);
UInt32 __palm_enter(void *a5, const SysAppInfoType *appInfoP);

/* Calls __palm_run(appInfoP, 1) with A5 = a5 and restores A5 afterwards. */
__asm__(
    "	.section .vectors.palm,\"ax\",@progbits\n"
    "	.even\n"
    "__palm_enter:\n"
    "	move.l	%a5,-(%sp)\n"
    "	move.l	8(%sp),%a5\n"           /* a5 */
    "	move.w	#1,-(%sp)\n"            /* newGlobals = 1 (UInt16) */
    "	move.l	14(%sp),-(%sp)\n"       /* appInfoP */
    "	bsr.w	__palm_run\n"
    "	addq.l	#6,%sp\n"
    "	move.l	(%sp)+,%a5\n"
    "	rts\n"
    "	.previous\n");

/* Allocates and initializes the RAM image; returns its A5 value or NULL. */
static STARTUP void *PrvGlobalsNew(void **blockP)
{
	UInt32 *osA5, *a5;
	const PalmImageHeader *hdr = &__palm_image_header;
	const UInt8 *image;
	const UInt16 *reloc;
	UInt8 *ram;
	UInt32 codeDelta, ramDelta;
	UInt16 n;

	if (hdr->magic != palmImageMagic || hdr->relocOffset == 0)
		return NULL;     /* not processed by palm-postlink */

	ram = (UInt8 *)MemPtrNew(hdr->ramSize ? hdr->ramSize : 4);
	if (ram == NULL)
		return NULL;

	image = __palm_code_start + hdr->imageOffset;
	if (hdr->initSize)
		MemMove(ram, image, (Int32)hdr->initSize);
	if (hdr->ramSize > hdr->initSize)
		MemSet(ram + hdr->initSize, (Int32)(hdr->ramSize - hdr->initSize), 0);

	codeDelta = (UInt32)__palm_code_start - hdr->romOrigin;
	ramDelta = (UInt32)ram - hdr->ramOrigin;
	reloc = (const UInt16 *)(__palm_code_start + hdr->relocOffset);
	for (n = *reloc++; n; n--) {
		UInt32 *slot = (UInt32 *)(ram + *reloc++);
		*slot += (*slot >= hdr->ramOrigin) ? ramDelta : codeDelta;
	}

	/* The SysAppInfoType pointer of this launch, as SysAppStartup left it */
	__asm__("move.l %%a5,%0" : "=d"(osA5));
	osA5 = (UInt32 *)((UInt32)osA5 & 0x7FFFFFFFUL);   /* as SysGetAppInfo does */
	a5 = (UInt32 *)(ram + hdr->a5Offset);
	a5[0] = osA5[0];

	*blockP = ram;
	return a5;
}

/* Entry point: offset 0 of the code resource (first in .vectors). */
UInt32 __attribute__((section(".vectors"), used)) __Startup__(void)
{
	SysAppInfoPtr appInfoP;
	MemPtr prevGlobalsP, globalsP;
	UInt32 result;

	if (SysAppStartup(&appInfoP, &prevGlobalsP, &globalsP) != errNone) {
		SndPlaySystemSound(sndError);
		return (UInt32)-1;
	}

	if (appInfoP->launchFlags & sysAppLaunchFlagSubCall) {
		/* The application calls itself: A5 already points at its globals. */
		result = __palm_run(appInfoP, 0);
	} else {
		void *block;
		void *a5 = PrvGlobalsNew(&block);

		if (a5 != NULL) {
			result = __palm_enter(a5, appInfoP);
			MemPtrFree(block);
		} else {
			SndPlaySystemSound(sndError);
			result = memErrNotEnoughSpace;
		}
	}

	SysAppExit(appInfoP, prevGlobalsP, globalsP);
	return result;
}
