// Entity_RenderAll_3D.cpp
// Entity_RenderAll_3D @ 0x0045AB00  (51 lines, decompile completo)
//
// Itera el array global de entidades y para cada una activa y visible llama
// a Entity_UpdateRender. También aplica overrides de física al player local
// cuando está en juego (SceneFlag == 5) y tiene flag de colisión activo.
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
//   void Entity_RenderAll_3D(void)
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
//        && SceneFlag == 5) {                         // SceneFlag == InGame
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
//         uint is_local = (iVar2 == SelectedCharacter || iVar2 == SelectedNpc) ? 1 : 0;
//         RenderCharacter(entity, entity, is_local);        // Entity_UpdateRender
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
//   SceneFlag  — SceneFlag (5 = InGame)
//   SelectedCharacter  — slot index A del jugador local (para identificación)
//   SelectedNpc  — slot index B del jugador local
//
// ── FUNCIÓN CROSS-REFERENCE ───────────────────────────────────────────────────
//
//   RenderCharacter  → Entity_UpdateRender(entity, entity, is_local_player)
//                   Actualiza el estado de renderizado de la entidad (animación, posición, etc.)

#include "stdafx.h"
#include "Render/Entity_RenderAll_3D.h"


// IDA: Entity_RenderAll_3D (0x0045AB00)
// Iterates entity array, resets the local player's velocity fields if in InGame,
// then calls Entity_UpdateRender for each active entity.
// Defined as Entity_RenderAll_3D to match callers (Scene_Login, Scene_CharSelect, etc.).
void Entity_RenderAll_3D(void)
{
    char       *pcVar1;
    int         iVar2;   // entity slot index
    int         iVar3;   // byte offset
    undefined4 *puVar4;


    iVar3 = 0;
    DAT_07abf5d4 = 0;
    _DAT_07abf5e8 = 0;
    iVar2 = 0;

    do {
        pcVar1 = (char *)(DAT_07abf5d0 + iVar3);


        // DESVIACION: las branches del IDA original tienen condiciones que en nuestro
        // build nunca matchean (flag bit 2, visibility flag), así que el héroe se
        // detecta ANTES que cualquier otra cosa y se fuerza su render.
        if (pcVar1 == DAT_07abf5d8 && SceneFlag == 5 && *pcVar1 != '\0') {
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
            // El 3er param de RenderCharacter es el flag de HOVER/highlight (dibuja el
            // borde de selección). El IDA pasa `(slot == SelectedCharacter || SelectedNpc)`
            // y el Hero está EXCLUIDO de esos: va 0.
            RenderCharacter((undefined4 *)pcVar1, (undefined4 *)pcVar1, (undefined4 *)0);
        } else if ((*pcVar1 != '\0') && (pcVar1[0x160] != '\0')) {
            DAT_07abf5d4 = DAT_07abf5d4 + 1;
            // is_local_player = (slot == SelectedCharacter || slot == SelectedNpc)
            puVar4 = ((iVar2 == SelectedCharacter) || (iVar2 == SelectedNpc))
                     ? (undefined4 *)0x1 : (undefined4 *)0x0;
            RenderCharacter((undefined4 *)pcVar1, (undefined4 *)pcVar1, puVar4);
        }

        iVar3 += 0x394;
        iVar2++;
    } while (iVar3 < 0x59740);   // 0x59740 = 400 entities × 0x394
}
