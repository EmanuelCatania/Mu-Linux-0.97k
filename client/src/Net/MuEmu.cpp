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

// Clave en uso (ver MuEmu.h). Arranca en los valores historicos, que son los
// que corresponden a CustomerName="MuLinux".
BYTE g_EncKey1   = kEncKey1Default;
BYTE g_EncKeyAdd = kEncKeyAddDefault;

// Valores por defecto de este fork, usados cuando server.cfg no los trae.
static const char kDefaultCustomerName[] = "MuLinux";
static const char kDefaultServerSerial[] = "TbYehR2hFUPBKgZj";

// -----------------------------------------------------------------------------
// InitKeys — port fiel de GameServer/HackCheck.cpp::InitHackCheck.
//
// Los tamanos de buffer son los del server (CustomerName[32], ServerSerial[17])
// y el loop recorre el array entero, no strlen: el relleno de ceros participa
// del calculo. Los dos campos se copian igual que lo hace
// GetPrivateProfileString (trunca al tamano del buffer y garantiza el NUL).
//
// `char` con signo a proposito: el termino `CustomerName[n] - ServerSerial[..]`
// da otro resultado con unsigned char, y el server usa `char`.
// -----------------------------------------------------------------------------
void InitKeys(const char* customerName, const char* serverSerial)
{
    if (customerName == nullptr || customerName[0] == 0) customerName = kDefaultCustomerName;
    if (serverSerial == nullptr || serverSerial[0] == 0) serverSerial = kDefaultServerSerial;

    char cust[32] = { 0 };   // CServerInfo::m_CustomerName[32]
    char ser[17]  = { 0 };   // CServerInfo::m_ServerSerial[17]
    lstrcpynA(cust, customerName, sizeof(cust));
    lstrcpynA(ser,  serverSerial, sizeof(ser));

    WORD encDecKey = 0;
    for (int n = 0; n < (int)sizeof(cust); ++n) {
        const char c = cust[n];
        const char t = ser[n % (int)sizeof(ser)];
        encDecKey = (WORD)(encDecKey + (BYTE)(c ^ t));
        encDecKey = (WORD)(encDecKey ^ (BYTE)(c - t));
    }

    const BYTE encDecKey1 = (BYTE)(0xB0 + LOBYTE(encDecKey));
    const BYTE encDecKey2 = (BYTE)(0xF8 + HIBYTE(encDecKey));

    g_EncKey1   = encDecKey1;
    g_EncKeyAdd = (BYTE)(encDecKey2 * encDecKey1);   // el server usa el producto

    {
        char line[192];
        wsprintfA(line,
                  "MuEmu: InitKeys CustomerName='%s' Serial='%s' -> "
                  "EncDecKey1=0x%02X EncDecKey2=0x%02X (xor=0x%02X add=0x%02X)",
                  cust, ser, encDecKey1, encDecKey2, g_EncKey1, g_EncKeyAdd);
        DbgLogPublic(line);
    }
}

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
// con (EncDecKey1, EncDecKey2*EncDecKey1) = (g_EncKey1, g_EncKeyAdd), que
// deriva InitKeys de CustomerName+ServerSerial (ver MuEmu.h).
// -----------------------------------------------------------------------------
void DecryptRecv(BYTE* buf, int len)
{
    if (!g_active || buf == nullptr || len <= 0) return;

    DumpHex("recv cipher", buf, len);
    for (int i = 0; i < len; ++i) {
        buf[i] = (BYTE)((buf[i] ^ g_EncKey1) - g_EncKeyAdd);
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
        buf[i] = (BYTE)((buf[i] + g_EncKeyAdd) ^ g_EncKey1);
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
