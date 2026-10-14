// Net_Disconnect.cpp

#include "stdafx.h"
#include "Platform/ISocket.h"
#include "globals.h"
#include "functions.h"
#include "Net/MuEmu.h"
#include "Net/Net.h"
#include "Render/Camera.h"

// ── Net_Disconnect @ 0x0043dc90 ──────────────────────────────────────────────
// Closes the game socket and resets connection state.
int __cdecl Net_Disconnect(int ctx) {
    (void)ctx;
    if (SocketClientSocket != (DWORD)INVALID_SOCKET) {
        Platform::GetSocket().Close((SOCKET)SocketClientSocket);
        SocketClientSocket = (DWORD)INVALID_SOCKET;
    }
    return 0;
}

