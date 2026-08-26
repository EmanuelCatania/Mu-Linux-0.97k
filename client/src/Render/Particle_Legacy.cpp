// Particle_Legacy.cpp
// Extracted from stubs.cpp; IDA provenance comments retained.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "Net/MuEmu.h"
#include "Net/Net.h"
#include "Render/Camera.h"

// Character_UpdateAll @ 0x00479730 — Particle_RenderAll.
// Iterates effect/particle pool (base DAT_07C85890, stride 0x1BC).
// For each active slot: sets GL blend mode (0=normal,1=additive,2=alpha),
// calls FUN_00479670 to draw it, then clears the active flag.
// Pool fix 2026-04-27: AUTO-SKIP previo bloqueaba TODOS los efectos (glow +9,
// wing FX, etc.). Ahora itera por índice acotado (1002 slots).
void __cdecl Character_UpdateAll(void) {
    char *pcVar2 = DAT_07c85890;
    for (int i = 0; i < 1002; ++i, pcVar2 += 0x1bc) {
        if (*pcVar2 != '\0') {
            int blend = *(int*)(pcVar2 + 4);
            if      (blend == 0) FUN_00511710();
            else if (blend == 1) FUN_00511790();
            else if (blend == 2) FUN_00511680('\x01');
            FUN_00479670((int)pcVar2);
            *pcVar2 = 0;
        }
    }
}

// Effect_UpdateAll @ 0x00479790 — marks all active particle entries dirty (+0x160 = 1).
// Pool fix 2026-04-27: ahora itera por índice acotado (1002 slots).
void __cdecl Effect_UpdateAll(void) {
    char *pcVar1 = DAT_07c85890;
    for (int i = 0; i < 1002; ++i, pcVar1 += 0x1bc) {
        if (*pcVar1 != '\0')
            pcVar1[0x160] = '\x01';
    }
}


// Particle_Spawn (5-arg legacy alias) — delegates to FUN_00475220 (Effect_Spawn).
// Skills.cpp callers pass (type, x, y, z, flags). The real spawner takes
// (type, bone_mat, pos, size, flag, alpha, mode) — we synthesize a position
// vec3 from x/y/z and pass NULL for the optional bone_mat / size with default
// alpha = 1.0 and mode = 0.
void __cdecl Particle_Spawn(int type, float x, float y, float z, int flags) {
    float pos[3]   = { x, y, z };
    float color[3] = { 1.0f, 1.0f, 1.0f };
    FUN_00475220(type, /*bone_mat*/nullptr, pos, color, flags, 1.0f, 0);
}

