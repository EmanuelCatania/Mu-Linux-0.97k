// Cursor_Render.cpp
// Extracted from stubs_mouse_hover.cpp; IDA provenance comments retained.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// FUN_004BFFA0 @ 0x004BFFA0 — Cursor_Render.
// Draws the in-game mouse cursor sprite. Sprite ID selected by:
//   game_substate, hovered entity type, cursor-mode flags (DAT_00559C48/4C/50/54).
// Uses FUN_005125A0(sprite_id, x, y, 24, 24, u, v, 1, 1) for fixed sprites,
// or FUN_005126E0 for animated/colored variants.
// Cursor offset = _DAT_0055264C from mouse pos (DAT_083A427C/78).
// Reescrito 1:1 con IDA `RenderCursor` (004BFFA0_RenderCursor.c, 152 líneas).
// Decisión de sprite por prioridad:
//   SelectedItem       → bitmap 5   (item ground)
//   SelectedNpc        → bitmap 6   (NPC, animación 3×2 uv via u/v)
//   SelectedOperate    → bitmap 8 (world match) / 9 (genérico)
//   !Hero.dead && SelectedCharacter:
//       CheckAttack && !MouseOnWindow → bitmap 4 (attack target)
//       else                          → bitmap 2 (arrow, LABEL_43)
//   RepairEnable == 1  → bitmap 7
//   RepairEnable == 2  → bitmap 7 (animado, sin(WorldTime*0.02))
//   !MouseLButton      → bitmap 2 (arrow, LABEL_43)
//   MouseLButton && DontMove  → bitmap 10
//   MouseLButton && !DontMove → bitmap 3 (move)
//
// NULL-guard sobre Hero (DAT_07abf5d8): en el original el crash acá era
// imposible porque SelectedCharacter=-1 en login y Hero siempre apuntaba a
// una entidad válida in-game; acá Hero=NULL en login si aún no se asignó.
void __cdecl FUN_004bffa0(void) {
    GL_SetBlendSrcOver('\x01');  // EnableAlphaTest(1)
    glColor3f(1.0f, 1.0f, 1.0f);

    int u_bits = 0, v_bits = 0;
    // Frame = (int64)(WorldTime * 0.01) % 6  — IDA lo emite con __int64 cast explícito.
    int frame = (int)((long long)((double)(int)DAT_05826e08 * 0.0099999998)) % 6;
    if (frame == 1 || frame == 3 || frame == 5) u_bits = 0x3F000000;  // 0.5f
    if (frame == 2 || frame == 3 || frame == 4) v_bits = 0x3F000000;  // 0.5f

    float cx = (float)(int)DAT_083a427c - _DAT_0055264c;  // MouseX - 2
    float cy = (float)(int)DAT_083a4278 - _DAT_0055264c;  // MouseY - 2

    // ── LABEL_43: default arrow cursor ───────────────────────────────────────
    auto draw_arrow = [&](){
        GL_DrawTexture(2, cx, cy, 24.0f, 24.0f, 0.0f, 0.0f, 1.0f, 1.0f, '\x01', '\x01');
    };

    // Error message o diálogo bloqueante → arrow
    if (DAT_07eaa14c != 0 || DAT_083a7c24 != 0) { draw_arrow(); return; }

    // SelectedItem (hover sobre ítem en el piso)
    if (SelectedItem != -1) {
        GL_DrawTexture(5, cx, cy, 24.0f, 24.0f, 0.0f, 0.0f, 1.0f, 1.0f, '\x01', '\x01');
        return;
    }
    // SelectedNpc (hover sobre NPC) — UV animado 3×2
    if (SelectedNpc != -1) {
        GL_DrawTexture(6, cx, cy, 24.0f, 24.0f,
                     *(float*)&u_bits, *(float*)&v_bits, 0.5f, 0.5f, '\x01', '\x01');
        return;
    }
    // SelectedOperate (hover sobre objeto interactivo del mundo)
    if (SelectedOperate != -1) {
        // Match per-World contra el type-code de la entidad; fallback bitmap 9.
        // DAT_0055a7ac aquí actúa como `World` en IDA; puede no coincidir 100%
        // con nuestra interpretación de sub-state pero no afecta el default.
        short cls = *(short*)(((int*)&DAT_083a2378)[SelectedOperate * 3] + 2);
        int world = DAT_0055a7ac;
        bool match = false;
        if      (world == 0) match = (cls == 133);
        else if (world == 1) match = (cls == 60);
        else if (world == 2) match = (cls == 91);
        else if (world == 3) match = (cls == 38);
        if (match) GL_DrawTexture(8, cx, cy, 24.0f, 24.0f, 0.0f, 0.0f, 1.0f, 1.0f, '\x01', '\x01');
        else       GL_DrawTexture(9, cx, cy, 24.0f, 24.0f, 0.0f, 0.0f, 1.0f, 1.0f, '\x01', '\x01');
        return;
    }
    // Hover sobre char vivo + SelectedCharacter → attack crosshair o arrow
    // NULL-guard: en login Hero=NULL y SelectedCharacter=-1, así que el branch
    // se saltea; el guard sólo protege contra misconfig.
    if (DAT_07abf5d8 != NULL &&
        *(char*)((char*)DAT_07abf5d8 + 0x34e) == '\0' &&
        SelectedCharacter != -1)
    {
        if ((char)FUN_00483160() != '\0' && DAT_07d78094 == '\0') {
            GL_DrawTexture(4, cx, cy, 24.0f, 24.0f, 0.0f, 0.0f, 1.0f, 1.0f, '\x01', '\x01');
        } else {
            draw_arrow();
        }
        return;
    }
    // Repair active (modo taladro del smith)
    if (DAT_07eaa134 == 1) {
        GL_DrawTexture(7, cx, cy, 24.0f, 24.0f, 0.0f, 0.0f, 1.0f, 1.0f,
                     DAT_07eaa134, DAT_07eaa134);
        return;
    }
    if (DAT_07eaa134 == 2) {
        float10 fv = fsin((float10)(int)DAT_05826e08 * (float10)_DAT_00552914);
        if (fv <= (float10)_DAT_005528a0) {
            FUN_005126e0(7, (float)(int)DAT_083a427c + _DAT_00552660,
                            (float)(int)DAT_083a4278 + _DAT_005529fc,
                         24.0f, 24.0f, 0x42340000);
        } else {
            FUN_005126e0(7, (float)(int)DAT_083a427c + _DAT_00552488,
                            (float)(int)DAT_083a4278 + _DAT_00552488,
                         24.0f, 24.0f, 0);
        }
        return;
    }
    // Sin LMB → arrow. Con LMB → move (3) / DontMove (10).
    if (DAT_083a42c4 == 0) { draw_arrow(); return; }
    if (DAT_07e11d64 != 0)
        GL_DrawTexture(10, cx, cy, 24.0f, 24.0f, 0.0f, 0.0f, 1.0f, 1.0f, '\x01', '\x01');
    else
        GL_DrawTexture(3,  cx, cy, 24.0f, 24.0f, 0.0f, 0.0f, 1.0f, 1.0f, '\x01', '\x01');
}
