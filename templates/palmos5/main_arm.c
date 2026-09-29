/*
 * Palm OS 5 - ARM main application
 * Compiled with arm-palmos-gcc.
 *
 * All Palm OS calls must go through PACE wrappers,
 * since ARM code cannot call the 68K traps directly.
 */

#include <PalmOS.h>
#include <PceNativeCall.h>
#include "pealstub.h"
#include "pace.h"

/* Resource IDs (must match app.rcp) */
#define MainForm       1000
#define AboutAlert     1100
#define MenuAbout      1001

/*--- Event Handling ---*/

static Boolean MainFormHandleEvent(void *eventP)
{
    UInt16 eType;
    void *form;

    /* Event type is a big-endian UInt16 at the start of the EventType struct */
    eType = ByteSwap16(*(UInt16 *)eventP);

    switch (eType) {
    case 0x1601: /* frmOpenEvent */
        form = pace_FrmGetActiveForm();
        pace_FrmDrawForm(form);
        return 1;

    default:
        break;
    }

    return 0;
}

static void AppEventLoop(void)
{
    UInt8 event[64]; /* EventType buffer */
    UInt16 eType;

    do {
        pace_EvtGetEvent(event, (Int32)0x7FFFFFFF); /* evtWaitForever */

        eType = ByteSwap16(*(UInt16 *)event);

        if (!pace_SysHandleEvent(event)) {
            UInt16 err = 0;
            if (!pace_MenuHandleEvent(0, event, &err)) {
                if (eType == 0x1600) { /* frmLoadEvent */
                    UInt16 formID = ByteSwap16(*(UInt16 *)((UInt8 *)event + 8));
                    void *form = pace_FrmInitForm(formID);
                    pace_FrmSetActiveForm(form);
                    /* Note: FrmSetEventHandler cannot simply be set to
                       ARM functions. Instead, dispatch events
                       directly here. */
                }
                if (eType == 0x1601) { /* frmOpenEvent */
                    MainFormHandleEvent(event);
                } else {
                    pace_FrmDispatchEvent(event);
                }
            }
        }

    } while (eType != 0x16FF); /* appStopEvent */
}

/*--- ARM Entry Point (called by the 68K stub via PEAL) ---*/

unsigned long AppMain(void *cmdPBP)
{
    pace_FrmGotoForm(MainForm);
    AppEventLoop();
    pace_FrmCloseAllForms();

    return 0;
}
