/*
 * C runtime for Palm OS 68K apps built with the modern gcc (-msep-data).
 *
 * __palm_run() is called by startup.c with A5 pointing at the application's
 * globals. It runs the static constructors, PilotMain and the destructors:
 * those registered with __cxa_atexit (global C++ objects, in reverse order of
 * construction), then .fini_array (__attribute__((destructor))).
 *
 * Also here: the few C library functions gcc may call on its own (struct
 * copies and initializations), since there is no C library on Palm OS.
 */

#include <PalmOS.h>

typedef void (*PalmInitFunc)(void);

/* Defined in palmos.ld */
extern PalmInitFunc __preinit_array_start[], __preinit_array_end[];
extern PalmInitFunc __init_array_start[], __init_array_end[];
extern PalmInitFunc __fini_array_start[], __fini_array_end[];

/* The application's entry point (extern "C" in C++). */
UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags);

/* Destructors of global C++ objects, registered by their constructors. */
typedef struct AtExit {
	struct AtExit *next;
	void (*func)(void *);
	void *arg;
} AtExit;

static AtExit *atExitList;

void *__dso_handle __attribute__((visibility("hidden"))) = &__dso_handle;

int __cxa_atexit(void (*func)(void *), void *arg, void *dso)
{
	AtExit *e = (AtExit *)MemPtrNew(sizeof(AtExit));

	(void)dso;
	if (e == NULL)
		return -1;
	e->func = func;
	e->arg = arg;
	e->next = atExitList;
	atExitList = e;
	return 0;
}

static void PrvRunAtExit(void)
{
	while (atExitList != NULL) {
		AtExit *e = atExitList;

		atExitList = e->next;
		e->func(e->arg);
		MemPtrFree(e);
	}
}

UInt32 __attribute__((section(".vectors.palm")))
__palm_run(const SysAppInfoType *appInfoP, UInt16 newGlobals)
{
	PalmInitFunc *f;
	UInt32 result;

	if (newGlobals) {
		for (f = __preinit_array_start; f < __preinit_array_end; f++)
			(*f)();
		for (f = __init_array_start; f < __init_array_end; f++)
			(*f)();
	}

	result = PilotMain((UInt16)appInfoP->cmd, appInfoP->cmdPBP, appInfoP->launchFlags);

	if (newGlobals) {
		PrvRunAtExit();
		for (f = __fini_array_end; f > __fini_array_start; )
			(*--f)();
	}

	return result;
}

/*
 * gcc must not turn these loops into calls of the very functions they
 * implement.
 */
#define LIBC_FUNC __attribute__((optimize("no-tree-loop-distribute-patterns")))

typedef unsigned long size_t;

LIBC_FUNC void *memcpy(void *dst, const void *src, size_t n)
{
	UInt8 *d = (UInt8 *)dst;
	const UInt8 *s = (const UInt8 *)src;

	while (n--)
		*d++ = *s++;
	return dst;
}

LIBC_FUNC void *memmove(void *dst, const void *src, size_t n)
{
	UInt8 *d = (UInt8 *)dst;
	const UInt8 *s = (const UInt8 *)src;

	if (d <= s || d >= s + n) {
		while (n--)
			*d++ = *s++;
	} else {
		d += n;
		s += n;
		while (n--)
			*--d = *--s;
	}
	return dst;
}

LIBC_FUNC void *memset(void *dst, int c, size_t n)
{
	UInt8 *d = (UInt8 *)dst;

	while (n--)
		*d++ = (UInt8)c;
	return dst;
}

LIBC_FUNC int memcmp(const void *a, const void *b, size_t n)
{
	const UInt8 *p = (const UInt8 *)a;
	const UInt8 *q = (const UInt8 *)b;

	for (; n; n--, p++, q++)
		if (*p != *q)
			return *p < *q ? -1 : 1;
	return 0;
}
