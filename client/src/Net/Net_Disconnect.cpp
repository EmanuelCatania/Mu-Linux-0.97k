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
int __cdecl Net_Disconnect(void* ctx) {
    (void)ctx;
    if (Platform::GetSocket().IsOpen(SocketClientSocket)) {
        Platform::GetSocket().Close((SOCKET)SocketClientSocket);
        SocketClientSocket = static_cast<DWORD>(Platform::InvalidSocket);
    }
    return 0;
}

