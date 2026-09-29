/*
 * peal_compat.h - Kompatibilitaets-Header fuer m68k-palmos-gcc mit -nostdinc
 *
 * Stellt stdint.h und stddef.h Typen bereit, die in der Palm-OS-Umgebung
 * nicht verfuegbar sind. Muss VOR allen anderen Includes eingebunden werden.
 */

#ifndef PEAL_COMPAT_H
#define PEAL_COMPAT_H

/* stdint.h Typen fuer m68k (32-bit, Big Endian) */
#ifndef _STDINT_H
#define _STDINT_H
typedef unsigned char      uint8_t;
typedef signed char        int8_t;
typedef unsigned short     uint16_t;
typedef signed short       int16_t;
typedef unsigned long      uint32_t;
typedef signed long        int32_t;
typedef unsigned long      uintptr_t;
#endif

/* stddef.h */
#ifndef _STDDEF_H
#define _STDDEF_H
#ifndef NULL
#define NULL ((void *)0)
#endif
typedef unsigned long size_t;
typedef long ptrdiff_t;
#endif

#endif /* PEAL_COMPAT_H */
