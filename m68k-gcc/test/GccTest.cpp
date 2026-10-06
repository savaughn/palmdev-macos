/*
 * GccTest: checks the modern gcc and its Palm OS runtime on a device or in
 * an emulator. Every launch appends its results to the database
 * "GccTestResults" (one record per line, "OK ..." or "FAIL ..."):
 *
 *   normal launch   C++17 checks, C checks (checks.c); the destructor of a
 *                   global object logs after PilotMain has returned
 *   system reset    (sent to every app after a soft reset, without
 *                   sysAppLaunchFlagNewGlobals) checks that calls and string
 *                   constants work in such a launch as well
 */

#include "GccTest.h"

#include <array>
#include <cstdint>
#include <optional>
#include <type_traits>

#define RESULT_DB "GccTestResults"

namespace {

// --- compile time -----------------------------------------------------------
constexpr Int32 factorial(Int16 n) { return n <= 1 ? 1 : n * factorial(n - 1); }
static_assert(factorial(10) == 3628800L, "constexpr recursion");
static_assert(sizeof(int) == 2, "-mshort: int is 16 bit like on Palm OS");
static_assert(sizeof(void *) == 4 && sizeof(long) == 4, "pointers and long");

enum class Verb : std::uint8_t { Walk, Look, Use };
static_assert(sizeof(Verb) == 1 && std::is_enum_v<Verb>, "enum class");

// --- globals ------------------------------------------------------------------
UInt16 constructionOrder;

struct Tracker {
	UInt16 order;
	UInt16 magic;
	explicit Tracker(UInt16 m) : order(++constructionOrder), magic(m) {}
	~Tracker();
};

struct Settings {
	static inline UInt16 counter = 7;           // C++17 inline variable
	static constexpr Coord width = 160;
};

class Shape {
public:
	virtual ~Shape() = default;
	virtual Int32 area() const = 0;
	virtual const char *name() const { return "shape"; }
};

class Rect : public Shape {
	Int16 w, h;
public:
	constexpr Rect(Int16 w_, Int16 h_) : w(w_), h(h_) {}
	Int32 area() const override { return (Int32)w * h; }
	const char *name() const override { return "rect"; }
};

class Square final : public Rect {
public:
	constexpr explicit Square(Int16 s) : Rect(s, s) {}
	const char *name() const override { return "square"; }
};

Tracker first(0x1111);
Tracker second(0x2222);
Rect rect(3, 4);
Square square(5);
Shape *const shapes[] = { &rect, &square };     // pointers into .bss/.data
UInt16 zeroed;
UInt16 initialized = 0xBEEF;
UInt16 destructorsRun;

Tracker::~Tracker()
{
	++destructorsRun;
	if (magic == 0x1111) {
		// Destroyed last (reverse order of construction), after PilotMain.
		TestCheck("C++: global destructors in reverse order after PilotMain",
		          destructorsRun == constructionOrder);
		TestLog("END launch");
	}
}

UInt16 localStaticInit()
{
	static Tracker local(0x3333);                // function-local static
	return local.magic;
}

void cxxChecks()
{
	TestCheck("C++: global constructors ran in order",
	          first.order == 1 && second.order == 2 && first.magic == 0x1111);
	TestCheck("C++: global constructor in other unit", SecondUnitValue() == 1234);
	TestCheck("C++: .bss zero, .data initialized", zeroed == 0 && initialized == 0xBEEF);
	TestCheck("C++: static inline member", ++Settings::counter == 8 && Settings::width == 160);

	Int32 total = 0;
	for (const Shape *s : shapes)
		total += s->area();
	TestCheck("C++: virtual calls via relocated pointers", total == 12 + 25
	          && StrCompare(shapes[1]->name(), "square") == 0);

	Shape *heap = new Square(6);
	Boolean heapOk = heap->area() == 36;
	delete heap;                                 // virtual destructor, operator delete
	TestCheck("C++: new/delete with virtual destructor", heapOk);

	UInt16 *array = new UInt16[100]();
	Boolean arrayOk = array[99] == 0;
	delete[] array;
	TestCheck("C++: new[]/delete[] value-initialized", arrayOk);

	TestCheck("C++: function-local static", localStaticInit() == 0x3333
	          && localStaticInit() == 0x3333 && constructionOrder == 3);

	Int16 base = 10;
	auto addBase = [base](Int16 x) { return x + base; };
	TestCheck("C++: lambda with capture", addBase(5) == 15);

	std::array<UInt8, 4> bytes{ 1, 2, 3, 4 };
	UInt16 sum = 0;
	for (UInt8 b : bytes)
		sum += b;
	std::optional<Verb> verb = Verb::Use;
	TestCheck("C++: freestanding std::array, std::optional",
	          sum == 10 && verb.has_value() && *verb == Verb::Use);

	if constexpr (sizeof(int) == 2)
		TestCheck("C++: if constexpr, nullptr", shapes[0] != nullptr);

	struct Big { UInt32 v[40]; };
	Big a{}, b;
	a.v[39] = 0xCAFE;
	b = a;                                       // memcpy
	TestCheck("C++: struct copy (memcpy/memset)", b.v[39] == 0xCAFE && b.v[0] == 0);
}

} // namespace

// --- result database ----------------------------------------------------------
extern "C" void TestLog(const char *line)
{
	LocalID id = DmFindDatabase(0, RESULT_DB);
	if (id == 0) {
		if (DmCreateDatabase(0, RESULT_DB, 'GcTs', 'DATA', false) != errNone)
			return;
		id = DmFindDatabase(0, RESULT_DB);
	}
	DmOpenRef db = DmOpenDatabase(0, id, dmModeReadWrite);
	if (db == nullptr)
		return;
	UInt16 index = dmMaxRecordIndex;
	UInt16 len = StrLen(line) + 1;
	MemHandle h = DmNewRecord(db, &index, len);
	if (h != nullptr) {
		DmWrite(MemHandleLock(h), 0, line, len);
		MemHandleUnlock(h);
		DmReleaseRecord(db, index, true);
	}
	DmCloseDatabase(db);
}

extern "C" void TestCheck(const char *name, Boolean ok)
{
	char line[80];
	StrCopy(line, ok ? "OK   " : "FAIL ");
	StrNCat(line, name, sizeof(line));
	TestLog(line);
}

extern "C" UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
	if (cmd == sysAppLaunchCmdNormalLaunch) {
		TestLog("BEGIN normal launch");
		cxxChecks();
		RunCChecks();
	} else if (cmd == sysAppLaunchCmdSystemReset) {
		TestCheck("reset launch (no OS globals): calls, strings, globals",
		          first.magic == 0x1111 && SecondUnitValue() == 1234);
	}
	return errNone;
}
