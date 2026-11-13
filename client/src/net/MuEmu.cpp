// MuEmu.cpp — ver MuEmu.h.

#include "stdafx.h"
#include "platform/ISocket.h"
#include "net/MuEmu.h"
#include "net/NetChannel.h"

// Acá se usa el send real, no el macro que apunta a este mismo enganche.
#ifdef send
#undef send
#endif

extern "C" int __stdcall MuEmu_send_hook(SOCKET s, const char* buf, int len, int flags)
{
    if (s == (SOCKET)SocketClientSocket && len > 0 && buf) {
        // Lo que sale de la cola de envío (CWsctlc_FDWriteSend) ya pasó por el
        // canal: va al socket tal cual.
        if (NetChannel::IsQueued(buf)) return Platform::GetSocket().Send(s, buf, len, flags);
        return NetChannel::Send(buf, len);
    }
    return Platform::GetSocket().Send(s, buf, len, flags);
}
