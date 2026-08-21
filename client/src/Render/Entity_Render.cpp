// Entity_Render.cpp
// Entity_Render_Sprites @ 0x005038E0  (147 lines, 26 basic blocks)
//
// (forward decls)
// extern "C" void DbgLogPublic(const char*);
//
// Renders all active entities as 2D sprites (billboard quads) in the 3D world.
// Called from Game_RenderTick after Terrain_Render (and only in normal view mode).
// This is the "2D-in-3D" character/NPC sprite pass — distinct from the 3D geometry
// pass done inside Terrain_Render via Entity_Render_3D.
//
// ── ENTITY ARRAY ─────────────────────────────────────────────────────────────
//
//   Base:   DAT_07e12945
//   Stride: 0x204 bytes per entry (512 bytes)
//   Count:  ends when pointer > 0x7e908e5
//     → 500 entities: (0x7e908e5 - 0x07e12945) / 0x204 ≈ 500
//
//   Entity struct (offsets relative to puVar9):
//     puVar9[-0x105]   — byte: active flag (0 = slot unused, skip)
//     puVar9[-0xf5]    — float: world X position
//     puVar9[-0xf1]    — float: world Y position
//     puVar9[-0xed]    — float: world Z position
//     puVar9[-0x103]   — short: entity class code (drives model lookup)
//     puVar9[-0x145]   — uint:  entity state flags (byte>>3 & 0xf = sub-class)
//     puVar9[-0x12e]   — byte:  — (passed to FUN_00505a10)
//     puVar9[-0x1d..]  — floats: position-related offsets (for name tag placement)
//     puVar9[+0x03]    — float: rotation angle
//     puVar9[+0x07]    — float: — (model param)
//     puVar9[+0x01]    — byte:  — (passed to FUN_00440060)
//     puVar9[+0x63]    — float: — (passed to FUN_00505a10 as 'rot')
//     puVar9[-0xa9]    — short: screen X (world_to_screen result, written here)
//     puVar9[-0xa7]    — short: screen Y (world_to_screen result, written here)
//     puVar9[+0x5b]    — byte:  frustum visibility flag (written here)
//
// ── GLOBALS ───────────────────────────────────────────────────────────────────
//
//   DAT_05828d58  — sprite model table base (model entries at base + class * 0xbc)
//   DAT_07abf5d8  — local player entity (stride 0x394); byte+0x1bc & 7 = equipped weapon slot
//   DAT_0055a7ac  — g_GameSubState (== 10 → apply Z oscillation)
//   DAT_05826e08  — frame oscillation counter (for Z bob)
//   local_9c      — running counter incremented each active entity
//   local_98      — angle accumulator for Z bob (+=0x4d5 per entity)
//
// ── MAIN LOOP ─────────────────────────────────────────────────────────────────
//
//   puVar9 = &DAT_07e12945
//   do {
//     if puVar9[-0x105] == '\0': goto next_entity
//
//     pfVar1 = &puVar9[-0xf5]           // position pointer
//     sVar6 = FUN_004f9590(pfVar1, 400.0)  → FrustumCull_2D(pos, 400.0)
//     puVar9[0x5b] = (char)sVar6         // store visibility
//     if sVar6 == '\0': goto next_entity  // frustum culled
//
//     // Class code → model index
//     sVar6 = *(short*)(puVar9 - 0x103)  (entity class code)
//     iVar7 = (int)sVar6                 (default model index = class code)
//
//     if 0x270 <= sVar6 <= 0x30f:        // standard playable classes (0x270-0x30f range)
//       iVar7 = 0x186                    // remap all player classes to DW model index
//     else if sVar6 == 0x35c:            // special NPC type 0x35c
//       sub = (entity_state >> 3) & 0xf  (state-driven sub-class)
//       if sub == 0: iVar7 = 0x3b3
//       if sub == 2: iVar7 = 0x3b4
//     // else: iVar7 = sVar6 (use class code directly as model index)
//
//     this = (void*)(DAT_05828d58 + iVar7 * 0xbc)  // model data for class
//
//     // Set model fields from entity
//     this[+0xa0] = 0
//     this[+0x98] = local_player.byte1bc & 7     (equipped weapon type)
//     this[+0xa0] = puVar9[0]                    (entity byte at +0x00)
//
//     // Copy position to model
//     this[+0x6c] = puVar9[-0xf5]   (world X)
//     this[+0x70] = puVar9[-0xf1]   (world Y)
//     this[+0x74] = puVar9[-0xed]   (world Z)
//
//     FUN_00503830(class, this) → Sprite_SetupAnimation(class, model)
//     FUN_00440060(this, 0x6970a9c, rot, pos, scale, ptr1, ptr2, '\0', '\0')
//         → Sprite_Draw(model, flags=0x6970a9c, rot, pos, scale, ...) — main draw call
//
//     // Terrain slope angle for grounding
//     FUN_004f7960(x, y, &local_74) → Terrain_GetAngle(x, y, out_angle)
//     local_74 += puVar9[-0x1d]   (entity Z angle offset)
//     local_70 += puVar9[-0x19]   (entity X angle offset)
//     local_6c += puVar9[-0x15]   (entity Y angle offset)
//
//     // Caso especial: modelo 0x35f (863 = Gold01, el Zen del suelo)
//     if sVar6 == 0x35f:
//       n = ftol(sqrt(cantidad de Zen) / 2)  acotado a [3, 80]
//       for i in 1..n:
//         // desplazamiento en círculo tomado de RandomTable (DAT_055c9e58)
//         local_60 = {random_angle, 0, 0}
//         local_3c = {random_radius, 0, 0}
//         FUN_004f9db0(local_60, local_3c+3) → Matrix_FromEuler
//         FUN_004fa0b0(local_3c, local_3c+3, &local_54) → Matrix_Transform
//         offset_pos = base_pos + local_54
//         FUN_00505a10(entity, class, 0, &local_74, rot, state_flags, ...)
//             → Entity_DrawAt(entity, class, 0, angle, rot, flags, ...) (trail node)
//       // restore original pos
//
//     // Main sprite draw
//     FUN_00505a10(entity, class, 0, &local_74, rot, state_flags, ...)
//         → Entity_DrawAt(entity, class, 0, angle, rot, flags, ...)
//
//     // Z oscillation (when g_GameSubState == 10)
//     if g_GameSubState == 10:
//       puVar9[-0xed] += sin(local_98 + DAT_05826e08) * _DAT_00552488
//
//     // World-to-screen projection for UI (HP bars, name tags)
//     FUN_005113f0(&pos, (int*)local_68, (int*)local_64)
//         → World_ToScreen(pos, &screen_x, &screen_y)
//     puVar9[-0xa9] = local_68[0]   (screen X)
//     puVar9[-0xa7] = local_64[0]   (screen Y)
//
//   next_entity:
//     puVar9 += 0x204
//     local_94 += 1
//     local_9c += 0x14
//     local_98 += 0x4d5
//   } while puVar9 < 0x7e908e5
//
// ── MODEL TABLE (DAT_05828d58) ────────────────────────────────────────────────
//
//   Stride: 0xbc (188 bytes) per class entry
//   Indexed by iVar7 (class code or remapped code):
//     0x186  = Dark Wizard (or player class remap)
//     0x3b3  = special NPC sub-type 0
//     0x3b4  = special NPC sub-type 2
//     0x35f  = Zen del suelo (dibuja N copias del modelo de moneda)
//
//   Model entry fields:
//     +0x6c  float: world X
//     +0x70  float: world Y
//     +0x74  float: world Z
//     +0x98  byte:  weapon slot (from local player)
//     +0xa0  byte:  entity flag (copied from entity[0])
//
// ── FUNCTION CROSS-REFERENCE ─────────────────────────────────────────────────
//
//   FUN_004f9590  → FrustumCull_2D(float *pos, float max_dist) — returns short
//   FUN_00503830  → Sprite_SetupAnimation(class, model_ptr)
//   FUN_00440060  → Sprite_Draw(model, flags, rot, pos_ptr, scale_ptr, anim_ptr, dir_ptr, a, b)
//   FUN_00505a10  → Entity_DrawAt(entity, class, slot, angle_ptr, rot, state, byte, a, b, c, d, e)
//   FUN_004f7960  → Terrain_GetAngle(world_x, world_y, out_angle_xyz)
//   FUN_005113f0  → World_ToScreen(pos[3], out_x, out_y)
//   FUN_004f9db0  → Matrix_FromEuler(angles[3], out_mat[12])
//   FUN_004fa0b0  → Matrix_TransformPoint(pt, mat, out)
//   DAT_055c9e58  → RandomTable (100 ints; reparte en círculo las monedas del Zen)
//   DAT_05826e08  → frame oscillation counter (for Z bob in sub-state 10)
//   DAT_0055284c  → Z bob amplitude constant
//   DAT_00552488  → Z oscillation scale factor

#include "stdafx.h"
#include "Render/Entity_Render.h"
#include <math.h>

extern "C" void DbgLogPublic(const char* msg);

// ⚠ CÓDIGO MUERTO — NO USAR.  La copia VIVA de Entity_Render (0x5038E0) es
// `FUN_005038e0` en stubs_render_helpers.cpp: es la que llama Render_Frame.cpp
// (`FUN_005038e0()`), y nadie llama a `Entity_Render()`.  Esta copia además
// camina el pool con el ancla equivocada (base + 0x105 en vez de base + 0x14D,
// o sea lee el flag `active` en Items+0 y no en Items+72).  Se deja porque
// documenta la función, pero cualquier arreglo va en la copia viva.
//
// Entity_Render (sprite loop) @ 0x005038E0 (147 lines)
// Iterates all ground items, does LOD/visibility, handles the Zen coin pile (type 0x35f),
// applies Z-bob in sub-state 10, then calls Entity_DrawAt and projects to screen.
//
// 2026-05-03: AUTO-SKIP removed. The "DAT_07e12945" walker base is +0x105 bytes
// inside DAT_07e12840 (the 1000-slot × 0x204-byte sprite pool, properly sized
// in globals.cpp). Original IDA bound 0x7e908e5 = pool_end + 0x105 (= start
// of slot[1000]'s "+0x105" field) → 1000 iterations. Replaced with explicit
// count + computed walker from the symbol base instead of literal address.
void Entity_Render(void)
{
    float      *pfVar1;
    void       *pModel;
    float       fVar2, fVar3, fVar4;
    undefined4  uVar5;
    short       sVar6;
    int         iVar7;
    uint        uVar8;
    undefined1 *puVar9;
    int         iVar10;
    float10     fVar11;
    longlong    lVar12;
    int         local_a0;                 // comet trail length
    int         local_9c = 1;             // trail step counter
    int         local_98 = 0;             // Z-bob phase accumulator
    int         local_94 = 0;            // entity index (for random table)
    float       local_74[3] = {};  // terrain angle output (x,y,z) from FUN_004f7960
#define local_70 local_74[1]    // Ghidra alias: local_70 is local_74+4
#define local_6c local_74[2]    // Ghidra alias: local_6c is local_74+8
    undefined2  local_68[2] = {}, local_64[2] = {};   // screen XY output
    float       local_60[3] = {};    // Euler angles for trail
    float       local_xyz[3] = {};   // Matrix_TransformPoint output
#define local_54 local_xyz[0]   // Ghidra alias
#define local_50 local_xyz[1]
#define local_4c local_xyz[2]
    float       local_3c[15] = {};        // rotation matrix (Matrix_FromEuler output)

    // BUG-FIX 2026-05-03: was `puVar9 = (undefined1*)&DAT_07e12945; do{…} while
    // ((int)puVar9 < 0x7e908e5);` — DAT_07e12945 is a stray DWORD = 0 in our
    // globals (only the +0x105 "anchor" field). The real pool base is
    // DAT_07e12840 (1000 × 0x204). Walk from DAT_07e12840 + 0x105 with explicit
    // count of 1000 slots × 0x204 stride.
    puVar9 = (undefined1*)(&DAT_07e12840[0] + 0x105);
    for (int spriteIdx = 0; spriteIdx < 1000; ++spriteIdx) {
        if (puVar9[-0x105] != '\0') {   // active flag
            pfVar1 = (float *)(puVar9 + -0xf5);   // world XYZ

            // Visibility/LOD cull (returns slot byte if visible)
            sVar6 = FUN_004f9590(pfVar1, 400.0f);
            puVar9[0x5b] = (char)sVar6;
            if ((char)sVar6 != '\0') {

                sVar6 = *(short *)(puVar9 + -0x103);   // entity type
                iVar7 = (int)sVar6;

                // Map player class range 0x270..0x30f → 0x186
                if ((sVar6 < 0x270) || (0x30f < sVar6)) {
                    if (sVar6 == 0x35c) {
                        // DarkKnight sub-class resolution
                        uVar8 = *(int *)(puVar9 + -0x145) >> 3 & 0xf;
                        if      (uVar8 == 0) iVar7 = 0x3b3;
                        else if (uVar8 == 2) iVar7 = 0x3b4;
                    }
                } else {
                    iVar7 = 0x186;
                }

                pModel = (void *)(DAT_05828d58 + iVar7 * 0xbc);
                *(undefined1 *)((int)pModel + 0xa0) = 0;
                *(byte *)((int)pModel + 0x98) = *(byte *)(DAT_07abf5d8 + 0x1bc) & 7;
                *(undefined1 *)((int)pModel + 0xa0) = *puVar9;
                *(float *)((int)pModel + 0x6c) = *pfVar1;                             // world X
                *(undefined4 *)((int)pModel + 0x70) = *(undefined4 *)(puVar9 + -0xf1); // world Y
                *(undefined4 *)((int)pModel + 0x74) = *(undefined4 *)(puVar9 + -0xed); // world Z

                FUN_00503830((int)*(short *)(puVar9 + -0x103), (int)pModel);

                // Bone animation setup + draw (anim[3] from entity offsets)
                FUN_00440060(pModel, (int)&DAT_06970a9c,
                             *(float *)(puVar9 + 3),
                             *(undefined4 *)(puVar9 + 7),
                             puVar9[1],
                             (undefined4 *)(puVar9 + -0xe9),
                             (float *)(puVar9 + -0xdd),
                             '\0', '\0');

                // Terrain slope angle for this position
                FUN_004f7960(*pfVar1, *(float *)(puVar9 + -0xf1), local_74);
                local_74[0] += *(float *)(puVar9 + -0x1d);
                local_74[1] += *(float *)(puVar9 + -0x19);
                local_74[2] += *(float *)(puVar9 + -0x15);

                // ── Montón de monedas del Zen del suelo (modelo 863 = 0x35f) ──
                // IDA 0x503A46-0x503BCA: para el Zen se dibuja el modelo Gold01
                // N veces repartido en círculo alrededor del punto de drop, con
                // N = clamp(sqrt(cantidad) / 2, 3, 80).  La cantidad vive en
                // Items+8 (= puVar9 - 0x145), que es donde CreateItem la guarda
                // para el tipo 463.
                //
                // 2026-08-21: el port calculaba N con la distancia a la cámara
                // (mal-guess del artefacto __ftol) y encima leía RandomTable con
                // aritmética de DWORD* (paso de 16 bytes) sobre un global que era
                // un único DWORD = 0 → todas las monedas con ángulo y radio 0,
                // apiladas en un punto.  De ahí el "Zen sin sprite expandido".
                if (*(short *)(puVar9 + -0x103) == 0x35f) {
                    fVar2 = *pfVar1;
                    fVar3 = *(float *)(puVar9 + -0xf1);
                    fVar4 = *(float *)(puVar9 + -0xed);
                    lVar12 = (longlong)sqrt((double)*(int *)(puVar9 + -0x145));
                    local_a0 = (int)lVar12 / 2;
                    if (local_a0 < 3)    local_a0 = 3;
                    if (0x50 < local_a0) local_a0 = 0x50;

                    iVar10 = 1;
                    iVar7  = local_9c;
                    if (1 < local_a0) {
                        do {
                            // Orbital offsets from random table
                            local_60[0] = 0.0f; local_60[1] = 0.0f;
                            local_3c[1] = 0.0f; local_3c[2] = 0.0f;
                            local_60[2] = (float)(RandomTable[iVar7 % 100] % 0x168);
                            local_3c[0] = (float)(RandomTable[(iVar10 + local_94) % 100]
                                                  % (local_a0 + 0x14));
                            FUN_004f9db0(local_60, local_3c + 3);         // Matrix_FromEuler
                            FUN_004fa0b0(local_3c, local_3c + 3, local_xyz); // Matrix_TransformPoint
                            // Temporarily offset world pos for trail ghost
                            *(float *)(puVar9 + -0xf5) = local_54 + fVar2;
                            *(float *)(puVar9 + -0xf1) = local_50 + fVar3;
                            *(float *)(puVar9 + -0xed) = local_4c + fVar4;
                            FUN_00505a10((int)(puVar9 + -0x105),
                                         (int)*(short *)(puVar9 + -0x103),
                                         0, &local_74[0],
                                         *(float *)(puVar9 + 99),
                                         *(unsigned int *)(puVar9 + -0x145),
                                         puVar9[-0x12e],
                                         '\x01', 1, '\x01', 0, 2);
                            iVar10++; iVar7++;
                        } while (iVar10 < local_a0);
                    }
                    // Restore world pos
                    *(float *)(puVar9 + -0xf5) = fVar2;
                    *(float *)(puVar9 + -0xf1) = fVar3;
                    *(float *)(puVar9 + -0xed) = fVar4;
                }

                // Snapshot current pos
                fVar2 = *(float *)(puVar9 + -0xf5);
                uVar5 = *(undefined4 *)(puVar9 + -0xf1);
                fVar3 = *(float *)(puVar9 + -0xed);

                // ── Z-bob in sub-state 10 ─────────────────────────────────────
                if (DAT_0055a7ac == 10) {
                    fVar11 = (float10)sinl(((float10)local_98 + (float10)DAT_05826e08)
                                           * (float10)_DAT_005528e0);
                    *(float *)(puVar9 + -0xed) = (float)(fVar11 * (float10)_DAT_00552488
                                                         + (float10)*(float *)(puVar9 + -0xed));
                }

                // Main sprite draw
                FUN_00505a10((int)(puVar9 + -0x105),
                             (int)*(short *)(puVar9 + -0x103),
                             0, local_74,
                             *(float *)(puVar9 + 99),
                             *(unsigned int *)(puVar9 + -0x145),
                             puVar9[-0x12e],
                             '\x01', 1, '\x01', 0, 2);

                // Restore Z (in case of Z-bob)
                *(float *)(puVar9 + -0xed) = fVar3;
                *(float *)(puVar9 + -0xf5) = fVar2;
                *(undefined4 *)(puVar9 + -0xf1) = uVar5;

                // Project to screen coords — local_48/44/40 form a float[3] on the original stack
                // local_48 = X, local_44 = Y (as undefined4 = raw float bits), local_40 = Z+offset
                float screen_pos[3];
                screen_pos[0] = fVar2;                         // X
                screen_pos[1] = *(float *)&uVar5;              // Y (reinterpret undefined4→float)
                screen_pos[2] = fVar3 + _DAT_0055284c;        // Z + name-tag height offset

                // Project to screen coords and store in entity
                FUN_005113f0(screen_pos, (int *)local_68, (int *)local_64);  // World_ToScreen
                *(undefined2 *)(puVar9 + -0xa9) = local_68[0];
                *(undefined2 *)(puVar9 + -0xa7) = local_64[0];
            }
        }

        puVar9   = puVar9 + 0x204;
        local_94++;
        local_9c  += 0x14;
        local_98  += 0x4d5;
    }  // end of explicit count loop
}


// FUN_004fc030 @ 0x004fc030
//
// Entity_PrepareRender — validates an entity then sets up its render state.
// Calls FUN_004faa70 (Entity_IsVisible) and, if non-zero, FUN_004fae00
// (Entity_SetupRenderState) to configure matrices/culling for the entity.

void __cdecl FUN_004fc030(unsigned char *param_1,unsigned int param_2,int param_3,char param_4)

{
  undefined4 uVar1;

  uVar1 = FUN_004faa70((int)param_1,(char)param_2,param_3);
  if ((char)uVar1 != '\0') {
    FUN_004fae00(param_1,param_2,param_3,param_4);
  }
  return;
}


// FUN_00454fc0 — Entity_UpdateVisibility
// Updates one entity's visibility flag and per-frame state.
// param_1: entity pointer (float*)
// Checks active flag (byte at param_1+0), runs frustum test, then calls:
//   FUN_004f8ff0  — Frustum_TestPoint2D (returns visible flag)
//   FUN_00454cd0  — update animation frame
//   FUN_00449900  — compute screen position
//   FUN_004520c0  — update entity state
void __cdecl FUN_00454fc0(float *param_1)
{
  undefined2 uVar1;

  if (*(char *)param_1 != '\0') {
    uVar1 = FUN_004f8ff0(param_1[4] * _DAT_005524f8,param_1[5] * _DAT_005524f8,-20.0);
    *(char *)(param_1 + 0x58) = (char)uVar1;
    FUN_00454cd0((int)param_1,(int)param_1);
    FUN_00449900((int)param_1);
    FUN_004520c0((int)param_1);
  }
  return;
}


// FUN_0045ab00 — Entity_TickAll  (DISABLED 2026-04-26: duplicate definition;
// active version is in src/Render/Entity_RenderAll_3D.cpp with diag tracers)
#if 0
void FUN_0045ab00(void)
{
  char *pcVar1;
  int iVar2;
  int iVar3;
  undefined4 *puVar4;

  iVar3 = 0;
  DAT_07abf5d4 = 0;
  _DAT_07abf5e8 = 0;
  iVar2 = 0;
  do {
    pcVar1 = (char *)(DAT_07abf5d0 + iVar3);
    if (((pcVar1 == DAT_07abf5d8) && ((DAT_07abf5d8[0x1c0] & 4U) != 0)) && (DAT_005615c0 == 5)) {
      pcVar1[0x130] = '\0';
      pcVar1[0x131] = '\0';
      pcVar1[0x132] = 'z';
      pcVar1[0x133] = 'D';
      pcVar1[0x15c] = '\0';
      pcVar1[0x15d] = '\0';
      pcVar1[0x15e] = -0x80;
      pcVar1[0x15f] = '?';
      pcVar1[0x14c] = '\0';
      pcVar1[0x14d] = '\0';
      pcVar1[0x14e] = -0x80;
      pcVar1[0x14f] = '?';
      pcVar1[0x13c] = '\0';
      pcVar1[0x13d] = '\0';
      pcVar1[0x13e] = -0x80;
      pcVar1[0x13f] = '?';
    }
    else if ((*pcVar1 != '\0') && (pcVar1[0x160] != '\0')) {
      DAT_07abf5d4 = DAT_07abf5d4 + 1;
      if ((iVar2 == DAT_00559c50) || (iVar2 == DAT_00559c4c)) {
        puVar4 = (undefined4 *)0x1;
      }
      else {
        puVar4 = (undefined4 *)0x0;
      }
      FUN_00456770((undefined4 *)pcVar1,(undefined4 *)pcVar1,puVar4);
    }
    iVar3 = iVar3 + 0x394;
    iVar2 = iVar2 + 1;
  } while (iVar3 < 0x59740);
  return;
}
#endif  // duplicate FUN_0045ab00 disabled — use Entity_RenderAll_3D.cpp


// FUN_00500970 — RenderBugs (Entity_VisibilityCheckAll)
// Itera el pool de butterflies/effect-entities (DAT_083a1218, 10 entries × 0x1BC).
// Por cada entry activo: frustum test, si visible y (owner es player o type==0x330)
// llama a FUN_004fc030 (render). Type 0x330 además spawnea sparkle.
//
// BUGFIX 2026-04-26: tenía AUTO-SKIP early-return. Reactivado y reescrito para
// usar nuestro DAT_083a1218 array (stride 0x1bc, 10 entries). El layout original
// usaba pcVar4 = pool_base + 0x160 con índices negativos (-0x160 = +0); aquí
// indexamos directo desde el base del entry.
//
// Entry layout (offsets desde base):
//   +0x000  byte   active flag
//   +0x002  short  Type (816=butterfly, 195=elf-bug, 267=MG fire, 0x330=special)
//   +0x010  float  pos.x
//   +0x014  float  pos.y
//   +0x018  float  pos.z
//   +0x0FC  DWORD  owner (entity ptr)
//   +0x160  byte   visibility flag (escrito por frustum test)
uint FUN_00500970(void)
{
    // 2026-05-07: re-habilitado. Antes estaba TEMP DISABLED por flicker en
    // char-select. Ahora gated por g_GameState == 5 (in-world) para evitar
    // ese path. Port FIEL desde IDA mu97k-src-IDA/raw/00500970_RenderBugs.c.
    //
    // Pool: DAT_083a1218 (10 entries × 0x1BC = 4440 bytes).
    // Slot layout:
    //   +0     active flag (byte)
    //   +2     type code (WORD): 195=elf-bug, 267=MG fire, 816=butterfly, 0x330=special
    //   +16    pos.x (float)
    //   +20    pos.y (float)
    //   +0xFC  owner entity ptr (DWORD)
    //   +0x160 visible flag (byte)
    //
    // Logic per IDA: if active → frustum test → if visible AND (owner is
    // player [type 390] or type==816) → PrepareRender + sparkle for type 816.
    // BUG-FIX 2026-07-16: se gateaba a state 5/2, EXCLUYENDO char-select (state 4).
    // Eso rompía el render de las monturas (Uniria bug=195 / Dinorant bug=267) que
    // se crean con CreateBug y se dibujan acá vía FUN_004fc030 → Draw_RenderObject.
    // El IDA no tiene gate interno — Scene_CharSelect (0x523B30 L142) llama RenderBugs
    // directamente. Se agrega state 4.
    if (!(DAT_005615c0 == 5 || DAT_005615c0 == 4 || DAT_005615c0 == 2)) return 0;

    char* base = (char*)&DAT_083a1218[0];
    for (int i = 0; i < 10; ++i) {
        char* slot = base + i * 0x1BC;
        if (slot[0] == 0) continue;            // not active

        float posX = *(float*)(slot + 16);
        float posY = *(float*)(slot + 20);
        unsigned short vis = FUN_004f8ff0(posX * 0.01f, posY * 0.01f, -20.0f);
        slot[0x160] = (char)(vis != 0);
        if (!vis) continue;

        DWORD ownerPtr = *(DWORD*)(slot + 0xFC);
        short typeCode = *(short*)(slot + 2);

        bool ownerIsPlayer = false;
        if (ownerPtr) {
            short ownerType = *(short*)(ownerPtr + 2);
            ownerIsPlayer = (ownerType == 390);
        }

        if (ownerIsPlayer || typeCode == 0x330) {
            FUN_004fc030((unsigned char*)slot, 0u, 0, 0);
            if (typeCode == 0x330) {   // IDA: type==816 → sparkle 1150 (pool de efectos, no causa whiteout)
                // Fairy helper sparkle effect
                float intensity = (float)(_rand() % 30 + 70) * 0.01f;
                float light[3];
                light[0] = intensity * 0.5f;
                light[1] = intensity * 0.8f;
                light[2] = intensity * 0.6f;
                FUN_004795c0(1150, (float*)(slot + 16), 1.0f, light,
                             (int)slot, 0.0f, 0);
            }
        }
    }
    return 0;
}


// FUN_00503830 — Entity_SetGravity
// Sets the gravity value at param_2+0x84 based on entity type param_1.
// Maps specific type ranges to negative float constants (gravity strengths).
void __cdecl FUN_00503830(int param_1,int param_2)
{
  if (param_1 < 0x270) {
    if (param_1 < 0x290) goto LAB_0050386e;
  }
  else if (param_1 < 0x290) {
    *(undefined4 *)(param_2 + 0x84) = 0xc3200000;
    return;
  }
  if (param_1 < 0x2b0) {
    *(undefined4 *)(param_2 + 0x84) = 0xc2c80000;
    return;
  }
LAB_0050386e:
  if ((0x2cf < param_1) && (param_1 < 0x2f0)) {
    *(undefined4 *)(param_2 + 0x84) = 0xc28c0000;
    return;
  }
  if ((0x2af < param_1) && (param_1 < 0x2d0)) {
    *(undefined4 *)(param_2 + 0x84) = 0xc2480000;
    return;
  }
  if ((0x2ef < param_1) && (param_1 < 0x310)) {
    *(undefined4 *)(param_2 + 0x84) = 0;
    return;
  }
  *(undefined4 *)(param_2 + 0x84) = 0;
  return;
}


// FUN_00505970 — Entity_RenderSlotWith
// Sets up render parameters for one entity from entity data at param_2,
// then delegates to FUN_004404e0 (model-matrix setup) and FUN_00441e00 (draw).
// param_1: render object (void*)
// param_2: entity data ptr
// param_3: LOD/flag
// param_4: alpha param (char)
// param_5: alpha scale global override
void __cdecl FUN_00505970(void *param_1,void *param_2_v,int param_3,char param_4,int param_5)
{
  undefined *puVar1;
  int param_2 = (int)(uintptr_t)param_2_v;

  *(undefined1 *)((int)param_1 + 0x44) = 0;
  // _DAT_005597c8 is float; param_5 is a float passed as undefined4. Reinterpret
  // the bit pattern (cast through uint) so we don't do an int→float conversion.
  *(unsigned int *)&_DAT_005597c8 = (unsigned int)param_5;
  if (*(char *)(param_2 + 0x110) == '\x01') {
    puVar1 = *(undefined **)(param_2 + 0x114);
  }
  else {
    param_4 = '\0';
    puVar1 = (undefined*)&DAT_06970a9c;
  }
  FUN_004404e0(param_1,(int)puVar1,(float *)(param_2 + 0x118),(float *)(param_2 + 0x124),
               (float *)(param_2 + 0x130),param_4);
  FUN_00441e00(param_1,(uint)param_3,*(float *)(param_2 + 0x168),*(float *)(param_2 + 100),
               *(float *)(param_2 + 0x68),*(float *)(param_2 + 0x6c),*(float *)(param_2 + 0x70),
               *(float *)(param_2 + 0x58),0xffffffff);
  _DAT_005597c8 = 1.0f;   // reset LOD scale
  return;
}


// FUN_00505a10 — Entity_RenderFull
// Full entity render with LOD selection and double-pass for shadow/highlight.
// param_11: quality level (0/1/2/3) controlling alpha and double-draw.
// Skips if param_5 (distance) is below epsilon. Adjusts model type for
// class 0x35c by sub-class bits. Copies position/flag fields from entity
// to render object, then calls FUN_00504b50 for final draw.
void __cdecl
FUN_00505a10(int param_1,int param_2,undefined4 param_3,float *param_4,float param_5,uint param_6,
            byte param_7,char param_8,undefined1 param_9,char param_10,int param_11,uint param_12)
{
  void *this_;
  uint uVar1;
  undefined *puVar2;

  if (_DAT_005524f8 < param_5) {
    if (param_2 == 0x35c) {
      uVar1 = (int)param_6 >> 3 & 0xf;
      if (uVar1 == 0) {
        param_2 = 0x3b3;
      }
      else if (uVar1 == 2) {
        param_2 = 0x3b4;
      }
    }
    this_ = (void *)(DAT_05828d58 + param_2 * 0xbc);
    *(undefined1 *)((int)this_ + 0x99) = param_9;
    *(undefined4 *)((int)this_ + 0x68) = *(undefined4 *)(param_1 + 0xc);
    *(undefined1 *)((int)this_ + 0x45) = *(undefined1 *)(param_1 + 0xe4);
    *(undefined1 *)((int)this_ + 0x44) = *(undefined1 *)(param_1 + 0xdc);
    *(undefined4 *)((int)this_ + 0x6c) = *(undefined4 *)(param_1 + 0x10);
    *(undefined4 *)((int)this_ + 0x70) = *(undefined4 *)(param_1 + 0x14);
    *(undefined4 *)((int)this_ + 0x74) = *(undefined4 *)(param_1 + 0x18);
    _DAT_005597c8 = 1.0f;
    if (param_11 == 3) {
      _DAT_005597c8 = 1.4f;
    }
    else if (param_11 == 2) {
      _DAT_005597c8 = 1.2f;
    }
    else if (param_11 == 1) {
      if (*(char *)(param_1 + 0x84) == '\x04') {
        *(undefined4 *)((int)this_ + 0x48) = 0x3ca3d70a;
        *(undefined4 *)((int)this_ + 0x4c) = 0x3dcccccd;
      }
      else {
        *(undefined4 *)((int)this_ + 0x48) = 0x3dcccccd;
        *(undefined4 *)((int)this_ + 0x4c) = 0x3cf5c28f;
      }
      *(undefined4 *)((int)this_ + 0x50) = 0;
      FUN_00505970(this_,(void*)param_1,0x40,param_10,0x3f99999a);
      if (*(char *)(param_1 + 0x84) == '\x04') {
        *(undefined4 *)((int)this_ + 0x48) = 0x3e23d70a;
        *(undefined4 *)((int)this_ + 0x4c) = 0x3f333333;
      }
      else {
        *(undefined4 *)((int)this_ + 0x48) = 0x3f333333;
        *(undefined4 *)((int)this_ + 0x4c) = 0x3e4ccccd;
      }
      *(undefined4 *)((int)this_ + 0x50) = 0;
      FUN_00505970(this_,(void*)param_1,0x40,param_10,0x3f8a3d71);
    }
    FUN_004fa930(param_1,(int)this_);
    if (param_8 == '\0') {
      puVar2 = *(undefined **)(param_1 + 0x114);
    }
    else {
      puVar2 = (undefined*)&DAT_06970a9c;
    }
    FUN_004404e0(this_,(int)puVar2,(float *)(param_1 + 0x118),(float *)(param_1 + 0x124),
                 (float *)(param_1 + 0x130),param_10);
    FUN_00504b50(param_1,param_2,param_4,param_5,param_6,param_7,param_11,param_12);
  }
  return;
}
