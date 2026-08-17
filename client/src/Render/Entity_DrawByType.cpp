// Entity_DrawByType.cpp — FUN_004fae00 @ 0x004fae00
// Dispatches entity render based on entity type (*(short*)(param_1+2)).
//
// param_1 = entity data pointer (stride 0x394, entity array at DAT_07abf5d0)
// param_2 = second entity pointer (unused in most cases)
// param_3 = extra param (used in type 0x14a HashTable block)
// param_4 = flag byte (player-focused override for some types)
//
// Render model context:
//   this = (void*)(DAT_05828d58 + entity_type * 0xbc)
//   this+0x48/4c/50 = tint RGB floats
//   this+0x88       = render layer mask
//
// Entity fields used:
//   param_1+0x02  = entity_type (short)
//   param_1+0x04  = unk_04 (int)
//   param_1+0x58  = facing/anim_id (float/int) — -2 = invisible guard
//   param_1+0x68  = world_x
//   param_1+0x6c  = world_y
//   param_1+0x70  = world_z
//   param_1+0x74  = render_mode byte
//   param_1+0x78  = entity_flags (uint) — bit0=player flag, bit1=PvP
//   param_1+0x7c  = unk_7c byte
//   param_1+0x84  = alive state
//   param_1+0x8b  = flash counter (10=full red, >0=fading)
//   param_1+0x105 = anim_state (byte) — 6=death trigger
//   param_1+0x168 = scale float
//   param_1+0x17c = base level (int)
//   param_1+100   = height_int
//
// ── Anti-tamper note ──────────────────────────────────────────────────────────
// Type 0x14a contains an ~300-line HashTable encode/decode block operating on
// DAT_083a7c00 (a local slot key). Per CLAUDE.md policy this is obfuscation,
// not game logic. The post-hash render logic IS implemented.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include <windows.h>
#include <GL/gl.h>
#include <math.h>

extern "C" void DbgLogPublic(const char*);   // [DIAG TEMP #4]
extern "C" void DbgForge(const char* fn, int type, int model, int bmp, int glTex,
                         int mesh, int blend, float wx, float wy, float wz,
                         float r, float g, float b, float a);   // [DIAG FORGE]

// fsin is an x87 FPU intrinsic declared in math.h — no separate extern needed

// ─────────────────────────────────────────────────────────────────────────────

void __cdecl FUN_004fae00(void *param_1_v, int param_2, int param_3, char param_4)
{
    unsigned char *param_1 = (unsigned char *)param_1_v;
    // Model context for this entity type
    void *model = (void *)(DAT_05828d58 + *(short *)(param_1 + 2) * 0xbc);


    // Invisible guard — IDA Draw_RenderObject L82:
    //     if ( !EditFlag && *(_DWORD *)(o + 88) == -2 ) return;
    // `+0x58` (88) es HiddenMesh; -2 = objeto entero invisible (marcadores como
    // PoseBox01 / Light01-03). `DAT_07e11d30` es EditFlag y vale 0 fuera del
    // editor, así que este return SÍ dispara.
    if (DAT_07e11d30 == 0 && *(int *)(param_1 + 0x58) == -2)
        return;

    // ── Tint overrides based on game sub-state / type ──────────────────────────

    // Sub-state 10 (dense rain) + type 0x12d → blue-tinted tint
    if (DAT_0055a7ac == 10 && *(short *)(param_1 + 2) == 0x12d) {
        *(float *)((int)model + 0x48) = 0.3f * 0.5f;   // ~0.020 R (0x3ca3d70a)
        *(float *)((int)model + 0x4c) = 0.05f;          // 0x3d4ccccd G
        *(float *)((int)model + 0x50) = 0.15f;          // 0x3e19999a B
        goto LAB_render_dispatch;
    }

    // Sub-state 9 (snow) + type 0x120 → icy tint
    if (DAT_0055a7ac == 9 && *(short *)(param_1 + 2) == 0x120) {
        *(float *)((int)model + 0x48) = 0.0f;
        *(float *)((int)model + 0x4c) = 0.3f;           // 0x3e99999a G
        *(float *)((int)model + 0x50) = 1.0f;            // 0x3f800000 B
    }

LAB_render_dispatch:
    // param_4 + type 0x129 → pure-blue override
    if (param_4 != '\0' && *(short *)(param_1 + 2) == 0x129) {
        *(float *)((int)model + 0x48) = 0.0f;
        *(float *)((int)model + 0x4c) = 0.0f;
        *(float *)((int)model + 0x50) = 1.0f;
    }

    // Render mode 0x80: special full render + return
    if (param_1[0x74] == (unsigned char)0x80) {
        FUN_00441e00(model, 0x82,
                     *(float *)(param_1 + 0x168), *(float *)(param_1 + 100),
                     *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                     *(float *)(param_1 + 0x70),  *(float *)(param_1 + 0x58),
                     0xffffffff);
        return;
    }

    short sType = *(short *)(param_1 + 2);

    // ── Type dispatch ──────────────────────────────────────────────────────────

    if (sType == 0x128) {
        // Double-pass: dim orange first, then normal
        *(float *)((int)model + 0x48) = 0.4f;    // 0x3ecccccd R
        *(float *)((int)model + 0x4c) = 0.575f;  // 0x3f19999a G
        *(float *)((int)model + 0x50) = 1.0f;
        *(unsigned char *)((int)model + 0x88) = 0;
        FUN_00441e00(model, 2,
                     *(float *)(param_1 + 0x168), *(float *)(param_1 + 100),
                     *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                     *(float *)(param_1 + 0x70),  *(float *)(param_1 + 0x58),
                     0x492);
        *(float *)((int)model + 0x48) = 1.0f;
        *(float *)((int)model + 0x4c) = 1.0f;
        *(float *)((int)model + 0x50) = 1.0f;
        *(unsigned char *)((int)model + 0x88) = 0xff;
        goto LAB_simple_render;
    }

    if (sType == 0x10b)
        goto LAB_simple_render;

    if (DAT_0055a7ac == 0) {
        if (sType == 0x69) {
            // IDA 0x004FAE00: Waterspout01 in Lorencia renders 4 explicit mesh
            // passes with mesh indices 0..3 and object fields as the remaining
            // parameters. Our prior decompile treated small integer args as
            // denormal floats, collapsing mesh selection and blend args.
            FUN_00440d30();
            FUN_00440d50(model, 0.0f, 2,
                         *(float *)(param_1 + 0x168), *(float *)(param_1 + 100),
                         *(float *)(param_1 + 104), 0.0f, 0.0f, 0xffffffff);
            FUN_00440d50(model, 1.0f, 2,
                         *(float *)(param_1 + 0x168), *(float *)(param_1 + 100),
                         *(float *)(param_1 + 104), 0.0f, 0.0f, 0xffffffff);
            FUN_00440d50(model, 2.0f, 2,
                         *(float *)(param_1 + 0x168), *(float *)(param_1 + 100),
                         *(float *)(param_1 + 104), 0.0f, 0.0f, 0xffffffff);
            FUN_00440d50(model, 3.0f, 2,
                         *(float *)(param_1 + 0x168), 3.0f,
                         *(float *)(param_1 + 104), *(float *)(param_1 + 108),
                         *(float *)(param_1 + 0x70),  0xffffffff);
            glPopMatrix();
            return;
        }
    } else if (DAT_0055a7ac == 4) {
        // Char-select scene: specific entity types get extra render passes
        if (sType == 0x17 || sType == 0x13 || sType == 0x14 || sType == 3 || sType == 4) {
            _rand();
            sType = *(short *)(param_1 + 2);
            if (sType == 0x17) {
                // Normal char-select render
                goto LAB_substate4_done;
            }
            if (sType == 0x13 || sType == 0x14) {
                // Save tint, render with warm overlay, restore
                float savedR = *(float *)((int)model + 0x48);
                float savedG = *(float *)((int)model + 0x4c);
                float savedB = *(float *)((int)model + 0x50);
                *(float *)((int)model + 0x48) = 1.0f;
                *(float *)((int)model + 0x4c) = 0.2f;   // 0x3e4ccccd
                *(float *)((int)model + 0x50) = 0.1f;   // 0x3dcccccd
                *(unsigned char *)((int)model + 0x88) = 2;
                FUN_00441e00(model, 2,
                             *(float *)(param_1 + 0x168), *(float *)(param_1 + 100),
                             *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                             *(float *)(param_1 + 0x70),  *(float *)(param_1 + 0x58),
                             0x492);
                *(float *)((int)model + 0x48) = savedR;
                *(float *)((int)model + 0x4c) = savedG;
                *(float *)((int)model + 0x50) = savedB;
                *(unsigned char *)((int)model + 0x88) = 0xff;
            } else {
                if (sType != 3 && sType != 4) return;
                // Decoration objects: same warm overlay
                float savedR = *(float *)((int)model + 0x48);
                float savedG = *(float *)((int)model + 0x4c);
                float savedB = *(float *)((int)model + 0x50);
                *(float *)((int)model + 0x48) = 1.0f;
                *(float *)((int)model + 0x4c) = 0.2f;
                *(float *)((int)model + 0x50) = 0.1f;
                *(unsigned char *)((int)model + 0x88) = 1;
                FUN_00441e00(model, 2,
                             *(float *)(param_1 + 0x168), *(float *)(param_1 + 100),
                             *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                             *(float *)(param_1 + 0x70),  *(float *)(param_1 + 0x58),
                             0x492);
                *(float *)((int)model + 0x48) = savedR;
                *(float *)((int)model + 0x4c) = savedG;
                *(float *)((int)model + 0x50) = savedB;
                *(unsigned char *)((int)model + 0x88) = 0xff;
            }
LAB_substate4_done:
            FUN_00441e00(model, 2,
                         *(float *)(param_1 + 0x168), *(float *)(param_1 + 100),
                         *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                         *(float *)(param_1 + 0x70),  *(float *)(param_1 + 0x58),
                         0xffffffff);
            return;
        }
    } else if (DAT_0055a7ac == 8) {
        if (sType == 0x51) {
            // Map transition portal
            FUN_00440d30();
            FUN_00440d50(model, 0.0f,  2,
                         *(float *)(param_1 + 0x168), *(float *)(param_1 + 100),
                         *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                         *(float *)(param_1 + 0x70),  0xffffffff);
            FUN_00440d50(model, 0.0f,  0x44,
                         *(float *)(param_1 + 0x168), *(float *)(param_1 + 100),
                         *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                         *(float *)(param_1 + 0x70),  0x492);
            glPopMatrix();
            return;
        }
    } else if (DAT_0055a7ac > 10 && DAT_0055a7ac < 0x11) {
        if (sType == 0x1c || sType == 0x1d) {
            // Map-load transition: render + shadow drop
            FUN_00440d30();
            FUN_00441e00(model, 2,
                         *(float *)(param_1 + 0x168), *(float *)(param_1 + 100),
                         *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                         *(float *)(param_1 + 0x70),  *(float *)(param_1 + 0x58),
                         0xffffffff);
            glPopMatrix();
            FUN_00511680('\x01');
            glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
            float wx = *(float *)(param_1 + 0x10);
            float wy = *(float *)(param_1 + 0x14);
            *(float *)((int)model + 0x74) = FUN_004f7500(wx, wy);
            *(float *)((int)model + 0x6c) = wx;
            *(float *)((int)model + 0x70) = wy;
            *(int *)(param_1 + 0x58) = 2;
            FUN_00441f00(model, *(int *)(param_1 + 100), 2);
            *(int *)(param_1 + 0x58) = -1;
            return;
        }
    }

    // ── Entity flag tint (PvP/party) ───────────────────────────────────────────
    {
        unsigned char flags = (unsigned char)*(unsigned int *)(param_1 + 0x78);
        if (flags & 1) {
            if (flags & 2) {
                // PvP player: yellow-green
                *(float *)((int)model + 0x4c) = 1.0f;
                *(float *)((int)model + 0x48) = 0.3f;   // 0x3e99999a
                *(float *)((int)model + 0x50) = 0.6f;   // 0x3f4ccccd
            } else {
                // Party player: yellow
                *(float *)((int)model + 0x4c) = 1.0f;
                *(float *)((int)model + 0x48) = 0.3f;
                *(float *)((int)model + 0x50) = 0.5f;   // 0x3f000000
            }
        } else if (flags & 2) {
            // PvP non-party: blue-orange
            *(float *)((int)model + 0x48) = 0.3f;
            *(float *)((int)model + 0x4c) = 0.5f;
            *(float *)((int)model + 0x50) = 1.0f;
        }
    }

    // ── Flash / hit-flash counter (param_1+0x8b) ─────────────────────────────
    {
        unsigned char flashCtr = param_1[0x8b];
        if (flashCtr == 10) {
            // Full red flash
            *(float *)((int)model + 0x48) = 1.0f;
            *(float *)((int)model + 0x4c) = 0.3f;
            *(float *)((int)model + 0x50) = 0.3f;
            param_1[0x8b] = flashCtr - 1;
        } else if (flashCtr != 0) {
            // Fading flash
            *(float *)((int)model + 0x48) = 1.0f;
            float fade = (float)(10 - (int)flashCtr) * _DAT_00552a10 + _DAT_005528b8;
            *(float *)((int)model + 0x4c) = fade;
            *(float *)((int)model + 0x50) = fade;
            param_1[0x8b] = flashCtr - 1;
        }
    }

    // ── Type-specific render dispatch ─────────────────────────────────────────
    sType = *(short *)(param_1 + 2);

    // Scale fixup for types 0x131/0x11d/0x140/0x141 at specific scale+x
    if ((sType == 0x131 || sType == 0x11d || sType == 0x140 || sType == 0x141) &&
        *(int *)(param_1 + 0x168) == 0x3f800000 &&
        *(int *)(param_1 + 0x68)  == 0x3d4ccccd) {
        *(float *)(param_1 + 0x68) = 1.0f;
    }

    if (sType == 0x138 && *(int *)(param_1 + 4) == 1) {
        // Wave-bobbing object (water lily etc.)
        FUN_00440d30();
        float wave = (float)(fsin((long double)DAT_05826e08 * (long double)_DAT_005528e0)
                     * (long double)_DAT_005524f8 + (long double)_DAT_0055256c);
        FUN_00440d50(model, 0.0f,        0x42,
                     *(float *)(param_1 + 0x168), 0.0f, wave,
                     *(float *)(param_1 + 0x6c),  *(float *)(param_1 + 0x70), 0x56c);
        FUN_00440d50(model, 1.4013e-45f, 0x42,
                     *(float *)(param_1 + 0x168), 1.4013e-45f, wave,
                     *(float *)(param_1 + 0x6c),  *(float *)(param_1 + 0x70), 0x56b);
        FUN_00440d50(model, 2.8026e-45f, 0x42,
                     *(float *)(param_1 + 0x168), 1.4013e-45f, wave,
                     *(float *)(param_1 + 0x6c),  *(float *)(param_1 + 0x70), 0xffffffff);
        FUN_00440d50(model, 2.8026e-45f, 0x42,
                     *(float *)(param_1 + 0x168), 1.4013e-45f, wave,
                     *(float *)(param_1 + 0x6c),  *(float *)(param_1 + 0x70), 0xffffffff);
        glPopMatrix();
        goto LAB_postprocess;
    }

    if (sType == 0x14c) {
        // Double render (main + overlay at scale 1.0)
        FUN_00441e00(model, 2,
                     *(float *)(param_1 + 0x168), *(float *)(param_1 + 100),
                     *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                     *(float *)(param_1 + 0x70),  *(float *)(param_1 + 0x58),
                     0xffffffff);
        FUN_00440d50(model, 2.8026e-45f, 2,
                     *(float *)(param_1 + 0x168), 2.8026e-45f, 1.0f,
                     *(float *)(param_1 + 0x6c),  *(float *)(param_1 + 0x70), 0xffffffff);
        goto LAB_postprocess;
    }

    if (sType == 0x13f) {
        // Portal/gate object: render + VTable dispatch
        FUN_00440d30();
        FUN_00440d50(model, 0.0f, 2,
                     *(float *)(param_1 + 0x168), *(float *)(param_1 + 100),
                     *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                     *(float *)(param_1 + 0x70),  0xffffffff);
        glPopMatrix();
        // VTable-dispatch secondary effect
        void *vObj = operator_new(0x20);
        if (vObj) {
            int *vThis = (int *)FUN_0040a660(vObj);
            typedef void (__cdecl *VFn4)(DWORD*, void*, unsigned char*, int);
            typedef void (__cdecl *VFn0)(void);
            ((*(VFn4*)(*vThis + 4)))(&DAT_0584621c, model, param_1, 1);
            FUN_0040a860(vThis, 0x584621c);
            ((*(VFn0*)(*vThis + 8)))();
        }
        goto LAB_postprocess;
    }

    if (sType == 0x144) {
        if (param_4 != '\0') {
            // Colored dual render
            *(float *)((int)model + 0x48) = 0.1f;   // 0x3dcccccd
            *(float *)((int)model + 0x4c) = 0.1f;
            *(float *)((int)model + 0x50) = 0.1f;
            FUN_00441e00(model, 2,
                         *(float *)(param_1 + 0x168), *(float *)(param_1 + 100),
                         *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                         *(float *)(param_1 + 0x70),  1.4013e-45f, 0xffffffff);
            *(float *)((int)model + 0x48) = 1.0f;
            *(float *)((int)model + 0x4c) = 0.1f;
            *(float *)((int)model + 0x50) = 0.1f;
            FUN_00440d50(model, 1.4013e-45f, 2,
                         *(float *)(param_1 + 0x168), *(float *)(param_1 + 100),
                         *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                         *(float *)(param_1 + 0x70),  0xffffffff);
            goto LAB_postprocess;
        }
        goto LAB_standard_render;
    }

    if (sType == 0x142) {
        // Double-pass with mid tint + optional PvP override
        *(unsigned char *)((int)model + 0x88) = 0;
        *(float *)((int)model + 0x48) = 0.4f;   // 0x3ecccccd
        *(float *)((int)model + 0x4c) = 0.3f;   // 0x3e99999a
        *(float *)((int)model + 0x50) = 0.5f;   // 0x3f000000
        FUN_00441e00(model, 2,
                     *(float *)(param_1 + 0x168), *(float *)(param_1 + 100),
                     *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                     *(float *)(param_1 + 0x70),  *(float *)(param_1 + 0x58),
                     0x4f6);
        *(float *)((int)model + 0x48) = 0.5f;
        *(float *)((int)model + 0x4c) = 0.5f;
        *(float *)((int)model + 0x50) = 0.5f;
        *(unsigned char *)((int)model + 0x88) = 0xff;
        if (param_4 != '\0') {
            *(float *)((int)model + 0x48) = 0.1f;
            *(float *)((int)model + 0x4c) = 1.0f;
            *(float *)((int)model + 0x50) = 0.6f;  // 0x3f4ccccd
        }
        goto LAB_standard_render;
    }

    if (sType == 0x143) {
        // Purple-tint render (chest/relic)
        *(float *)((int)model + 0x48) = 0.9f;   // 0x3f666666
        *(float *)((int)model + 0x4c) = 0.8f;   // 0x3f4ccccd
        *(float *)((int)model + 0x50) = 1.0f;
        FUN_00441e00(model, 4,
                     *(float *)(param_1 + 0x168), *(float *)(param_1 + 100),
                     *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                     *(float *)(param_1 + 0x70),  *(float *)(param_1 + 0x58),
                     0x493);
        *(float *)((int)model + 0x48) = 1.0f;
        *(float *)((int)model + 0x4c) = 1.0f;
        *(float *)((int)model + 0x50) = 1.0f;
        goto LAB_standard_render;
    }

    if (sType == 0x141) {
        FUN_00441e00(model, 2,
                     *(float *)(param_1 + 0x168), *(float *)(param_1 + 100),
                     *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                     *(float *)(param_1 + 0x70),  1.4013e-45f, 0xffffffff);
        FUN_00440d50(model, 1.4013e-45f, 0x402,
                     *(float *)(param_1 + 0x168), *(float *)(param_1 + 100),
                     *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                     *(float *)(param_1 + 0x70),  *(unsigned int *)(param_1 + 0x58));
        FUN_00440d50(model, 1.4013e-45f, 0x402,
                     *(float *)(param_1 + 0x168), *(float *)(param_1 + 100),
                     *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                     *(float *)(param_1 + 0x70),  *(unsigned int *)(param_1 + 0x58));
        goto LAB_postprocess;
    }

    if (sType == 0x146) {
        // White then gray dual render
        *(unsigned char *)((int)model + 0x88) = 0;
        *(float *)((int)model + 0x48) = 1.0f;
        *(float *)((int)model + 0x4c) = 1.0f;
        *(float *)((int)model + 0x50) = 1.0f;
        FUN_00441e00(model, 0x42,
                     *(float *)(param_1 + 0x168), *(float *)(param_1 + 100),
                     *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                     *(float *)(param_1 + 0x70),  *(float *)(param_1 + 0x58),
                     0x492);
        *(unsigned char *)((int)model + 0x88) = 0xff;
        *(float *)((int)model + 0x48) = 0.575f;  // 0x3f19999a
        *(float *)((int)model + 0x4c) = 0.575f;
        *(float *)((int)model + 0x50) = 0.575f;
        goto LAB_standard_render;
    }

    if (sType == 0x10c) {
        // Direct render at facing=0
        FUN_00441e00(model, 0x402,
                     *(float *)(param_1 + 0x168), *(float *)(param_1 + 100),
                     *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                     *(float *)(param_1 + 0x70),  1.4013e-45f, 0xffffffff);
        goto LAB_postprocess;
    }

    if (sType == 0x14b) {
        if (param_1[0x105] == '\x06') {
            // Death animation trigger
            *param_1 = 0;
            FUN_00404bc0(0x6a, 0, 0);
            FUN_00441be0(model, 0, 0x106);
            goto LAB_postprocess;
        }
        goto LAB_standard_render;
    }

    if (sType == 0x14a && DAT_005615c0 == 2) {
        if (param_1[0x105] == '\x06') {
            *param_1 = 0;
            FUN_00404bc0(0x6a, 0, 0);
            FUN_00441be0(model, 0, 0x104);
            goto LAB_postprocess;
        }

        FUN_00441e00(model, 2,
                     *(float *)(param_1 + 0x168), *(float *)(param_1 + 100),
                     *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                     *(float *)(param_1 + 0x70),  *(float *)(param_1 + 0x58),
                     0xffffffff);
        *(float *)((int)model + 0x48) = 1.0f;
        *(float *)((int)model + 0x4c) = 1.0f;
        *(float *)((int)model + 0x50) = 1.0f;
        FUN_00441e00(model, 0x44,
                     *(float *)(param_1 + 0x168), *(float *)(param_1 + 100),
                     *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                     *(float *)(param_1 + 0x70),  *(float *)(param_1 + 0x58),
                     1170);
        *(float *)((int)model + 0x48) = 0.3f;
        *(float *)((int)model + 0x4c) = 0.3f;
        *(float *)((int)model + 0x50) = 1.0f;
        FUN_00441e00(model, 0x48,
                     *(float *)(param_1 + 0x168), *(float *)(param_1 + 100),
                     *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                     *(float *)(param_1 + 0x70),  *(float *)(param_1 + 0x58),
                     1170);
        goto LAB_postprocess;
    }

    if (sType == 0x14a) {
        if (param_1[0x105] != '\x06') {
            // ── Anti-tamper HashTable block (~300 lines) on DAT_083a7c00 ──────
            // Operates on the entity's cached render state.
            // Omitted per project policy (hash table operations are not game logic).
            // ────────────────────────────────────────────────────────────────────

            // Post-hash logic: check entity level vs base level
            int curLevel = param_3;  // iStack_24 from Ghidra — caller-passed level
            if ((int)curLevel - *(int *)(param_1 + 0x17c) < 0x19) {
                // Level too low / dying
                if (param_1[0x105] == '\x06') {
                    *param_1 = 0;
                }
                FUN_00404bc0(0x6a, 0, 0);
                FUN_00441be0(model, 0, 0x104);
            } else {
                // Full triple-layer render
                FUN_00441e00(model, 2,
                             *(float *)(param_1 + 0x168), *(float *)(param_1 + 100),
                             *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                             *(float *)(param_1 + 0x70),  *(float *)(param_1 + 0x58),
                             0xffffffff);
                *(float *)((int)model + 0x48) = 1.0f;
                *(float *)((int)model + 0x4c) = 1.0f;
                *(float *)((int)model + 0x50) = 1.0f;
                FUN_00441e00(model, 0x44,
                             *(float *)(param_1 + 0x168), *(float *)(param_1 + 100),
                             *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                             *(float *)(param_1 + 0x70),  *(float *)(param_1 + 0x58),
                             0x492);
                *(float *)((int)model + 0x50) = 1.0f;
                *(float *)((int)model + 0x48) = 0.3f;
                *(float *)((int)model + 0x4c) = 0.3f;
                FUN_00441e00(model, 0x48,
                             *(float *)(param_1 + 0x168), *(float *)(param_1 + 100),
                             *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                             *(float *)(param_1 + 0x70),  *(float *)(param_1 + 0x58),
                             0x492);
            }
            goto LAB_postprocess;
        }
        goto LAB_standard_render;
    }

    if (sType == 0x104 || sType == 0x105) {
        // Multi-layer entity: base + armor + weapon
        FUN_00441e00(model, 2,
                     *(float *)(param_1 + 0x168), *(float *)(param_1 + 100),
                     *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                     *(float *)(param_1 + 0x70),  *(float *)(param_1 + 0x58),
                     0xffffffff);
        *(float *)((int)model + 0x48) = 1.0f;
        *(float *)((int)model + 0x4c) = 1.0f;
        *(float *)((int)model + 0x50) = 1.0f;
        FUN_00441e00(model, 0x44,
                     *(float *)(param_1 + 0x168), *(float *)(param_1 + 100),
                     *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                     *(float *)(param_1 + 0x70),  *(float *)(param_1 + 0x58),
                     0x492);
        *(float *)((int)model + 0x50) = 1.0f;
        *(float *)((int)model + 0x48) = 0.3f;
        *(float *)((int)model + 0x4c) = 0.3f;
        FUN_00441e00(model, 0x48,
                     *(float *)(param_1 + 0x168), *(float *)(param_1 + 100),
                     *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                     *(float *)(param_1 + 0x70),  *(float *)(param_1 + 0x58),
                     0x492);
        goto LAB_postprocess;
    }

LAB_standard_render:
    FUN_00441e00(model, 2,
                 *(float *)(param_1 + 0x168), *(float *)(param_1 + 100),
                 *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                 *(float *)(param_1 + 0x70),  *(float *)(param_1 + 0x58),
                 0xffffffff);
    goto LAB_postprocess;

LAB_simple_render:
    FUN_00441e00(model, 2,
                 *(float *)(param_1 + 0x168), *(float *)(param_1 + 100),
                 *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                 *(float *)(param_1 + 0x70),  *(float *)(param_1 + 0x58),
                 0xffffffff);

LAB_postprocess:
    // ── Post-render: PvP outline ───────────────────────────────────────────────
    if (((unsigned char)*(unsigned int *)(param_1 + 0x78) & 2) == 2) {
        FUN_00441e00(model, 2,
                     *(float *)(param_1 + 0x168), -1.0f /*NaN*/, 1.0f,
                     *(float *)(param_1 + 0x6c),  *(float *)(param_1 + 0x70),
                     *(float *)(param_1 + 0x58),  0xffffffff);
    }

    // ── Type 0x133: wave-bobbing shadow ───────────────────────────────────────
    if (*(short *)(param_1 + 2) == 0x133) {
        FUN_00440d30();
        float wave2 = (float)(fsin((long double)DAT_05826e08 * (long double)_DAT_005528e0)
                      * (long double)_DAT_005528b8 + (long double)_DAT_00552504);
        int texFrame = (int)(*(unsigned int *)((int)*(int *)((int)model + 0x38) + 0xc));
        FUN_00440d50(model, 0.0f, 2,
                     *(float *)(param_1 + 0x168), 0.0f, wave2,
                     *(float *)(param_1 + 0x6c), *(float *)(param_1 + 0x70),
                     texFrame);
        // Modulo-100 periodic shadow offset (based on frame counter)
        int framemod = (int)((long long)DAT_05826e08 % 100);
        float modFrac = -(float)(framemod * _DAT_005524f8);
        FUN_00440d50(model, 4.2039e-45f, 0x44,
                     *(float *)(param_1 + 0x168), 4.2039e-45f, wave2,
                     *(float *)(param_1 + 0x6c), modFrac, 0xffffffff);
        glPopMatrix();
    }

    // ── Types 0x178/0x177: extra zero-position render ─────────────────────────
    if (*(short *)(param_1 + 2) == 0x178 || *(short *)(param_1 + 2) == 0x177) {
        FUN_00440d50(model, 0.0f, 0x42,
                     *(float *)(param_1 + 0x168), *(float *)(param_1 + 100),
                     *(float *)(param_1 + 0x68),  0.0f, 0.0f, 0xffffffff);
    }
}
