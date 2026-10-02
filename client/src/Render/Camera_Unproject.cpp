// Camera_Unproject.cpp
// Rayo del mouse en coordenadas de mundo (Camera_BuildMouseRay).

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// FUN_005112F0 @ 0x005112F0 — Screen_UnprojectRay(screenX, screenY, outRay).
// Converts screen pixel (param_1, param_2) to a world-space ray direction for mouse picking.
// Uses viewport (DAT_0056156C=W, DAT_00561570=H), projection offsets (DAT_083A429C/A0),
// projection scale (DAT_083A42A4/A8), z-depth DAT_00561550, and view matrix DAT_083A4140.
// FUN_005112F0
void __cdecl Camera_BuildMouseRay(int param_1, int param_2, float *param_3) {
    // El original usa locals contiguas en stack (local_18/14/10 es un vec3,
    // local_c/8/4 es otro). Ghidra los declaró como floats separados, y el
    // compilador C++ los puede reubicar en cualquier orden, así que tienen que ser
    // arrays reales: Vector_InverseRotate lee/escribe 3 floats secuenciales.
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
    Vector_InverseRotate(cam_fwd_neg, (float*)&CameraMatrix, (float*)&CameraRayOriginX);
    // Step 2: transform view-space direction by view rotation → world-space direction
    float world_dir[3];
    Vector_InverseRotate(view_dir,    (float*)&CameraMatrix, world_dir);

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
// SetErrorMessage — implemented in src/Render/GL_State.cpp
// UI_InGameMenu — implemented in src/UI/UI_InGameMenu.cpp (UI_InGameMenu state machine)

