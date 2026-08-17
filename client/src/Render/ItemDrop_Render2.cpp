// ItemDrop_Render2.cpp — FUN_00478c00 @ 0x00478c00
// Sprite-pool render loop for decoration/item-drop billboards.
//
// Iterates a fixed sprite entity pool starting at (DAT_07abf634 - 0x44) = 0x07abf5f0,
// stride 0x70 bytes, up to address limit 0x7b116b3 (~300 entries).
//
// Pool entry layout (relative to pool base, stride 0x70):
//   +0x00  char   active           — skip if 0
//   +0x04  uint   entity_type      — switch key; maps into type table at DAT_083a7cc0
//   +0x08  uint   anim_state       — animation sub-state (5/7/8 = special cases)
//   +0x0c  uint   scale_mult       — integer multiplier for world scale
//   +0x10  float  pos[3]           — world position (x, y, z)
//   +0x28  float  uv_base[3]       — UV base coords (used in 0x4df inner loop)
//   +0x38  int    tick / frame_src — frame source or tick counter
//   +0x40  uint   current_frame    — current sprite frame (may be updated by 0x4f4)
//   +0x44  uint   frame_counter    — incremented each call (first field named by Ghidra)
//   +0x48  float  scale_factor     — extra scale for 0x598 type
//   +0x58  float  anim_speed       — animation speed multiplier (for 0x4f4 type)
//   +0x68  float  base_frame       — base frame offset (for 0x4f4 type)
//
// Type table at DAT_083a7cc0, stride 0x38:
//   +0x00  float  scale_x          — base world-X scale per type
//   +0x04  float  scale_y          — base world-Y scale per type
//   +0x08  char   hide_flag        — if == 3: call FUN_00511710() (hide anim sprite)
//
// Called from Scene_Login, Scene_CharSelect each frame to animate/draw background
// billboards (item drops, decorative glows, spell sprites).
//
// After per-type UV / frame computation, calls:
//   FUN_00511d00(type, pos, scale_x, scale_y, uv_base, frame, u0, v0, u_size, v_size)

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

extern "C" void DbgLogPublic(const char*);
extern "C" void DbgForge(const char* fn, int type, int model, int bmp, int glTex,
                         int mesh, int blend, float wx, float wy, float wz,
                         float r, float g, float b, float a);   // [DIAG FORGE]

// ─────────────────────────────────────────────────────────────────────────────

void FUN_00478c00(void)
{
    // Pool fix 2026-04-27: AUTO-SKIP previo bloqueaba TODO el render del particle
    // pool DAT_07abf5f0 — particles spawneadas via CreateParticle (FUN_00475220)
    // (lightning ELS=10/11, waterfall +9 glow, fire/smoke, rain/snow) NUNCA se
    // dibujaban. Ahora itera por índice acotado a 3000 slots.
    uint  *puVar8  = (uint*)((char*)DAT_07abf5f0 + 0x44);  // +0x44 from slot 0 base
    uint   local_2c = 0;             // entry index / loop counter

    do {
        // active flag is at pool_base+0 = puVar8[-0x11]
        if ((char)puVar8[-0x11] == '\0') goto next_entry;

        {
            // BUG-FIX 2026-04-28: bounds-check entity_type. La tabla en
            // DAT_083a7cc0 tiene 0x600 entradas (stride 0x38). Tipos fuera
            // de rango leen memoria inválida → AV. Causa: el pool se sembraba
            // con tipos garbage (ej. bits de un float) cuando un particle se
            // marcaba activo pero no se inicializaba bien.
            uint entityType = puVar8[-0x10];
            if (entityType >= 0x600) goto next_entry;

            int   iVar2  = (int)entityType * 0x38;
            // BUG-FIX (sistema de partículas): el campo scale (slot+0x0c) es un FLOAT.
            // IDA: Width = Bitmaps[type].Width * *((float*)v1 - 14). El port hacía
            // `(float)puVar8[-0xe]` = conversión int→float de los BITS del float →
            // p.ej. 0.5f (bits 0x3F000000 = 1056964608) se convertía en 1e9 → billboard
            // gigante → whiteout. Reinterpretamos los bits como el binario original.
            float fVar13 = *(float *)((char *)&DAT_083a7cc0 + iVar2) * *(float *)(puVar8 - 0xe);
            float fVar14 = *(float *)((char *)&DAT_083a7cc4 + iVar2) * *(float *)(puVar8 - 0xe);

            // hide or show the character anim sprite based on type record flag
            if (((char *)&DAT_083a7cc8)[iVar2] == '\x03')
                FUN_00511710();          // EnableAlphaBlend
            else
                // ── 2026-08-16: CAUSA DEL CUADRO BLANCO ───────────────────────
                // IDA 00478C00 L66-72:
                //     if (Bitmaps[v2].Components == 3) EnableAlphaBlend();
                //     else                             EnableAlphaTest(0);
                // `EnableAlphaTest` es **0x00511680**. El comentario anterior
                // afirmaba que estaba "mapped at 00511590" y es FALSO: 0x511590
                // es `DisableTexture(bool)`, que hace glDisable(GL_TEXTURE_2D).
                // O sea TODA particula con Components != 3 apagaba el
                // texturizado y su quad salia pintado con el color plano
                // (blanco). Como las particulas se dibujan de a cientos
                // (parts llego a 439 en el log) y el estado GL queda pegado,
                // se veia una masa blanca de bordes escalonados que ademas
                // contaminaba lo que se dibujara despues.
                // Mismo error que ya estaba en SkillEffect_Render; es la 4ta
                // vez que esta familia muerde (ver la tabla en CLAUDE.md).
                FUN_00511680('\0');      // EnableAlphaTest(0)

            uint  uVar3  = puVar8[-0x10];  // entity_type (passed to FUN_00511d00)
            uint  uVar6;
            float fVar15, fVar16, fVar17, fVar18, fVar19;

            switch (uVar3)
            {
            // ── 0x4a6: simple 4-frame column strip ─────────────────────────
            case 0x4a6:
                uVar6 = *puVar8 & 0x80000003u;
                if ((int)uVar6 < 0) uVar6 = (uVar6 - 1u | 0xfffffffcu) + 1u;
                fVar19 = 1.0f; fVar18 = 0.25f; fVar17 = 0.0f;
                fVar16 = (float)(int)uVar6 * _DAT_005528dc;
                fVar15 = *(float *)(puVar8 - 10);
                fVar13 = fVar13 * _DAT_005528dc;
                break;

            // ── 0x4ab / 0x4ad: state-dependent column strip ────────────────
            case 0x4ab:
            case 0x4ad:
                uVar6 = puVar8[-0xf];
                if (uVar6 == 5 || uVar6 == 7 || uVar6 == 8) {
                    fVar15 = *(float *)(puVar8 - 1);
                } else {
                    fVar15 = *(float *)(puVar8 - 10);
                }
                uVar6 = *puVar8 & 0x80000003u;
                if ((int)uVar6 < 0) uVar6 = (uVar6 - 1u | 0xfffffffcu) + 1u;
                fVar19 = 1.0f; fVar18 = 0.25f; fVar17 = 0.0f;
                fVar16 = (float)(int)uVar6 * _DAT_005528dc;
                fVar13 = fVar13 * _DAT_005528dc;
                break;

            // ── 0x4b6: 2-row × 2-col grid strip ───────────────────────────
            case 0x4b6:
                fVar19 = 0.5f; fVar18 = 0.5f;
                uVar6 = *puVar8 & 0x80000001u;
                fVar17 = (float)((int)*puVar8 / 2) * _DAT_00552504;
                if ((int)uVar6 < 0) uVar6 = (uVar6 - 1u | 0xfffffffeu) + 1u;
                fVar16 = (float)(int)uVar6 * _DAT_00552504;
                fVar15 = 0.0f;
                break;

            // ── 0x4bf: 4-row × 4-col sprite sheet (row = tick/4) ──────────
            case 0x4bf:
            {
                uint uVar11;
                uVar6  = *puVar8;
                uVar11 = uVar6 & 0x80000003u;
                fVar17 = (float)((int)(uVar6 + ((int)uVar6 >> 0x1f & 3u)) >> 2) * _DAT_005528dc
                         + _DAT_00552940;
                if ((int)uVar11 < 0) uVar11 = (uVar11 - 1u | 0xfffffffcu) + 1u;
                fVar15 = (float)(int)uVar11;
                // shared tail with 0x596
                fVar19 = 0.24f; fVar18 = 0.24f;
                fVar16 = fVar15 * _DAT_005528dc + _DAT_00552940;
                fVar15 = 0.0f;
                break;
            }

            // ── 0x4c0: same as 0x4a6 (fall-through to shared code) ────────
            case 0x4c0:
                uVar6 = *puVar8 & 0x80000003u;
                if ((int)uVar6 < 0) uVar6 = (uVar6 - 1u | 0xfffffffcu) + 1u;
                fVar19 = 1.0f; fVar18 = 0.25f; fVar17 = 0.0f;
                fVar16 = (float)(int)uVar6 * _DAT_005528dc;
                fVar15 = *(float *)(puVar8 - 10);
                fVar13 = fVar13 * _DAT_005528dc;
                break;

            // ── 0x4c4: show anim on states 2/5, single-frame otherwise ────
            case 0x4c4:
                if (puVar8[-0xf] == 2 || puVar8[-0xf] == 5)
                    FUN_00511790();
                fVar19 = 1.0f; fVar18 = 1.0f; fVar17 = 0.0f; fVar16 = 0.0f;
                fVar15 = (puVar8[-0xf] == 6) ? 0.0f : *(float *)(puVar8 - 1);
                uVar3  = puVar8[-0x10];
                break;

            // ── 0x4d9: 3×3 sprite-sheet (row = tick%9/3, col = tick%9%3) ──
            case 0x4d9:
                fVar17 = (float)(((int)*puVar8 % 9) / 3) * _DAT_005528dc + _DAT_00552940;
                fVar15 = (float)(((int)*puVar8 % 9) % 3);
                // shared tail with 0x596
                fVar19 = 0.24f; fVar18 = 0.24f;
                fVar16 = fVar15 * _DAT_005528dc + _DAT_00552940;
                fVar15 = 0.0f;
                break;

            // ── 0x4df: multi-frame interpolated trail (special inner loop) ─
            case 0x4df:
            {
                // Renders up to (local_3c+1) blended frames along a trail
                // using positions interpolated from puVar8[2..4] and tick math.
                int   local_3c = (int)(local_2c & 0x80000007u);
                if (local_3c < 0) local_3c = (local_3c - 1 | -8) + 1;
                local_3c += 0x16;

                int iVar9 = ((int)(puVar8[-3] + ((int)puVar8[-3] >> 0x1f & 3)) >> 2)
                            + (int)puVar8[-0xf];
                int iVar2b  = iVar9 / 10;
                iVar9       = iVar9 % 10;

                if (0 <= local_3c) {
                    int local_38 = 0;
                    float local_18[6];
                    do {
                        float *pfVar7 = (float *)(puVar8 + 2);
                        for (int iVar10 = 0; iVar10 < 3 * 4; iVar10 += 4) {
                            int iVar4 = iVar2b + 1;
                            int iVar5 = iVar4 % 3;
                            int iVar1 = (iVar4 - 1) % 3;
                            // interpolated world position
                            local_18[iVar10/4 + 3] =
                                pfVar7[-0xf] - (float)local_3c * pfVar7[0] * _DAT_005524f4;

                            int local_30 = (local_38 > 9) ? 10 : local_38;
                            uint local_34 = puVar8[-3];
                            if ((int)local_34 > 9) local_34 = 10;

                            local_18[iVar10/4] =
                                (float)(int)local_34 * _DAT_00552500 *
                                ((float)iVar9 * *(float *)(puVar8 + iVar5 - 7) +
                                 (float)(10 - iVar9) * *(float *)(puVar8 + iVar1 - 7))
                                * (float)local_30;
                            pfVar7++;
                            iVar2b++;
                        }
                        FUN_00511d00(0x4df, local_18 + 3, fVar13, fVar14,
                                     local_18, *(float *)(puVar8 - 1),
                                     0.0f, 0.0f, 1.0f, 1.0f);
                        local_3c--;
                        local_38++;
                    } while (0 <= local_3c);
                }
                goto next_entry;   // already drew above; skip final FUN_00511d00 call
            }

            // ── 0x4e1: single-frame or skip based on anim_state ───────────
            case 0x4e1:
                if (puVar8[-0xf] == 4) {
                    // fall through to default-style draw with current_frame
                    fVar15 = *(float *)(puVar8 - 1);
                    fVar19 = 1.0f; fVar18 = 1.0f; fVar17 = 0.0f; fVar16 = 0.0f;
                    break;
                }
                if (puVar8[-3] != 0x3c) {
                    // default: single frame
                    fVar15 = *(float *)(puVar8 - 1);
                    fVar19 = 1.0f; fVar18 = 1.0f; fVar17 = 0.0f; fVar16 = 0.0f;
                    break;
                }
                goto next_entry;

            // ── 0x4f4: time-driven animated frame (updates current_frame) ──
            case 0x4f4:
                if (puVar8[-0xf] == 0 || puVar8[-0xf] == 3) {
                    // pick speed constant based on even/odd entry index
                    uint uVar6b = local_2c & 0x80000001u;
                    bool bEven;
                    if ((int)uVar6b < 0) bEven = (uVar6b - 1u | 0xfffffffeu) == 0xffffffffu;
                    else                 bEven = (uVar6b == 0);
                    float fSpeed = bEven ? _DAT_00552914 : _DAT_00552acc;
                    // update current_frame = delta * entry_speed * speed + base_frame
                    // (float field: escribir bits float, no truncar a int)
                    *(float *)(puVar8 - 1) = DAT_05826e08 * *(float *)(puVar8 + 5) * fSpeed
                                        + *(float *)(puVar8 + 9);
                }
                fVar15 = *(float *)(puVar8 - 1);
                fVar19 = 1.0f; fVar18 = 1.0f; fVar17 = 0.0f; fVar16 = 0.0f;
                break;

            // ── 0x596: 4-row × 4-col sheet (same formula as 0x4bf) ─────────
            case 0x596:
            {
                uint uVar11;
                uVar6  = *puVar8;
                uVar11 = uVar6 & 0x80000003u;
                fVar17 = (float)((int)(uVar6 + ((int)uVar6 >> 0x1f & 3u)) >> 2) * _DAT_005528dc
                         + _DAT_00552940;
                if ((int)uVar11 < 0) uVar11 = (uVar11 - 1u | 0xfffffffcu) + 1u;
                fVar15 = (float)(int)uVar11;
                fVar19 = 0.24f; fVar18 = 0.24f;
                fVar16 = fVar15 * _DAT_005528dc + _DAT_00552940;
                fVar15 = 0.0f;
                break;
            }

            // ── 0x598: scaled billboard with per-entry size factor ─────────
            case 0x598:
                fVar15 = *(float *)(puVar8 - 1);
                fVar19 = 0.99f; fVar18 = 0.99f;
                fVar17 = 0.11f; fVar16 = 0.11f;
                fVar14 = fVar14 * _DAT_005528b8;
                fVar13 = fVar13 * *(float *)(puVar8 + 1);
                break;

            // ── default: plain single-frame quad ──────────────────────────
            default:
                fVar15 = *(float *)(puVar8 - 1);
                fVar19 = 1.0f; fVar18 = 1.0f; fVar17 = 0.0f; fVar16 = 0.0f;
                break;
            } // switch

            FUN_00511d00(uVar3,
                         (float *)(puVar8 - 0xd),  // world pos
                         fVar13, fVar14,
                         (float *)(puVar8 - 7),     // uv_base
                         fVar15, fVar16, fVar17, fVar18, fVar19);
        }

    next_entry:
        puVar8   += 0x1c;   // advance by 0x70 bytes (0x1c * sizeof(uint))
        local_2c += 1;
    } while (local_2c < 3000);
}
