// A global object with a constructor in a second translation unit.
#include "GccTest.h"

namespace {
struct Second {
	UInt16 value;
	Second() : value(1234) {}
};
Second second;
}

extern "C" UInt16 SecondUnitValue(void)
{
	return second.value;
}
