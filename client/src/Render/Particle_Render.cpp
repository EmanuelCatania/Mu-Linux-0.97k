// Particle_Render.cpp
// Particle_Render @ 0x0046BE40  (145 lines, 36 basic blocks)
//
// Updates particle simulation state and renders all active particles each frame.
// Called from Game_RenderTick after Terrain_Render().
//
// ── PARTICLE POOL ─────────────────────────────────────────────────────────────
//
//   Base:   DAT_07b116d0
//   Stride: 0x6f * 4 = 0x1bc bytes per entry
//   Count:  500 entries (pool ends at 0x7b271af = base + 500*0x1bc - 4)
//
//   Particle struct (offsets relative to piVar2, each field in int-size units):
//     piVar2[-0x18]    — byte: active flag (0 = slot free, skip)
//     piVar2[+0x40]    — byte: visible / renderable flag (0 = skip)
//     piVar2[-0x5e]    — ushort: particle type code
//     piVar2[-0x17]    — int: sub-type (color variant selector, 0-4)
//     piVar2[-0x15]    — int: lifetime counter (incremented by 1.0 per frame for 0x4a7)
//     piVar2[-0x14]    — float: world X position
//     piVar2[-0x13]    — float: world Y position
//     piVar2[-0x0f]    — float: world Z position (negated when passed to draw)
//     piVar2[+0x00]    — int: used for 0x4f0/0x4f1 size/countdown
//
// ── PER-FRAME UPDATES (each active particle) ─────────────────────────────────
//
//   _rand()                — advance RNG state
//   GL_SetBlendAdditive()         — frame counter / timing update (used for 0x4f1 orbit angle)
//
// ── PARTICLE TYPE SWITCH ──────────────────────────────────────────────────────
//
//   case 0x4a7  (Linear / fire spark):
//     piVar2[-0x15] += DAT_005526e4     (lifetime += delta_time)
//     size = piVar2[0] * _DAT_005524f4  (base_size * tile_scale)
//     RGB = (constant R, G, B based on velocity)
//     gravity = -(float)piVar2[-0xf]
//     → FUN_004f8bb0(0x4a7, x, y, size, size, &color_r, z_neg, 1.0)
//
//   case 0x4b0  (Random walk / snow):
//     uVar1 = rand() & 0x80000003   (small random int)
//     size  = (uVar1 + 8) * _DAT_005524f4
//     RGB   = constant
//     → FUN_004f8bb0(0x4b0, x, y, 2.0, 2.0, &color_r, z_neg, 1.0)
//
//   case 0x4f0  (Fade / glow):
//     Lifetime fade:
//       if piVar2[0] < 5: fVar3 -= (5 - count) * delta
//       if piVar2[-0x17] == 4: local_18 = (float)piVar2[-0x15]
//                              special sin-based size for type 4
//       else: local_18 = (0x14 - piVar2[0]) * 0.052f  (linear scale-down)
//     Color by sub-type (piVar2[-0x17]):
//       0, 1: local_c = fVar3*0x48B, local_8 = fVar3*0x53C, local_4 = fVar3
//       2:    local_c = fVar3*0x48B, local_8 = fVar3,       local_4 = fVar3*0x53C
//       3:    local_c = fVar3,       local_8 = fVar3*0x53C, local_4 = fVar3*0x48B
//       4:    local_c = fVar3,       local_8 = fVar3*0x50C, local_4 = fVar3*0x24F
//       (warm orange/red, warm reverse, cool blue-green, fire red-orange)
//     → FUN_004f8bb0(0x4f0, x, y, scale, scale, &local_c, z_neg, 1.0)
//
//   case 0x4f1  (Circular orbit / portal ring):
//     GL_SetBlendAdditive()   — get frame time
//     fVar6 = (frame_counter % 0xe10) * _DAT_00552a00  (rotation angle, 0-3600°)
//     if piVar2[-0x17] == 2:
//       if piVar2[0] < 0xb: size = 1.0 - (10-count)*1.0
//       else: local_18 = (0x14-count) * some_scale; size = 1.0
//     else:
//       FUN_00473ea0(0x4f1, pos, 360, 520, 600, +fVar6, 0, 0.0)  → Orbit+
//       FUN_00473ea0(0x4f1, pos, 360, 520, 600, -fVar6, 0, 0.0)  → Orbit-
//     → FUN_004f8bb0(0x4f0, x, y, size, size, &local_c, z_neg, 1.0)
//
//   default (0x4a8 and all others):
//     goto switchD_0046be8b_caseD_4a8 → skip directly to:
//     FUN_004f8bb0(type, x, y, piVar2[-0x17], local_18, &local_c, z_neg, 1.0)
//     (passes sub-type as 'w', local_18 as extra param)
//     Note: 0x4a8 case falls through to default dispatch.
//
// ── PARTICLE DRAW CALL ────────────────────────────────────────────────────────
//
//   FUN_004f8bb0(type, x, y, w, h, color_ptr, z, alpha)
//     → Particle_Draw(type, world_x, world_y, size_w, size_h, &rgba, world_z, alpha)
//     Likely: project world coords to screen, call glVertex/glTexCoord, set glColor.
//     type selects texture or render mode (0x4a7=spark, 0x4b0=snowflake, 0x4f0=glow blob)
//
// ── FUNCTION CROSS-REFERENCE ─────────────────────────────────────────────────
//
//   FUN_004f8bb0  → Particle_Draw(type, x, y, w, h, color, z, alpha)
//   GL_SetBlendAdditive  → Frame_GetTime() or Frame_UpdateCounter()
//   FUN_00473ea0  → Particle_SpawnOrbit(type, pos, r_min, r_mid, r_max, angle, flag, param)
//   _rand         → MSVC rand()
//   DAT_005526e4  → g_DeltaTime (float, seconds per frame)
//   DAT_005524f4  → g_TileScale (float, world units per tile)
//   DAT_0055256c  → particle max intensity / base_size constant

#include "stdafx.h"
#include "Render/Particle_Render.h"

extern "C" void DbgLogPublic(const char*);   // [DIAG TEMP #4]

// =============================================================================
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 648-789 (142 lines)
// =============================================================================
// FUN_0046be40 @ 0x0046BE40 — Particle_Render
// 2026-05-07: port FIEL desde IDA mu97k-src-IDA/raw/0046BE40_Particle_Render.c.
// Itera el effect pool DAT_07b11670 (200 slots × 0x1BC bytes). Para cada slot
// activo y visible, despacha por type code (1191/1200/1264/1265) llamando a
// RenderTerrainAlphaBitmap con escala/color/rotación per-tipo:
//   1191 = scaling glow (incrementa scale 0.2/frame, color desde counter)
//   1200 = animated puff (random scale 0.8..1.1)
//   1264 = colored puff con palette de 5 variantes (var=0..4)
//   1265 = special rotating effect con 2 sprites + var-dependent fade
//
// IDA walker: v1 = (float*)(pool_base + 0x60), stride 111 floats = 0x1BC.
// Slot offsets (relativos a v1, byte offsets):
//   +0     active flag (byte at v1-96)
//   +2     type code (WORD at v1-94)
//   +4     variant/palette (int at v1-92, = "v4" 0..4)
//   +12    scale increment field (float at v1-84)
//   +16    pos.x (float)
//   +20    pos.y (float)
//   +36    rotation (float, NEGATED on render)
//   +96    lifetime counter (int at v1+0)
//   +352   visible flag (byte at v1+256)
// (RenderTerrainAlphaBitmap declared in functions.h; FUN_00473ea0 too)
void __cdecl FUN_0046be40(void)
{
    char* base = (char*)&DAT_07b11670[0];
    for (int i = 0; i < 200; ++i) {
        char* slot = base + i * 0x1BC;
        if (slot[0] == 0) continue;          // inactive
        if (slot[352] == 0) continue;         // not visible

        WORD typeCode = *(WORD*)(slot + 2);
        int  variant  = *(int*)(slot + 4);
        int  counter  = *(int*)(slot + 96);
        float posX    = *(float*)(slot + 16);
        float posY    = *(float*)(slot + 20);
        float rot     = *(float*)(slot + 36);
        float scaleF  = *(float*)(slot + 12);

        _rand();                              // IDA calls rand() once unconditionally
        // IDA 0046BE40: EnableAlphaBlend().  00511790 is the separate
        // "minus" blend mode; using it here makes the normal particle pass
        // differ from the original blend equation.
        GL_SetBlendAdditive();

        switch (typeCode) {
        case 1191: {
            float newScale = scaleF + 0.2f;
            *(float*)(slot + 12) = newScale;
            GLfloat color[3];
            color[0] = color[1] = color[2] = (float)counter * 0.1f;
            RenderTerrainAlphaBitmap(1191, posX, posY, newScale, newScale,
                                     color, -rot, 1.0f);
            break;
        }
        case 1200: {
            int r = _rand() % 4;
            GLfloat color[3];
            color[0] = color[1] = color[2] = (float)(r + 8) * 0.1f;
            RenderTerrainAlphaBitmap(1200, posX, posY, 2.0f, 2.0f,
                                     color, -rot, 1.0f);
            break;
        }
        case 1264: {
            float v3 = 1.0f;
            if (counter < 5) v3 = 1.0f - (float)(5 - counter) * 0.2f;
            float v21;
            if (variant == 4) {
                v21 = scaleF;
                if (v3 == 1.0f) {
                    v3 = (float)sin((double)(60 - counter) * 0.05) + 0.5f;
                }
            } else {
                v21 = (float)(20 - counter) * 0.15f;
            }
            GLfloat color[3];
            switch (variant) {
            case 0: case 1:
                color[0] = v3 * 0.4f;
                color[1] = v3 * 0.6f;
                color[2] = v3;
                break;
            case 2:
                color[0] = v3 * 0.4f;
                color[1] = v3;
                color[2] = v3 * 0.6f;
                break;
            case 3:
                color[0] = v3;
                color[1] = v3 * 0.6f;
                color[2] = v3 * 0.4f;
                break;
            case 4:
                color[0] = v3;
                color[1] = v3 * 0.5f;
                color[2] = v3 * 0.1f;
                break;
            default:
                color[0] = color[1] = color[2] = v3;
                break;
            }
            RenderTerrainAlphaBitmap(1264, posX, posY, v21, v21,
                                     color, -rot, 1.0f);
            break;
        }
        case 1265: {
            // IDA repeats EnableAlphaBlend for type 1265.
            GL_SetBlendAdditive();
            float v24 = (float)(((__int64)WorldTime) % 3600) * 0.1f;
            float v5 = 1.0f;
            float v0val = 0.0f;
            if (variant == 2) {
                if (counter <= 10) {
                    v5 = 1.0f - (float)(10 - counter) * 0.1f;
                } else {
                    v0val = (float)(20 - counter) * 0.55f;
                }
            } else {
                // 2 sprites con rotación opuesta usando sub_473EA0
                // sub_473EA0 (Particle_Spawn) — IDA passes integer bit
                // patterns 1119092736/1124204544/1128792064 = floats 90/130/180.
                // Per functions.h:350 sig is (int, float*, uint, uint, uint,
                // float, uint, float) — we pass slot pos pointer.
                float* slotPosPtr = (float*)(slot + 16);
                FUN_00473ea0(1265, slotPosPtr,
                             0x42B40000u, 0x43020000u, 0x43340000u,
                             v24, 0u, 0.0f);
                FUN_00473ea0(1265, slotPosPtr,
                             0x42B40000u, 0x43020000u, 0x43340000u,
                             -v24, 0u, 0.0f);
                if (counter >= 5) v5 = 1.0f;
                else v5 = 1.0f - (float)(5 - counter) * 0.2f;
                v0val = (float)(20 - counter) * 0.15f;
            }
            GLfloat color[3];
            color[0] = v5;
            color[1] = v5 * 0.4f;
            color[2] = v5 * 0.2f;
            RenderTerrainAlphaBitmap(1264, posX, posY, v0val, v0val,
                                     color, -rot, 1.0f);
            break;
        }
        default:
            break;
        }
    }
}
