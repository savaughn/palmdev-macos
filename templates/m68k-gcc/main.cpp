/*
 * Palm OS 68K application template for the modern gcc (C++17).
 * Classic PilotMain() with event loop and form handling.
 */

#include <PalmOS.h>

namespace {

// Resource IDs (must match app.rcp)
constexpr UInt16 MainForm   = 1000;
constexpr UInt16 AboutAlert = 1100;
constexpr UInt16 MenuAbout  = 1001;

Boolean mainFormHandleEvent(EventType *event)
{
	switch (event->eType) {
	case frmOpenEvent:
		FrmDrawForm(FrmGetActiveForm());
		return true;

	case menuEvent:
		if (event->data.menu.itemID == MenuAbout) {
			FrmCustomAlert(AboutAlert, "Built with gcc " __VERSION__, "", "");
			return true;
		}
		break;

	default:
		break;
	}
	return false;
}

Boolean appHandleEvent(EventType *event)
{
	if (event->eType == frmLoadEvent) {
		FormType *form = FrmInitForm(event->data.frmLoad.formID);
		FrmSetActiveForm(form);
		if (event->data.frmLoad.formID == MainForm)
			FrmSetEventHandler(form, mainFormHandleEvent);
		return true;
	}
	return false;
}

void appEventLoop()
{
	EventType event;
	Err err;

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
}

} // namespace

// Global objects may have constructors and destructors: the runtime
// (m68k-gcc/runtime) runs them around every launch except sub-calls.

extern "C" UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
	if (cmd == sysAppLaunchCmdNormalLaunch) {
		FrmGotoForm(MainForm);
		appEventLoop();
		FrmCloseAllForms();
	}
	return errNone;
}
