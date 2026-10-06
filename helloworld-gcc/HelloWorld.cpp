/*
 * HelloWorld for Palm OS 68K, built with the modern gcc (C++17).
 *
 * Besides showing a form it checks a few C++ features that need the runtime
 * in m68k-gcc/runtime: global objects with constructors and destructors,
 * virtual functions, initialized and zeroed globals, function pointers and
 * pointers in initialized data (relocated at launch).
 */

#include <PalmOS.h>
#include <cstdint>
#include <type_traits>

namespace {

constexpr Int16 square(Int16 x) { return x * x; }
static_assert(square(12) == 144, "constexpr evaluation");

enum class Shade : std::uint8_t { White, Grey, Black };
static_assert(sizeof(Shade) == 1, "enum class with underlying type");

struct Config {
	static inline UInt16 launches = 0;            // C++17 inline variable
	static constexpr Coord lineHeight = 12;
};

class Greeting {
public:
	virtual ~Greeting() = default;
	virtual const char *text() const = 0;
};

class English final : public Greeting {
public:
	const char *text() const override { return "Hello from gcc 16 / C++17"; }
};

class German final : public Greeting {
public:
	const char *text() const override { return "Hallo von gcc 16 / C++17"; }
};

// Global objects: their constructors run before PilotMain.
English english;
German german;
Greeting *const greetings[] = { &english, &german };   // pointers in .data

struct Counter {
	UInt16 value;
	Counter() : value(42) {}                              // dynamic initialization
	~Counter() { value = 0; }
};
Counter counter;

UInt16 zeroed;                                        // .bss
Int16 (*const squareFn)(Int16) = [](Int16 x) -> Int16 { return square(x); };

void drawLine(Coord y, const char *s)
{
	WinDrawChars(s, StrLen(s), 4, y);
}

void drawChecks()
{
	char buf[64];
	Coord y = 20;

	for (const Greeting *g : greetings) {
		drawLine(y, g->text());
		y += Config::lineHeight;
	}

	++Config::launches;
	StrPrintF(buf, "counter=%u (42) zeroed=%u (0)", counter.value, zeroed);
	drawLine(y, buf);
	y += Config::lineHeight;

	StrPrintF(buf, "square(7)=%d (49) shade=%d (2)", squareFn(7),
	          static_cast<Int16>(Shade::Black));
	drawLine(y, buf);
	y += Config::lineHeight;

	const char *ok = (counter.value == 42 && zeroed == 0 && squareFn(7) == 49
	                  && Config::launches == 1 && greetings[1] != nullptr)
	                 ? "All checks passed." : "CHECK FAILED";
	drawLine(y, ok);
}

Boolean mainFormHandleEvent(EventType *event)
{
	if (event->eType == frmOpenEvent) {
		FrmDrawForm(FrmGetActiveForm());
		drawChecks();
		return true;
	}
	return false;
}

Boolean appHandleEvent(EventType *event)
{
	if (event->eType == frmLoadEvent) {
		FormType *form = FrmInitForm(event->data.frmLoad.formID);
		FrmSetActiveForm(form);
		FrmSetEventHandler(form, mainFormHandleEvent);
		return true;
	}
	return false;
}

} // namespace

extern "C" UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
	if (cmd == sysAppLaunchCmdNormalLaunch) {
		EventType event;
		Err err;

		FrmGotoForm(1000);
		do {
			EvtGetEvent(&event, evtWaitForever);
			if (SysHandleEvent(&event))
				continue;
			if (MenuHandleEvent(nullptr, &event, &err))
				continue;
			if (appHandleEvent(&event))
				continue;
			FrmDispatchEvent(&event);
		} while (event.eType != appStopEvent);
		FrmCloseAllForms();
	}
	return errNone;
}
