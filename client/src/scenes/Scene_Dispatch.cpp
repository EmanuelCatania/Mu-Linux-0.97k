// Scene_Dispatch.cpp
// Scene_Dispatch @ 0x005274A8
//
// Main per-frame dispatcher. Called from the PeekMessage loop in WinMain.
// Dispatches to the active scene via SceneFlag, then handles keepalive
// re-authentication and anti-tamper obfuscation (HashTable ref-count).
//
// Signature: void __cdecl Scene_Dispatch(HDC param_1)
//
// Globals:
//   SceneFlag  — SceneFlag (dispatch switch)
//   WorldTime  — frame time accumulator
//   DAT_083a45d4  — login background animation angle
//   g_iNoMouseTime  — keepalive counter (> 0x1f → send re-auth)
//   SocketClientSocket  — socket handle
//   SocketClientSendBuffer  — send buffer (WSAEWOULDBLOCK queue, max 0x2001)
//   SocketClientSendBufferLength  — send buffer byte count
//   DAT_055ca038  — post-send flag (cleared at end of function)
//   DAT_05826ceb  — packet sequence counter
//   DAT_00559050  — 16-byte XOR key for keepalive re-randomization

#include "stdafx.h"
#include "platform/ISocket.h"
#include "scenes/Scene_Dispatch.h"
#include "scenes/Scene_Intro.h"
#include "scenes/Scene_Loading.h"
#include "game/core/Game_MainLoop.h"
#include "net/Net.h"
#include <stdlib.h>   // for rand()
// (_rand ya está definido como `_rand() rand()` en stdafx.h)
// OpenGL_Release: declared static in WinMain.cpp; forward-declare here
extern void OpenGL_Release(void);

// Same 32-byte XOR key used in login packets

void __cdecl Scene_Dispatch(HDC param_1)
{
    // ── FRAME ANGLE ANIMATION ─────────────────────────────────────────────────
    // Original stored WorldTime * _DAT_005524bc into local_41c (unused after);
    // omitted here — same effect, no observable side effect.
    {
        // 0.97k @ 005274A8: the item-level glow pulse is driven directly from
        // WorldTime.  The former intermediary is an unported stub returning 0,
        // which froze DAT_083a45d4 and therefore all +3..+6 item glow colours.
        const float phase = (float)WorldTime * 0.0040000002f;
        DAT_083a45d4 = sinf(phase) * 0.15000001f + 0.60000002f;
    }

    // ── SCENE DISPATCH ────────────────────────────────────────────────────────
    switch (SceneFlag) {
    case 1: Scene_Intro(param_1);    break;
    case 2:
    case 4:
    case 5: Game_MainLoop(param_1);  break;
    case 3: Scene_Loading(param_1);  break;
    }

    // ── KEEPALIVE COUNTER: HASH TABLE REF-COUNT TRACKING ─────────────────────
    // (anti-tamper obfuscation — tracks g_iNoMouseTime via a reference-count table)
    {
        unsigned idx = HashTable_GetIndex(&MAIN_HASH_CLASS, &g_iNoMouseTime);
        if (idx == 0xffffffff) {
            void* node = AntiTamper_HashNode();
            *((BYTE*)node + 4) = 1;
            HashTable_Insert(&MAIN_HASH_CLASS, node, &g_iNoMouseTime);
        }
    }

    // ── RE-AUTH PACKET SEND (when keepalive counter > 0x1f) ──────────────────
    //
    // Desactivado para MuEmu: el 0.97 original manda un re-auth [C3][len][crc
    // payload] crudo por send() (sin el cifrado MuEmu) cuando g_iNoMouseTime
    // pasa de 0x1f (es un ping de GameGuard); MuEmu no lo espera, no lo puede
    // decodificar y cierra el socket. El bookkeeping de hash-table de abajo sigue
    // corriendo (ref-count anti-tamper, inofensivo).

    // DESVIACION: en escenas jugables Enter vive hasta el próximo paso SIM;
    // un render sin paso no debe consumir la confirmación de login/selección.
    if (SceneFlag!=2 && SceneFlag!=4 && SceneFlag!=5) DAT_055ca038 = 0;
}
