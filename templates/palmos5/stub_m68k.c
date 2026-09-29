/*
 * Palm OS 5 - 68K Stub
 * Loads the ARM code via PEAL and calls the ARM main function.
 * Compiled with m68k-palmos-gcc.
 */

#include <PalmOS.h>
#include "peal.h"

UInt32 PilotMain(UInt16 cmd, void *cmdPBP, UInt16 launchFlags)
{
    PealModule *m;
    void *entry;
    UInt32 result;
    UInt32 cpuType;

    if (cmd != sysAppLaunchCmdNormalLaunch)
        return errNone;

    /* Check whether an ARM CPU is present */
    FtrGet(sysFileCSystem, sysFtrNumProcessorID, &cpuType);
    if (cpuType == sysFtrNumProcessorx86) {
        FrmCustomAlert(1100, "Simulator is not supported.", "", "");
        return errNone;
    }
    if (!sysFtrNumProcessorIsARM(cpuType)) {
        FrmCustomAlert(1100, "ARM processor required.", "", "");
        return errNone;
    }

    /* Load ARM code from 'armc' resources (ID 1000+) */
    m = PealLoadFromResources('armc', 1000, NULL, 0, 0, 0, 0, 0);
    if (!m) {
        FrmCustomAlert(1100, "ARM code could not be loaded.", "", "");
        return errNone;
    }

    /* Look up ARM entry point */
    entry = PealLookupSymbol(m, "AppMain");
    if (!entry) {
        PealUnload(m);
        FrmCustomAlert(1100, "AppMain not found.", "", "");
        return errNone;
    }

    /* Execute ARM code */
    result = PealCall(m, entry, cmdPBP);

    PealUnload(m);
    return result;
}
