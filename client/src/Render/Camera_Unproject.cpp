// Camera_Unproject.cpp
// Extracted from stubs_mouse_hover.cpp; IDA provenance comments retained.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// FUN_005112F0 @ 0x005112F0 — Screen_UnprojectRay(screenX, screenY, outRay).
// Converts screen pixel (param_1, param_2) to a world-space ray direction for mouse picking.
// Uses viewport (DAT_0056156C=W, DAT_00561570=H), projection offsets (DAT_083A429C/A0),
// projection scale (DAT_083A42A4/A8), z-depth DAT_00561550, and view matrix DAT_083A4140.
// FUN_005112F0
void __cdecl Camera_BuildMouseRay(int param_1, int param_2, float *param_3) {
    // BUG-FIX 2026-04-26 (deeper audit): el original usaba locals contiguas
    // en stack (local_18/14/10 era un vec3, local_c/8/4 era otro). El port
    // Ghidra los declaró como floats separados — el compilador C++ los puede
    // reubicar en CUALQUIER orden o slot, así que `&local_18` NO apuntaba a
    // un vec3 contiguo. FUN_004fa110 leía/escribía 3 floats secuenciales
    // desde esa dirección, leyendo basura y stompeando otros locals.
    // Síntoma: CameraRayOriginX (camera pos) y el endpoint del ray quedaban en
    // valores de miles de millones, hit-test contra entidades nunca pasaba.
    // Logueado en HT slot=N rayO=(-79771616,...) rayT=(779717248,...).
    float view_dir[3];
    view_dir[0] =  (float)(int)((UINT)(DAT_0056156c * param_1) / 0x280 - ViewportCenterX)
                 * _DAT_083a42a4 * Ff(DAT_00561550);
    view_dir[1] = -((float)(int)((UINT)(DAT_00561570 * param_2) / 0x1e0 - ViewportCenterY)
                 * _DAT_083a42a8 * Ff(DAT_00561550));
    view_dir[2] = -Ff(DAT_00561550);

    float cam_fwd_neg[3] = {
        -_DAT_083a414c,
        -_DAT_083a415c,
        -_DAT_083a416c
    };

    // Step 1: transform negated view-translation by view rotation → camera world pos
    FUN_004fa110(cam_fwd_neg, (float*)&DAT_083a4140, (float*)&CameraRayOriginX);
    // Step 2: transform view-space direction by view rotation → world-space direction
    float world_dir[3];
    FUN_004fa110(view_dir,    (float*)&DAT_083a4140, world_dir);

    // Endpoint = camera position + world-space direction
    param_3[0] = _CameraRayOriginX + world_dir[0];
    param_3[1] = _CameraRayOriginY + world_dir[1];
    param_3[2] = _CameraRayOriginZ + world_dir[2];
}
// Camera_ProjectWorldToScreen — implemented in src/Render/Camera.cpp
// GL_BindTextureSlot — implemented in src/Render/GL_State.cpp
// GL_ResetState — implemented in src/Render/GL_State.cpp
// GL_SetBlendSrcOver — implemented in src/Render/GL_State.cpp
// GL_SetBlendAdditive — implemented in src/Render/GL_State.cpp
// GL_BeginViewport — implemented in src/Render/Camera.cpp
// GL_DrawBillboard — implemented in src/Render/GL_2D.cpp
// GL_BeginSprite — implemented in src/Render/GL_State.cpp
// GL_Begin2D — implemented in src/Render/GL_2D.cpp
// GL_End2D — implemented in src/Render/GL_2D.cpp
// GL_DrawRect — implemented in src/Render/GL_2D.cpp
// FUN_005142d0 — implemented in src/Render/GL_State.cpp
// FUN_00514310 — implemented in src/UI/UI_InGameMenu.cpp (UI_InGameMenu state machine)

