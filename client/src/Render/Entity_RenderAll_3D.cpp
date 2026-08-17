// Entity_RenderAll_3D.cpp
// Entity_RenderAll_3D @ 0x0045AB00  (51 lines, decompile completo)
//
// Itera el array global de entidades y para cada una activa y visible llama
// a Entity_UpdateRender. También aplica overrides de física al player local
// cuando está en juego (g_GameState == 5) y tiene flag de colisión activo.
//
// ── ARRAY DE ENTIDADES ────────────────────────────────────────────────────────
//
//   Base:   DAT_07abf5d0
//   Stride: 0x394 bytes por entidad
//   Límite: iVar3 < 0x59740  → ~409 entidades máximo
//   Local player: DAT_07abf5d8 (apunta a la primera entrada del array + stride del slot)
//
// ── DECOMPILE COMPLETO ────────────────────────────────────────────────────────
//
//   void FUN_0045ab00(void)
//   {
//     int iVar3 = 0;
//     DAT_07abf5d4 = 0;          // reset contador de entidades visibles
//     DAT_07abf5e8 = 0;          // reset flag extra
//     int iVar2 = 0;             // índice entidad
//     do {
//       char *pcVar1 = (char*)(DAT_07abf5d0 + iVar3);   // puntero entidad
//
//       if (pcVar1 == DAT_07abf5d8                       // es el player local
//        && (DAT_07abf5d8[0x1c0] & 4) != 0              // flag de colisión/física activo
//        && DAT_005615c0 == 5) {                         // g_GameState == InGame
//
//         // Override de física: fuerza velocidades a valores fijos
//         // Probablemente resetea velocidad al colisionar con terreno
//         pcVar1[0x130] = '\0'; pcVar1[0x131] = '\0';
//         pcVar1[0x132] = 'z';  pcVar1[0x133] = 'D';   // float: ~0x447a0000 ≈ 1000.0f (vel X)
//         pcVar1[0x15c] = '\0'; pcVar1[0x15d] = '\0';
//         pcVar1[0x15e] = -0x80; pcVar1[0x15f] = '?';  // float: ~0x3f800000 = 1.0f (vel Y?)
//         pcVar1[0x14c] = '\0'; pcVar1[0x14d] = '\0';
//         pcVar1[0x14e] = -0x80; pcVar1[0x14f] = '?';  // float: ~0x3f800000 = 1.0f
//         pcVar1[0x13c] = '\0'; pcVar1[0x13d] = '\0';
//         pcVar1[0x13e] = -0x80; pcVar1[0x13f] = '?';  // float: ~0x3f800000 = 1.0f
//       }
//       else if (pcVar1[0] != '\0'                      // entidad activa
//             && pcVar1[0x160] != '\0') {               // entidad visible (frustum)
//
//         DAT_07abf5d4 += 1;                            // contador visibles
//         // Determina si es el jugador local (slot especial)
//         uint is_local = (iVar2 == DAT_00559c50 || iVar2 == DAT_00559c4c) ? 1 : 0;
//         FUN_00456770(entity, entity, is_local);        // Entity_UpdateRender
//       }
//
//       iVar3 += 0x394;   // siguiente entidad
//       iVar2 += 1;
//     } while (iVar3 < 0x59740);
//   }
//
// ── CAMPOS DE ENTIDAD ─────────────────────────────────────────────────────────
//
//   entity[+0x00]   — byte: active flag
//   entity[+0x160]  — byte: frustum visibility (escrito por Terrain_Render)
//   entity[+0x1c0]  — uint: flags; bit 2 (0x4) = colisión/física activa
//   entity[+0x130]  — float: override velocidad A (=1000.0f cuando bit2 activo)
//   entity[+0x13c]  — float: override velocidad B (=1.0f)
//   entity[+0x14c]  — float: override velocidad C (=1.0f)
//   entity[+0x15c]  — float: override velocidad D (=1.0f)
//
// ── GLOBALS ───────────────────────────────────────────────────────────────────
//
//   DAT_07abf5d0  — base del array de entidades
//   DAT_07abf5d4  — contador de entidades visibles este frame (reset aquí)
//   DAT_07abf5d8  — puntero a la entidad del jugador local (dentro del array)
//   DAT_07abf5e8  — flag extra (reset a 0)
//   DAT_005615c0  — g_GameState (5 = InGame)
//   DAT_00559c50  — slot index A del jugador local (para identificación)
//   DAT_00559c4c  — slot index B del jugador local
//
// ── FUNCIÓN CROSS-REFERENCE ───────────────────────────────────────────────────
//
//   FUN_00456770  → Entity_UpdateRender(entity, entity, is_local_player)
//                   Actualiza el estado de renderizado de la entidad (animación, posición, etc.)

#include "stdafx.h"
#include "Render/Entity_RenderAll_3D.h"

extern "C" { void DbgLogPublic(const char*); }

// Entity_RenderAll_3D @ 0x0045AB00 (51 lines)
// Iterates entity array, resets the local player's velocity fields if in InGame,
// then calls Entity_UpdateRender for each active entity.
// Defined as FUN_0045ab00 to match callers (Scene_Login, Scene_CharSelect, etc.).
void FUN_0045ab00(void)
{
    char       *pcVar1;
    int         iVar2;   // entity slot index
    int         iVar3;   // byte offset
    undefined4 *puVar4;

    // ── DIAG: dump first 5 slots' render-relevant fields (once/sec, char-select only)
    static DWORD s_lastERA = 0;
    bool diag = false;
    if (DAT_005615c0 == 4) {
        DWORD now = GetTickCount();
        if (now - s_lastERA > 1000) { s_lastERA = now; diag = true; }
    }

    iVar3 = 0;
    DAT_07abf5d4 = 0;
    _DAT_07abf5e8 = 0;
    iVar2 = 0;

    do {
        pcVar1 = (char *)(DAT_07abf5d0 + iVar3);

        if (diag && iVar2 < 5) {
            char b[200];
            BYTE* e = (BYTE*)pcVar1;
            _snprintf_s(b, sizeof(b), _TRUNCATE,
                "ERA slot=%d act=%d vis=%d type@2=%d cls=%d isLocal=%d "
                "(slot==%d/559c50 || slot==%d/559c4c) hero@5d8=%p ptr=%p",
                iVar2, e[0], e[0x160], *(short*)(e+2), e[0x1bc],
                (iVar2 == DAT_00559c50) || (iVar2 == DAT_00559c4c),
                DAT_00559c50, DAT_00559c4c, DAT_07abf5d8, e);
            DbgLogPublic(b);
        }

        // BUG-FIX 2026-04-28: el IDA original tiene branches con conditions
        // que en nuestro build nunca matchean (flag bit 2, visibility flag).
        // Resultado: hero nunca renderiza. Detectamos hero ANTES que cualquier
        // otra cosa y forzamos el render.
        if (pcVar1 == DAT_07abf5d8 && DAT_005615c0 == 5 && *pcVar1 != '\0') {
            // Reset velocity / motion fields (per IDA original).
            pcVar1[0x130] = '\0'; pcVar1[0x131] = '\0';
            pcVar1[0x132] = 'z';  pcVar1[0x133] = 'D';
            pcVar1[0x15c] = '\0'; pcVar1[0x15d] = '\0';
            pcVar1[0x15e] = -0x80; pcVar1[0x15f] = '?';
            pcVar1[0x14c] = '\0'; pcVar1[0x14d] = '\0';
            pcVar1[0x14e] = -0x80; pcVar1[0x14f] = '?';
            pcVar1[0x13c] = '\0'; pcVar1[0x13d] = '\0';
            pcVar1[0x13e] = -0x80; pcVar1[0x13f] = '?';

            pcVar1[0x160] = 1;   // force visible flag
            // FIX 2026-07-24: el 3er param de RenderCharacter (FUN_00456770) es
            // el flag de HOVER/highlight (dibuja el borde de selección).  El IDA
            // pasa `(slot == SelectedCharacter || SelectedNpc)`, y el Hero está
            // EXCLUIDO de esos → el original lo dibuja con 0.  Este forced-render
            // hardcodeaba 1 → el PJ tenía el borde de hover pegado siempre.
            // Debe ser 0 (el Hero nunca es su propio target de hover).
            FUN_00456770((undefined4 *)pcVar1, (undefined4 *)pcVar1, (undefined4 *)0);
        } else if ((*pcVar1 != '\0') && (pcVar1[0x160] != '\0')) {
            DAT_07abf5d4 = DAT_07abf5d4 + 1;
            // is_local_player = (slot == DAT_00559c50 || slot == DAT_00559c4c)
            puVar4 = ((iVar2 == DAT_00559c50) || (iVar2 == DAT_00559c4c))
                     ? (undefined4 *)0x1 : (undefined4 *)0x0;
            if (diag && iVar2 < 5) {
                char b[120];
                _snprintf_s(b, sizeof(b), _TRUNCATE,
                    "ERA slot=%d -> FUN_00456770(local=%d)", iVar2, (int)(uintptr_t)puVar4);
                DbgLogPublic(b);
            }
            FUN_00456770((undefined4 *)pcVar1, (undefined4 *)pcVar1, puVar4);
        } else if (diag && iVar2 < 5) {
            char b[120];
            _snprintf_s(b, sizeof(b), _TRUNCATE,
                "ERA slot=%d SKIPPED (act=%d vis@160=%d hero==%p)",
                iVar2, *pcVar1, pcVar1[0x160], DAT_07abf5d8);
            DbgLogPublic(b);
        }

        iVar3 += 0x394;
        iVar2++;
    } while (iVar3 < 0x59740);   // 0x59740 = 400 entities × 0x394
}
