/*
 * Minimal C++ runtime for Palm OS 68K apps built with the modern gcc.
 * Compile C++ code with -fno-exceptions -fno-rtti -fno-threadsafe-statics.
 *
 *   operator new/delete  on the Palm OS Memory Manager (MemPtrNew/MemPtrFree);
 *                        a failed plain new is a fatal error, as there are no
 *                        exceptions; new (std::nothrow) returns nullptr
 *   (__cxa_atexit, for the destructors of global objects, is in crt.c)
 *   __cxa_pure_virtual   fatal error
 */

#include <PalmOS.h>
#include <new>

extern "C" {

void __cxa_pure_virtual(void)
{
	ErrDisplayFileLineMsg(__FILE__, __LINE__, "pure virtual function called");
}

} // extern "C"

void *operator new(std::size_t size)
{
	void *p = MemPtrNew(size ? size : 1);
	if (p == nullptr)
		ErrDisplayFileLineMsg(__FILE__, __LINE__, "out of memory");
	return p;
}

void *operator new[](std::size_t size)
{
	return operator new(size);
}

void *operator new(std::size_t size, const std::nothrow_t &) noexcept
{
	return MemPtrNew(size ? size : 1);
}

void *operator new[](std::size_t size, const std::nothrow_t &) noexcept
{
	return MemPtrNew(size ? size : 1);
}

void operator delete(void *p) noexcept
{
	if (p != nullptr)
		MemPtrFree(p);
}

void operator delete[](void *p) noexcept
{
	operator delete(p);
}

void operator delete(void *p, std::size_t) noexcept
{
	operator delete(p);
}

void operator delete[](void *p, std::size_t) noexcept
{
	operator delete(p);
}
