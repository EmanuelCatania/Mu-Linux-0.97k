// Camera_Unproject.cpp
// Extracted from stubs_mouse_hover.cpp; IDA provenance comments retained.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// FUN_005112F0 @ 0x005112F0 — Screen_UnprojectRay(screenX, screenY, outRay).
// Converts screen pixel (param_1, param_2) to a world-space ray direction for mouse picking.
// Uses viewport (DAT_0056156C=W, DAT_00561570=H), projection offsets (DAT_083A429C/A0),
// projection scale (DAT_083A42A4/A8), z-depth DAT_00561550, and view matrix DAT_083A4140.
void __cdecl FUN_005112f0(int param_1, int param_2, float *param_3) {
    // BUG-FIX 2026-04-26 (deeper audit): el original usaba locals contiguas
    // en stack (local_18/14/10 era un vec3, local_c/8/4 era otro). El port
    // Ghidra los declaró como floats separados — el compilador C++ los puede
    // reubicar en CUALQUIER orden o slot, así que `&local_18` NO apuntaba a
    // un vec3 contiguo. FUN_004fa110 leía/escribía 3 floats secuenciales
    // desde esa dirección, leyendo basura y stompeando otros locals.
    // Síntoma: DAT_083a4284 (camera pos) y el endpoint del ray quedaban en
    // valores de miles de millones, hit-test contra entidades nunca pasaba.
    // Logueado en HT slot=N rayO=(-79771616,...) rayT=(779717248,...).
    float view_dir[3];
    view_dir[0] =  (float)(int)((UINT)(DAT_0056156c * param_1) / 0x280 - DAT_083a429c)
                 * _DAT_083a42a4 * Ff(DAT_00561550);
    view_dir[1] = -((float)(int)((UINT)(DAT_00561570 * param_2) / 0x1e0 - DAT_083a42a0)
                 * _DAT_083a42a8 * Ff(DAT_00561550));
    view_dir[2] = -Ff(DAT_00561550);

    float cam_fwd_neg[3] = {
        -_DAT_083a414c,
        -_DAT_083a415c,
        -_DAT_083a416c
    };

    // Step 1: transform negated view-translation by view rotation → camera world pos
    FUN_004fa110(cam_fwd_neg, (float*)&DAT_083a4140, (float*)&DAT_083a4284);
    // Step 2: transform view-space direction by view rotation → world-space direction
    float world_dir[3];
    FUN_004fa110(view_dir,    (float*)&DAT_083a4140, world_dir);

    // Endpoint = camera position + world-space direction
    param_3[0] = _DAT_083a4284 + world_dir[0];
    param_3[1] = _DAT_083a4288 + world_dir[1];
    param_3[2] = _DAT_083a428c + world_dir[2];
}
// FUN_005113f0 — implemented in src/Render/Camera.cpp
// FUN_00511480 — implemented in src/Render/GL_State.cpp
// FUN_00511600 — implemented in src/Render/GL_State.cpp
// FUN_00511680 — implemented in src/Render/GL_State.cpp
// FUN_00511710 — implemented in src/Render/GL_State.cpp
// FUN_005119b0 — implemented in src/Render/Camera.cpp
// FUN_00511c10 — implemented in src/Render/GL_2D.cpp
// FUN_00511cf0 — implemented in src/Render/GL_State.cpp
// FUN_005123c0 — implemented in src/Render/GL_2D.cpp
// FUN_005124b0 — implemented in src/Render/GL_2D.cpp
// FUN_005124c0 — implemented in src/Render/GL_2D.cpp
// FUN_005142d0 — implemented in src/Render/GL_State.cpp
// FUN_00514310 — implemented in src/UI/UI_InGameMenu.cpp (UI_InGameMenu state machine)

