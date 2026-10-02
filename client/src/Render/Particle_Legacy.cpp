// Particle_Legacy.cpp

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
// Itera por índice acotado (1002 slots).
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
// Itera por índice acotado (1002 slots).
void __cdecl Effect_UpdateAll(void) {
    char *pcVar1 = DAT_07c85890;
    for (int i = 0; i < 1002; ++i, pcVar1 += 0x1bc) {
        if (*pcVar1 != '\0')
            pcVar1[0x160] = '\x01';
    }
}


