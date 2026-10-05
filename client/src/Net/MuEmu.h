// MuEmu.h
// Capa de compatibilidad con MuEmu, aislada del código revertido del 0.97k.
//
// Qué hace el server
// ------------------
// El server MuEmu activa el cifrado "HackCheck" (ENCRYPT_STATE=1 en
// GameServer/stdafx.h), que envuelve TODO byte saliente con:
//
//     enc[n] = (plain[n] + K) ^ K1   (mod 256, all 8-bit arithmetic)
//
// where K1 = EncDecKey1 and K = EncDecKey2 * EncDecKey1. The symmetric inverse is:
//
//     plain[n] = (enc[n] ^ K1) - K   (mod 256)
//
// La clave sale de CustomerName y ServerSerial del server (ver InitKeys abajo).
//
// Architecture
// ------------
// CWsctlc_nRecv (Scene/Scene_CharSelect_Nav.cpp) llama a MuEmu::DecryptRecv
// sobre cada chunk recién devuelto por recv(), ANTES de parsear C1/C2/C3/C4.
// Del lado de envío, los sitios llaman a MuEmu::EncryptSend antes de ::send(),
// y el hook de send() de MuEmu.cpp cifra cualquier buffer que todavía empiece
// con un header plano.
//
// Ojo: el 0x41 del principio del stream no es un preámbulo especial: es un
// header C1 normal ya cifrado.

#pragma once
#include <windows.h>

namespace MuEmu {

// -----------------------------------------------------------------------------
// Clave efectiva, derivada en runtime
// -----------------------------------------------------------------------------
// `InitKeys` reproduce la derivacion del server y Config/ServerConfig.h trae
// CustomerName/ServerSerial. Hasta que corre InitKeys quedan los valores de abajo, que
// son los que da la formula del server para CustomerName="MuLinux" +
// ServerSerial="TbYehR2hFUPBKgZj".
//
// Equivalencia util al comparar contra el binario: sumar 0x80 mod 256 es lo
// mismo que XOR 0x80, asi que (K1,K)=(0x42,0x42) y (0xC2,0xC2) producen bytes
// identicos. La formula da 0xC2; aca se usa 0x42.
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
