// MuEmu.cpp
// MuEmu-compat layer — byte-level stream encryption that mirrors the Linux
// port's GameServer/HackCheck.cpp.  See MuEmu.h for the key-derivation story.

#include "stdafx.h"
#include "Net/MuEmu.h"
#include <winsock2.h>

// Disable the global send() redirect macro inside this TU so MuEmu_send_hook
// can call the real WS2_32 send().
#ifdef send
#undef send
#endif

extern "C" void DbgLogPublic(const char* msg);

namespace MuEmu {

static bool g_active = true;

bool IsActive() { return g_active; }
void SetActive(bool on) { g_active = on; }

// -----------------------------------------------------------------------------
// Hex/ASCII dump helper for diagnostics (first/last N bytes of a buffer).
//
// 2026-04-29: gated behind MUEMU_TRACE.  When debugging under VS the high
// volume of hex bytes flowing through DbgLog → WriteFile → debug.log was
// triggering first-chance KernelBase AVs (ImePadServer-style) and flooding
// the log file (300 KB+ of recv/send dumps per minute).  Define
// MUEMU_TRACE in the project for opt-in tracing during net-protocol work.
// -----------------------------------------------------------------------------
static void DumpHex(const char* tag, const BYTE* buf, int len)
{
#ifdef MUEMU_TRACE
    // Log full packet up to 256 bytes (fits any login/char-list payload).
    char hex[1024]; hex[0] = 0;
    int n = len > 256 ? 256 : len;
    for (int i = 0; i < n; ++i) {
        char t[8]; wsprintfA(t, "%02X ", buf[i]);
        lstrcatA(hex, t);
    }
    char line[1280];
    wsprintfA(line, "MuEmu: %s len=%d [%s%s]",
              tag, len, hex, (len > 256) ? "..." : "");
    DbgLogPublic(line);
#else
    (void)tag; (void)buf; (void)len;
#endif
}

// -----------------------------------------------------------------------------
// DecryptRecv — in-place decrypt of a freshly-received byte chunk.
//
// Mirrors GameServer/HackCheck.cpp::DecryptData:
//     lpMsg[n] = (lpMsg[n] ^ EncDecKey1) - (EncDecKey2 * EncDecKey1);
// with (EncDecKey1, EncDecKey2*EncDecKey1) = (kEncKey1, kEncKeyAdd) = (0x42, 0x42).
// -----------------------------------------------------------------------------
void DecryptRecv(BYTE* buf, int len)
{
    if (!g_active || buf == nullptr || len <= 0) return;

    DumpHex("recv cipher", buf, len);
    for (int i = 0; i < len; ++i) {
        buf[i] = (BYTE)((buf[i] ^ kEncKey1) - kEncKeyAdd);
    }
    DumpHex("recv plain ", buf, len);
}

// -----------------------------------------------------------------------------
// EncryptSend — in-place encrypt of a buffer about to be sent.
//
// Mirrors GameServer/HackCheck.cpp::EncryptData:
//     lpMsg[n] = (lpMsg[n] + (EncDecKey2 * EncDecKey1)) ^ EncDecKey1;
// -----------------------------------------------------------------------------
void EncryptSend(BYTE* buf, int len)
{
    if (!g_active || buf == nullptr || len <= 0) return;

    DumpHex("send plain ", buf, len);
    for (int i = 0; i < len; ++i) {
        buf[i] = (BYTE)((buf[i] + kEncKeyAdd) ^ kEncKey1);
    }
    DumpHex("send cipher", buf, len);
}

} // namespace MuEmu

// =============================================================================
// GLOBAL `send()` HOOK
// =============================================================================
// Wraps WS2_32 send() for the game socket: if the buffer's first byte looks
// like a PLAINTEXT MuEmu packet (C1/C2/C3/C4) instead of an already-encrypted
// byte (whose set is much narrower for these headers), apply EncryptSend
// transparently. Goal: catch any latent send path that bypasses our explicit
// MuEmu::EncryptSend.
//
// Encrypted-byte sentinel: `C1` ENC = 0x41, `C2` ENC = 0x42, `C3` ENC = 0x47,
// `C4` ENC = 0x44. Plain headers C1/C2/C3/C4 = 0xC1/0xC2/0xC3/0xC4.
//
// Important: HackCheck.cpp::MySend in the reference Main.dll transforms the
// caller's buffer *in place*, then passes that same buffer to Winsock.  Do the
// same here.  Encrypting a temporary copy is observably different when send()
// completes only part of a packet (or returns WSAEWOULDBLOCK): the next raw
// send/queued bytes would otherwise be the original plaintext tail.
//
// Some reconstructed C3 paths already call EncryptSend explicitly.  Their
// first byte is then the encrypted sentinel (41/42/47/44), so the plaintext
// header test below still guarantees exactly one MuEmu transform.
// =============================================================================
// DAT_055ca168 already declared in globals.h with C++ linkage.
// MuEmu_send_hook callable as extern "C" symbol; body is not in the C block
// to avoid name-mangling vs linkage conflicts.
extern "C" int __stdcall MuEmu_send_hook(SOCKET s, const char* buf, int len, int flags)
{
    if (s == (SOCKET)DAT_055ca168 && MuEmu::IsActive() && len > 0 && buf) {
        BYTE first = (BYTE)buf[0];
        // Plain MuEmu header? → not yet encrypted.
        if (first == 0xC1 || first == 0xC2 || first == 0xC3 || first == 0xC4) {
            {
                // DIAG 2026-07-19: volcar los bytes PLANOS (pre-encrypt) para
                // identificar qué paquete manda el cliente. Se está buscando un
                // C1 de 4 bytes que precede a un FD_CLOSE del server.
                char dbg[256];
                int p = wsprintfA(dbg, "MuEmu_send_hook: AUTO-ENCRYPT C%X len=%d plain=[",
                                  (first & 0xF), len);
                int dumpN = len > 24 ? 24 : len;
                for (int i = 0; i < dumpN && p < 220; ++i)
                    p += wsprintfA(dbg + p, "%02X ", (BYTE)buf[i]);
                wsprintfA(dbg + p, "]");
                DbgLogPublic(dbg);
            }
            MuEmu::EncryptSend((BYTE*)(uintptr_t)buf, len);
        }
    }
    return ::send(s, buf, len, flags);
}
