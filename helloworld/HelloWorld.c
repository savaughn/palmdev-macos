// Source: https://tldp.org/REF/palmdevqs/index.html

#include <PalmOS.h>

// ---------------------------------------------------------------------
// PilotMain is called by the startup code and implements a simple event
// handling loop.
// ---------------------------------------------------------------------
UInt32 PilotMain( UInt16 cmd, void *cmdPBP, UInt16 launchFlags )
{
    EventType event;


    if (cmd == sysAppLaunchCmdNormalLaunch) {

        //  Display centered text.
        {
            const char *line1 = "This app was built with";
            const char *line2 = "gcc 2.95.3 on macOS";
            const char *line3 = "(Apple Silicon)!";
            Int16 w = 160;
            Int16 y = 55;

            WinDrawChars(line1, StrLen(line1),
                (w - FntCharsWidth(line1, StrLen(line1))) / 2, y);
            WinDrawChars(line2, StrLen(line2),
                (w - FntCharsWidth(line2, StrLen(line2))) / 2, y + 15);
            WinDrawChars(line3, StrLen(line3),
                (w - FntCharsWidth(line3, StrLen(line3))) / 2, y + 30);
        }

        //  Main event loop:
        do {
            //  Doze until an event arrives.
            EvtGetEvent( &event, evtWaitForever );

            //  System gets first chance to handle the event.
            SysHandleEvent( &event );

            //  Normally, we would do other event processing here.

        // Return from PilotMain when an appStopEvent is received.
        } while (event.eType != appStopEvent);
    }
    return 0;
}

