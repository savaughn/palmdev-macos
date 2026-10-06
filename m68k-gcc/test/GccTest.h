#ifndef GCCTEST_H
#define GCCTEST_H

#include <PalmOS.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Appends a line to the result database. */
void TestLog(const char *line);
void TestCheck(const char *name, Boolean ok);

/* checks.c: C side (initialized data with pointers, function pointers). */
void RunCChecks(void);

/* second.cpp: a global object in another translation unit. */
UInt16 SecondUnitValue(void);

#ifdef __cplusplus
}
#endif

#endif
