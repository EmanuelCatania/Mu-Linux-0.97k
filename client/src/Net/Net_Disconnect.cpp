// Net_Disconnect.cpp
// Extracted from stubs.cpp; IDA provenance comments retained.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "Net/MuEmu.h"
#include "Net/Net.h"
#include "Render/Camera.h"

// ── Net_Disconnect @ 0x0043dc90 ──────────────────────────────────────────────
// Closes the game socket and resets connection state.
int __cdecl Net_Disconnect(int ctx) {
    (void)ctx;
    if (DAT_055ca168 != (DWORD)INVALID_SOCKET) {
        closesocket((SOCKET)DAT_055ca168);
        DAT_055ca168 = (DWORD)INVALID_SOCKET;
    }
    return 0;
}

