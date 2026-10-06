/* C checks: pointer tables in initialized data, 32/64-bit and float math. */
#include "GccTest.h"

static Int16 add(Int16 a, Int16 b) { return a + b; }
static Int16 mul(Int16 a, Int16 b) { return a * b; }

typedef struct { const char *name; Int16 (*fn)(Int16, Int16); } Op;
static const Op ops[] = { { "add", add }, { "mul", mul } };   /* relocated */
static const char *const names[] = { "zero", "one", "two" };
static char buffer[16] = "abc";
static UInt32 bssWord;

static volatile Int32 v32a = 100000, v32b = -7;
static volatile unsigned long long v64a = 0x123456789ULL;
static volatile float fa = 1.5f, fb = 2.25f;
static volatile double da = 10.0;

static const char *shadeName(UInt16 i)
{
	switch (i) {   /* dense switch: jump table */
	case 0: return "white";
	case 1: return "lightgrey";
	case 2: return "darkgrey";
	case 3: return "black";
	case 4: return "four";
	case 5: return "five";
	default: return "?";
	}
}

void RunCChecks(void)
{
	TestCheck("C: function pointer table in .data",
	          ops[0].fn(3, 4) == 7 && ops[1].fn(3, 4) == 12 && StrCompare(ops[1].name, "mul") == 0);
	TestCheck("C: string pointer table", StrCompare(names[2], "two") == 0);
	TestCheck("C: initialized char array", buffer[0] == 'a' && buffer[3] == 0);
	buffer[0] = 'x';
	TestCheck("C: writable global", buffer[0] == 'x');
	TestCheck("C: .bss zero", bssWord == 0);
	TestCheck("C: 32-bit mul/div (libgcc)", v32a * v32b == -700000L && v32a / v32b == -14285L
	          && v32a % v32b == 5);
	TestCheck("C: 64-bit div (libgcc)", v64a / 1000 == 4886718ULL && v64a % 1000 == 345);
	TestCheck("C: float (soft-float libgcc)", fa * fb == 3.375f && (Int32)(da / 4.0) == 2);
	TestCheck("C: switch jump table", StrCompare(shadeName(3), "black") == 0
	          && StrCompare(shadeName(5), "five") == 0);
}
