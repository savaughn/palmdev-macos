/*
 * Palm OS 4 Application Template
 * Classic 68K app with PilotMain(), event loop and form handling.
 */

#include <PalmOS.h>

/* Resource IDs (must match app.rcp) */
#define MainForm       1000
#define AboutAlert     1100
#define MenuAbout      1001

/* Forward declarations */
static Err    AppStart(void);
static void   AppStop(void);
static void   AppEventLoop(void);
static Boolean AppHandleEvent(EventPtr event);
static Boolean MainFormHandleEvent(EventPtr event);

/*--- Application Lifecycle ---*/

static Err AppStart(void)
{
    /* Open databases, load preferences etc. */
    return errNone;
}

static void AppStop(void)
{
    /* Close databases, free resources */
    FrmCloseAllForms();
}

/*--- Event Handling ---*/

static Boolean MainFormHandleEvent(EventPtr event)
{
    Boolean handled = false;
    FormPtr form;

    switch (event->eType) {
    case frmOpenEvent:
        form = FrmGetActiveForm();
        FrmDrawForm(form);
        handled = true;
        break;

    case menuEvent:
        if (event->data.menu.itemID == MenuAbout) {
            FrmAlert(AboutAlert);
            handled = true;
        }
        break;

    default:
        break;
    }

    return handled;
}

static Boolean AppHandleEvent(EventPtr event)
{
    UInt16 formID;
    FormPtr form;

    if (event->eType == frmLoadEvent) {
        formID = event->data.frmLoad.formID;
        form = FrmInitForm(formID);
        FrmSetActiveForm(form);

        switch (formID) {
        case MainForm:
            FrmSetEventHandler(form, MainFormHandleEvent);
            break;
        }
        return true;
    }

    return false;
}

static void AppEventLoop(void)
{
    EventType event;
    UInt16 error;

    do {
        EvtGetEvent(&event, evtWaitForever);

        if (!SysHandleEvent(&event))
            if (!MenuHandleEvent(0, &event, &error))
                if (!AppHandleEvent(&event))
                    FrmDispatchEvent(&event);

    } while (event.eType != appStopEvent);
}

/*--- Entry Point ---*/

UInt32 PilotMain(UInt16 cmd, void *cmdPBP, UInt16 launchFlags)
{
    Err error;

    if (cmd == sysAppLaunchCmdNormalLaunch) {
        error = AppStart();
        if (error) return error;

        FrmGotoForm(MainForm);
        AppEventLoop();
        AppStop();
    }

    return errNone;
}
