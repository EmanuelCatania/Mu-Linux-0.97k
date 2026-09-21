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
// calls Render_DrawSprite to draw it, then clears the active flag.
// Pool fix 2026-04-27: AUTO-SKIP previo bloqueaba TODOS los efectos (glow +9,
// wing FX, etc.). Ahora itera por índice acotado (1002 slots).
void __cdecl Character_UpdateAll(void) {
    char *pcVar2 = DAT_07c85890;
    for (int i = 0; i < 1002; ++i, pcVar2 += 0x1bc) {
        if (*pcVar2 != '\0') {
            int blend = *(int*)(pcVar2 + 4);
            if      (blend == 0) GL_SetBlendAdditive();
            else if (blend == 1) GL_SetBlendSrcAlpha();
            else if (blend == 2) GL_SetBlendSrcOver('\x01');
            Render_DrawSprite((int)pcVar2);
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


// 2026-09-21: aca vivia un wrapper de 5 argumentos
// `Particle_Spawn(type, x, y, z, flags)` que delegaba en el real pasando
// `nullptr` como Position.  Particle_Spawn hace `*param_2` sin guard, asi que
// cualquier llamada habria sido una lectura de la direccion 0.  No tenia
// callers (Combat/Skills.cpp solo lo declaraba), o sea era una trampa armada:
// la misma familia que ya causo dos crashes, en RenderBoids y en la caida de
// la puerta de Blood Castle.  Se borra en vez de ponerle un guard, porque el
// binario tampoco lo tiene: ahi Position nunca llega en NULL.

