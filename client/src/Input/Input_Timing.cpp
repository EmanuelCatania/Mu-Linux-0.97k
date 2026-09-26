// Input_Timing.cpp
// Extracted from stubs_mouse_hover.cpp; IDA provenance comments retained.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// IDA: RenderDebugWindow @ 0x004C14E0 — FPS_TickReset.
// Selects the UI font into the DC, resets text color to white (0xFFFFFFFF)
// and background to black (0xFF000000). On each 1-second interval resets
// the FPS counter (DAT_07E11DCC = 0) and updates the timestamp.
// IDA: RenderDebugWindow
void __cdecl UI_UpdateFpsCounter(void) {
    SelectObject(DAT_055c9fec, (HGDIOBJ)(uintptr_t)DAT_055ca00c);
    DAT_00559c78 = 0xffffffff;
    SetBackgroundTextColor = 0xff000000;
    DWORD DVar1 = timeGetTime();
    if (999 < DVar1 - (DWORD)_DAT_07e016f0) {
        _DAT_07e016f0 = timeGetTime();
        TotalPacketSize = 0;
    }
}
