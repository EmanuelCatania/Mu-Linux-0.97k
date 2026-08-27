// Scene_Loading.cpp
// Scene_Loading @ 0x00524B20  (34 lines, decompile completo)
//
// Carga 3 JPGs de loading, los dibuja side-by-side, SwapBuffers,
// setea g_GameState=5 (InGame), descarga texturas, llama CharList_Init(1).
//
// ── DECOMPILE COMPLETO ────────────────────────────────────────────────────────
//
//   void __cdecl Scene_Loading(HDC param_1)
//   {
//     if (DAT_083a7c4a == '\0') {
//       DAT_083a7c4a = '\x01';
//       FUN_00529740("Local/Loading01.jpg", 0x1e, 0x2600, 0x2900, 0, '\x01');
//       FUN_00529740("Local/Loading02.jpg", 0x1f, 0x2600, 0x2900, 0, '\x01');
//       FUN_00529740("Local/Loading03.jpg", 0x20, 0x2600, 0x2900, 0, '\x01');
//     }
//     DAT_083a42ea = 0;
//     GL_BeginViewport(0, 0, 0x280, 0x1e0);
//     glClear(0x4100);
//     GL_Begin2D();
//     glColor3f(1.0, 1.0, 1.0);
//     GL_DrawTexture(0x1e,   0.0, 100.0, 256.0, 256.0, 0.0, 0.0, 1.0, 1.0, '\x01', '\x01');
//     GL_DrawTexture(0x1f, 256.0, 100.0, 128.0, 256.0, 0.0, 0.0, 1.0, 1.0, '\x01', '\x01');
//     GL_DrawTexture(0x20, 384.0, 100.0, 256.0, 256.0, 0.0, 0.0, 1.0, 1.0, '\x01', '\x01');
//     GL_End2D();
//     GL_PopMatrixAll();
//     glFlush();
//     SwapBuffers(param_1);
//     DAT_005615c0 = 5;          // g_GameState = InGame
//     int iVar1 = 0;
//     do {
//       FUN_0052a050(iVar1 + 0x1e);   // Texture_Unload(0x1e..0x20)
//       iVar1++;
//     } while (iVar1 < 3);
//     FUN_0047ec60(1);
//   }
//
// ── TEXTURAS ──────────────────────────────────────────────────────────────────
//
//   0x1e  → "Local/Loading01.jpg"  posición (0,   100) tamaño 256×256
//   0x1f  → "Local/Loading02.jpg"  posición (256, 100) tamaño 128×256
//   0x20  → "Local/Loading03.jpg"  posición (384, 100) tamaño 256×256
//
// ── GLOBALS ───────────────────────────────────────────────────────────────────
//
//   DAT_083a7c4a  — init flag (carga solo una vez)
//   DAT_083a42ea  — reset a 0 cada frame
//   DAT_005615c0  — g_GameState → seteado a 5 (InGame) al final
//
// ── FUNCIÓN CROSS-REFERENCE ───────────────────────────────────────────────────
//
//   FUN_00529740  → Texture_Load(path, id, w, h, flag, mipmap)
//   GL_BeginViewport  → Viewport_Set(x, y, w, h)
//   GL_Begin2D  → GL_SetupOrtho2D()
//   GL_DrawTexture  → Texture_Draw2D(id, x, y, w, h, u0, v0, u1, v1, fx, fy)
//   GL_End2D  → GL_End2D()
//   FUN_0052a050  → Texture_Unload(id)
//   FUN_0047ec60  → CharList_Init(mode)

#include "stdafx.h"
#include "Scene/Scene_Loading.h"

// Scene_Loading @ 0x00524B20 (34 lines)
// Loading screen: loads 3 JPGs, draws them tiled, swaps, then transitions to InGame.
void __cdecl Scene_Loading(HDC param_1)
{
    int iVar1;

    // Load textures once (guarded by init flag)
    if (DAT_083a7c4a == '\0') {
        DAT_083a7c4a = '\x01';
        FUN_00529740(s_Local_Loading01_jpg_00561a88, 0x1e, 0x2600, 0x2900, 0, '\x01');
        FUN_00529740(s_Local_Loading02_jpg_00561a9c, 0x1f, 0x2600, 0x2900, 0, '\x01');
        FUN_00529740(s_Local_Loading03_jpg_00561ab0, 0x20, 0x2600, 0x2900, 0, '\x01');
    }

    DAT_083a42ea = 0;
    GL_BeginViewport(0, 0, 0x280, 0x1e0);   // Viewport_Set(0,0,640,480)
    glClear(0x4100);
    GL_Begin2D();                       // GL_SetupOrtho2D
    glColor3f(1.0f, 1.0f, 1.0f);

    // Draw 3 loading tiles: 256×256 + 128×256 + 256×256 at y=100
    GL_DrawTexture(0x1e,   0.0f, 100.0f, 256.0f, 256.0f, 0.0f, 0.0f, 1.0f, 1.0f, '\x01', '\x01');
    GL_DrawTexture(0x1f, 256.0f, 100.0f, 128.0f, 256.0f, 0.0f, 0.0f, 1.0f, 1.0f, '\x01', '\x01');
    GL_DrawTexture(0x20, 384.0f, 100.0f, 256.0f, 256.0f, 0.0f, 0.0f, 1.0f, 1.0f, '\x01', '\x01');

    GL_End2D();    // GL_End2D
    GL_PopMatrixAll();
    glFlush();
    SwapBuffers(param_1);

    // Transition to InGame
    DAT_005615c0 = 5;

    // Unload loading textures
    iVar1 = 0;
    do {
        FUN_0052a050(iVar1 + 0x1e);
        iVar1++;
    } while (iVar1 < 3);

    FUN_0047ec60(1);   // CharList_Init(1)
}
