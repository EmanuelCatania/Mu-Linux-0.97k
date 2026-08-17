// Player_Render.cpp
// Player_Render @ 0x00475110  (18 lines, decompile completo)
//
// Renderiza todos los jugadores/entidades del pool de jugadores como sprites.
// Función muy compacta: solo itera el pool, setea el color, y llama al draw.
//
// ── POOL DE JUGADORES ─────────────────────────────────────────────────────────
//
//   Base iterador: DAT_07c74f54 (puVar1 = &DAT_07c74f54)
//   Stride:        0x6f * 4 = 0x1bc bytes por entrada
//   Límite:        puVar1 < 0x7c7fcc4
//   Entradas:      (0x7c7fcc4 - 0x07c74f54) / 0x1bc = ~100 jugadores
//
//   Struct de jugador (relativo a puVar1):
//     puVar1[-0x3b] — char: active flag (0 = skip)
//     puVar1[-0x38] — float: rotation angle
//     puVar1[-0x37] — float[4+]: posición XYZ + extras (ptr usado como array)
//     puVar1[-0x32] — float: escala
//     puVar1[-1]    — float: color R
//     puVar1[+0]    — float: color G
//     puVar1[+1]    — float: color B
//     *(short*)((int)puVar1 - 0xea) — short: tipo de jugador / class code
//
// ── DECOMPILE COMPLETO ────────────────────────────────────────────────────────
//
//   void FUN_00475110(void)
//   {
//     puVar1 = &DAT_07c74f54;
//     do {
//       if (*(char*)(puVar1 - 0x3b) != '\0') {   // jugador activo
//         glColor3f(puVar1[-1], *puVar1, puVar1[1]);  // setear color RGB
//         FUN_00474f90(
//           (int)*(short*)((int)puVar1 - 0xea),  // class/type code
//           puVar1 - 0x37,                        // posición XYZ (array)
//           (float)puVar1[-0x38],                 // rotation angle
//           puVar1[-0x32]                         // escala
//         );  → Player_DrawSprite(class, pos[3], rotation, scale)
//       }
//       puVar1 += 0x6f;   // siguiente jugador (0x1bc bytes)
//     } while ((int)puVar1 < 0x7c7fcc4);
//   }
//
// ── NOTA ARQUITECTURAL ────────────────────────────────────────────────────────
//
//   A diferencia de Entity_Render_Sprites que usa el array general de entidades
//   (DAT_07e12945, stride 0x204), este pool es exclusivo para jugadores y tiene
//   stride diferente (0x1bc). Probablemente es el pool de otros jugadores en el
//   área visible, con datos simplificados (solo pos/rot/scale/color/type).
//
//   El color RGB por jugador posiblemente codifica:
//     - Estado del personaje (PK = rojo, normal = blanco/amarillo)
//     - Clase o nivel del jugador
//     - Efecto de habilidad activo
//
// ── FUNCIÓN CROSS-REFERENCE ───────────────────────────────────────────────────
//
//   FUN_00474f90  → Player_DrawSprite(class_code, pos[3], rotation, scale)
//                   Función que hace el draw real del sprite del jugador.
//                   Probablemente llama a Sprite_Draw o Entity_DrawAt con los parámetros.
//   glColor3f     → OpenGL color sin alpha (afecta al sprite del jugador)

#include "stdafx.h"
#include "Render/Player_Render.h"

// Player_Render @ 0x00475110 (18 lines)
// Iterates player entity table (base DAT_07c74f54 − 0xbc, stride 0x6f×4),
// renders each active player sprite with its stored color + transform.
void Player_Render(void)
{
    // 2026-05-07: UN-SKIPPED. El pool ahora se aloca propiamente como
    // g_PlayerRenderPool[100 × 0x1BC] (per IDA bound 0xAD70 = 100 × 444), con
    // DAT_07c74f54 apuntando a g_PlayerRenderPool + 0xEC (= v1 anchor del
    // slot 0). Los reads negativos (puVar1 - 0x3b = -0xEC bytes) ahora caen
    // dentro del buffer.
    //
    // Walker iter: v1 starts at DAT_07c74f54 (= slot 0 + 0xEC), advances 0x6F
    // floats (= 0x1BC bytes) per iter. Total 100 slots.
    //
    // Slot fields (relative to v1 = slot+0xEC):
    //   v1 - 0x3b (= slot+0)    byte: active flag
    //   v1 - 0x38 (= slot+0xC)  float: rotation
    //   v1 - 0x37 (= slot+0x10) float[3]: pos xyz
    //   v1 - 0x32 (= slot+0x24) float: scale
    //   v1 - 1, v1, v1 + 1      float: color RGB
    //   v1 - 0xea (= slot - 0xC2 from v1) short: class_code
    DWORD* puVar1 = DAT_07c74f54;
    for (int i = 0; i < 100; ++i) {
        if (*(char*)(puVar1 - 0x3b) != '\0') {                   // active flag
            float r = *(float*)(puVar1 - 1);
            float g = *(float*)puVar1;
            float b = *(float*)(puVar1 + 1);
            glColor3f(r, g, b);
            short classCode = *(short*)((int)puVar1 - 0xEA);
            float* posXYZ   = (float*)(puVar1 - 0x37);
            float* rotation = (float*)(puVar1 - 0x38);
            float  scale    = *(float*)(puVar1 - 0x32);
            FUN_00474f90((int)classCode, posXYZ, rotation, Ff(scale));
        }
        puVar1 += 0x6F;
    }
}
