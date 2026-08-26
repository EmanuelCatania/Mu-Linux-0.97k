// MuEmu.h
// MuEmu-compat layer. Strictly isolated from the reversed vanilla-0.97k code.
//
// What this server actually does
// ------------------------------
// This Linux MuEmu port enables server-side "HackCheck" encryption
// (ENCRYPT_STATE=1 in GameServer/stdafx.h) which wraps EVERY outbound byte with:
//
//     enc[n] = (plain[n] + K) ^ K1   (mod 256, all 8-bit arithmetic)
//
// where K1 = EncDecKey1 and K = EncDecKey2 * EncDecKey1. The symmetric inverse is:
//
//     plain[n] = (enc[n] ^ K1) - K   (mod 256)
//
// The key is derived from gServerInfo.m_CustomerName XOR m_ServerSerial at
// server init time, so it's a per-install constant.  For this specific server
// (ServerSerial="TbYehR2hFUPBKgZj") we reverse-engineered the effective key
// empirically from the observed handshake:
//
//     Expected plaintext : C1 0C F1 00 01 23 XX 30 39 37 31 31   (C1:F1:00 connect-client)
//     Observed ciphertext: 41 0C 71 00 01 27 YY 30 39 3B 31 31
//
// The only key that makes every byte match under `(x^K)-K` is K1=K=0x42
// (equivalent class {0x42,0xC2} — both produce identical bytes under the
// symmetric transform).
//
// Architecture
// ------------
// Net_Recv (stubs.cpp FUN_0043de70) calls MuEmu::DecryptRecv on every fresh
// chunk returned by recv(), BEFORE any C1/C2/C3/C4 parsing. Likewise any
// outbound send site that talks to a MuEmu server should encrypt with
// MuEmu::EncryptSend just before calling ::send().
//
// The old "0x41 preamble intake" approach was WRONG — the 0x41 wasn't a
// special preamble, it was a regular C1 header with encryption applied.

#pragma once
#include <windows.h>

namespace MuEmu {

// -----------------------------------------------------------------------------
// Clave efectiva, derivada en runtime (2026-08-26)
// -----------------------------------------------------------------------------
// Antes estas dos constantes estaban HARDCODEADAS en 0x42/0x42, que es lo que
// da la formula del server para CustomerName="MuLinux" + ServerSerial=
// "TbYehR2hFUPBKgZj". Contra cualquier otro CustomerName el cliente conectaba
// pero desencriptaba basura y nunca reconocia el JoinServer (F1/00): se quedaba
// en "conectando al GameServer" para siempre.
//
// Ahora `InitKeys` reproduce la derivacion del server y `server.cfg` puede
// traer CustomerName/ServerSerial. Sin esas lineas quedan los valores de abajo,
// asi que los server.cfg viejos siguen funcionando igual.
//
// Equivalencia util al comparar contra el binario: sumar 0x80 mod 256 es lo
// mismo que XOR 0x80, asi que (K1,K)=(0x42,0x42) y (0xC2,0xC2) producen bytes
// identicos. La formula da 0xC2; el valor historico de este archivo era 0x42.
constexpr BYTE kEncKey1Default   = 0x42;   // EncDecKey1            (mascara XOR)
constexpr BYTE kEncKeyAddDefault = 0x42;   // EncDecKey2*EncDecKey1 (offset +/-)

// Clave en uso. La escribe InitKeys; arranca en los valores por defecto.
extern BYTE g_EncKey1;
extern BYTE g_EncKeyAdd;

// Deriva la clave con la MISMA formula que el server
// (GameServer/HackCheck.cpp::InitHackCheck, identica en el port Linux y en el
// original de Windows):
//
//     WORD k = 0;
//     for (n = 0; n < sizeof(CustomerName); n++) {          // 32 bytes
//         k += (BYTE)(CustomerName[n] ^ ServerSerial[n % sizeof(ServerSerial)]);
//         k ^= (BYTE)(CustomerName[n] - ServerSerial[n % sizeof(ServerSerial)]);
//     }
//     EncDecKey1 = 0xB0 + LOBYTE(k);
//     EncDecKey2 = 0xF8 + HIBYTE(k);
//
// Los buffers del server son CustomerName[32] y ServerSerial[17], y el loop
// recorre el ARRAY COMPLETO (relleno de ceros incluido), no strlen. `char` es
// con signo en las dos plataformas, y de eso depende el resultado del `-`.
//
// customerName o serverSerial en NULL/vacio => se usa el valor por defecto
// correspondiente ("MuLinux" / "TbYehR2hFUPBKgZj").
void InitKeys(const char* customerName, const char* serverSerial);

// Is the MuEmu encryption layer active for the current connection?  Defaults
// to true for this fork since we always talk to the Linux MuEmu port.  A real
// vanilla Webzen server would set this false.
bool IsActive();
void SetActive(bool on);

// Decrypt `len` bytes starting at `buf` in place.  Applies
//     plain[n] = (enc[n] ^ kEncKey1) - kEncKeyAdd
// Caller invokes this immediately after recv() completes.
void DecryptRecv(BYTE* buf, int len);

// Encrypt `len` bytes starting at `buf` in place.  Applies
//     enc[n] = (plain[n] + kEncKeyAdd) ^ kEncKey1
// Caller invokes this immediately before ::send().
void EncryptSend(BYTE* buf, int len);

} // namespace MuEmu

// =============================================================================
// GLOBAL send() AUTO-ENCRYPT HOOK
// =============================================================================
// Wraps WS2_32 send() so any caller that forgot to apply MuEmu::EncryptSend
// gets it applied automatically (only when sending to the game socket and the
// buffer starts with a plain MuEmu header byte 0xC1/0xC2/0xC3/0xC4).
// Implemented in MuEmu.cpp.  Stdafx.h #defines `send` to this hook so every
// translation unit that includes stdafx.h gets it for free.
// =============================================================================
extern "C" int __stdcall MuEmu_send_hook(SOCKET s, const char* buf, int len, int flags);
