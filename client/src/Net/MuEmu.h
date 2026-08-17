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

// Effective encryption key derived empirically from this server's handshake.
// Bytes are 8-bit modular. K1 and K happen to be equal for this server since
// EncDecKey2 ≡ 1 (mod 256 / EncDecKey1).
//   EncDecKey1 = 0x42  (XOR mask)
//   K          = 0x42  (add/sub offset)
constexpr BYTE kEncKey1 = 0x42;
constexpr BYTE kEncKeyAdd = 0x42;

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
