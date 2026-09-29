/*
 * PACE wrapper macros for Palm OS ARM code
 *
 * ARM code cannot call Palm OS traps directly. Instead, it calls back
 * into the 68K emulator via Call68KFuncType.
 * These macros simplify the call.
 *
 * Based on TCPMP's pace.h (Greg Parker / Pieter van der Meulen).
 */

#ifndef PACE_H
#define PACE_H

#include <PceNativeCall.h>
#include "pealstub.h"

/*--- Byte-Swap Utilities (ARM is little-endian, Palm OS big-endian) ---*/

static inline UInt16 ByteSwap16(UInt16 v) {
    return (UInt16)((v >> 8) | (v << 8));
}

static inline UInt32 ByteSwap32(UInt32 v) {
    return ((v >> 24) & 0x000000FF) |
           ((v >>  8) & 0x0000FF00) |
           ((v <<  8) & 0x00FF0000) |
           ((v << 24) & 0xFF000000);
}

/*--- Call68K Helpers ---*/

/*
 * Calls a Palm OS trap. args points to the arguments in
 * big-endian format, size is the size in bytes.
 * Returns D0 (32-bit return value).
 */
static inline UInt32 PaceCall(UInt32 trapNo, const void *args, UInt32 size) {
    return PceCall.Func(PceCall.State, trapNo, args, size);
}

/*
 * Like PaceCall, but returns A0 (for functions that return pointers).
 */
static inline void *PaceCallPtr(UInt32 trapNo, const void *args, UInt32 size) {
    return (void *)PceCall.Func(PceCall.State, trapNo, args,
                                size | kPceNativeWantA0);
}

/*--- Commonly used Palm OS API wrappers ---*/

/* EvtGetEvent(&event, timeout) */
static inline void pace_EvtGetEvent(void *eventP, Int32 timeout) {
    UInt8 args[8];
    *(UInt32 *)&args[0] = ByteSwap32((UInt32)eventP);
    *(Int32  *)&args[4] = (Int32)ByteSwap32((UInt32)timeout);
    PaceCall(PceNativeTrapNo(sysTrapEvtGetEvent), args, 8);
}

/* SysHandleEvent(&event) -> Boolean */
static inline Boolean pace_SysHandleEvent(void *eventP) {
    UInt32 arg = ByteSwap32((UInt32)eventP);
    return (Boolean)PaceCall(PceNativeTrapNo(sysTrapSysHandleEvent), &arg, 4);
}

/* MenuHandleEvent(menuP, &event, &error) -> Boolean */
static inline Boolean pace_MenuHandleEvent(void *menuP, void *eventP, UInt16 *errP) {
    UInt8 args[12];
    *(UInt32 *)&args[0] = ByteSwap32((UInt32)menuP);
    *(UInt32 *)&args[4] = ByteSwap32((UInt32)eventP);
    *(UInt32 *)&args[8] = ByteSwap32((UInt32)errP);
    return (Boolean)PaceCall(PceNativeTrapNo(sysTrapMenuHandleEvent), args, 12);
}

/* FrmInitForm(formID) -> FormPtr */
static inline void *pace_FrmInitForm(UInt16 formID) {
    UInt16 arg = ByteSwap16(formID);
    return PaceCallPtr(PceNativeTrapNo(sysTrapFrmInitForm), &arg, 2);
}

/* FrmSetActiveForm(formP) */
static inline void pace_FrmSetActiveForm(void *formP) {
    UInt32 arg = ByteSwap32((UInt32)formP);
    PaceCall(PceNativeTrapNo(sysTrapFrmSetActiveForm), &arg, 4);
}

/* FrmGetActiveForm() -> FormPtr */
static inline void *pace_FrmGetActiveForm(void) {
    return PaceCallPtr(PceNativeTrapNo(sysTrapFrmGetActiveForm), NULL, 0);
}

/* FrmDrawForm(formP) */
static inline void pace_FrmDrawForm(void *formP) {
    UInt32 arg = ByteSwap32((UInt32)formP);
    PaceCall(PceNativeTrapNo(sysTrapFrmDrawForm), &arg, 4);
}

/* FrmDispatchEvent(&event) -> Boolean */
static inline Boolean pace_FrmDispatchEvent(void *eventP) {
    UInt32 arg = ByteSwap32((UInt32)eventP);
    return (Boolean)PaceCall(PceNativeTrapNo(sysTrapFrmDispatchEvent), &arg, 4);
}

/* FrmCloseAllForms() */
static inline void pace_FrmCloseAllForms(void) {
    PaceCall(PceNativeTrapNo(sysTrapFrmCloseAllForms), NULL, 0);
}

/* FrmGotoForm(formID) */
static inline void pace_FrmGotoForm(UInt16 formID) {
    UInt16 arg = ByteSwap16(formID);
    PaceCall(PceNativeTrapNo(sysTrapFrmGotoForm), &arg, 2);
}

/* FrmAlert(alertID) -> UInt16 */
static inline UInt16 pace_FrmAlert(UInt16 alertID) {
    UInt16 arg = ByteSwap16(alertID);
    return (UInt16)PaceCall(PceNativeTrapNo(sysTrapFrmAlert), &arg, 2);
}

/* FrmCustomAlert(alertID, s1, s2, s3) -> UInt16 */
static inline UInt16 pace_FrmCustomAlert(UInt16 alertID,
    const char *s1, const char *s2, const char *s3) {
    UInt8 args[14];
    *(UInt16 *)&args[0]  = ByteSwap16(alertID);
    *(UInt32 *)&args[2]  = ByteSwap32((UInt32)s1);
    *(UInt32 *)&args[6]  = ByteSwap32((UInt32)s2);
    *(UInt32 *)&args[10] = ByteSwap32((UInt32)s3);
    return (UInt16)PaceCall(PceNativeTrapNo(sysTrapFrmCustomAlert), args, 14);
}

/* WinDrawChars(chars, len, x, y) */
static inline void pace_WinDrawChars(const char *chars, Int16 len,
    Int16 x, Int16 y) {
    UInt8 args[10];
    *(UInt32 *)&args[0] = ByteSwap32((UInt32)chars);
    *(Int16  *)&args[4] = (Int16)ByteSwap16((UInt16)len);
    *(Int16  *)&args[6] = (Int16)ByteSwap16((UInt16)x);
    *(Int16  *)&args[8] = (Int16)ByteSwap16((UInt16)y);
    PaceCall(PceNativeTrapNo(sysTrapWinDrawChars), args, 10);
}

/* SndStreamCreate(&stream, dir, sampleRate, type, width, func, userData, bufSize, armNative)
 * Note: armNative=true allows direct ARM audio callbacks! */
static inline Err pace_SndStreamCreate(void *streamP, UInt8 dir,
    UInt32 sampleRate, UInt16 type, UInt16 width,
    void *funcP, void *userData, UInt32 bufSize, Boolean armNative) {
    UInt8 args[25];
    *(UInt32 *)&args[0]  = ByteSwap32((UInt32)streamP);
    *(UInt8  *)&args[4]  = dir;
    /* 1 byte padding */
    args[5] = 0;
    *(UInt32 *)&args[6]  = ByteSwap32(sampleRate);
    *(UInt16 *)&args[10] = ByteSwap16(type);
    *(UInt16 *)&args[12] = ByteSwap16(width);
    *(UInt32 *)&args[14] = ByteSwap32((UInt32)funcP);
    *(UInt32 *)&args[18] = ByteSwap32((UInt32)userData);
    *(UInt32 *)&args[22] = ByteSwap32(bufSize);
    /* armNative is the extra Palm OS 5 parameter */
    args[26] = armNative;
    return (Err)PaceCall(PceNativeTrapNo(sysTrapSndStreamCreate), args, 27);
}

/* SndStreamStart(stream) */
static inline Err pace_SndStreamStart(void *stream) {
    UInt32 arg = ByteSwap32((UInt32)stream);
    return (Err)PaceCall(PceNativeTrapNo(sysTrapSndStreamStart), &arg, 4);
}

/* SndStreamStop(stream) */
static inline Err pace_SndStreamStop(void *stream) {
    UInt32 arg = ByteSwap32((UInt32)stream);
    return (Err)PaceCall(PceNativeTrapNo(sysTrapSndStreamStop), &arg, 4);
}

/* SndStreamDelete(stream) */
static inline Err pace_SndStreamDelete(void *stream) {
    UInt32 arg = ByteSwap32((UInt32)stream);
    return (Err)PaceCall(PceNativeTrapNo(sysTrapSndStreamDelete), &arg, 4);
}

/* MemPtrNew(size) -> void* */
static inline void *pace_MemPtrNew(UInt32 size) {
    UInt32 arg = ByteSwap32(size);
    return PaceCallPtr(PceNativeTrapNo(sysTrapMemPtrNew), &arg, 4);
}

/* MemPtrFree(p) -> Err */
static inline Err pace_MemPtrFree(void *p) {
    UInt32 arg = ByteSwap32((UInt32)p);
    return (Err)PaceCall(PceNativeTrapNo(sysTrapMemChunkFree), &arg, 4);
}

/* TimGetTicks() -> UInt32 */
static inline UInt32 pace_TimGetTicks(void) {
    return PaceCall(PceNativeTrapNo(sysTrapTimGetTicks), NULL, 0);
}

#endif /* PACE_H */
