// Scene_Login.cpp
// Scene_Login @ 0x00521630  (port directo desde IDA Hex-Rays)
//
// Renderiza el fondo 3D + UI 2D de la pantalla de login.
// Delega a Scene_Login_ServerSelect cuando sub-estado == 0.
//
// ── SIGNATURA ─────────────────────────────────────────────────────────────────
//
//   bool __cdecl Scene_Login(HDC hDC)
//
// ── MAPEO FUN_XXXX → nombre real IDA ──────────────────────────────────────────
//   GL_BeginViewport → BeginOpengl(x,y,w,h)          — setup 3D: push PROJ+MV, persp, rota cam
//   FUN_004fd800 → Terrain_Render()
//   FUN_0045ab00 → Entity_RenderAll_3D()
//   FUN_00500970 → RenderBugs()                  — (no era Entity_Render_Sprites)
//   FUN_0046c3e0 → Trail_RenderAll()             — (no era Particle_Render)
//   GL_BeginSprite → BeginSprite()                 — sólo push MV + loadIdentity
//   FUN_00479730 → RenderSprites()               — (no era Portal_Render)
//   FUN_00478c00 → RenderParticles()             — (no era ItemDrop_Render_2)
//   GL_Begin2D → BeginBitmap()                 — setup 2D ortho
//   GL_SetBlendSrcOver → EnableAlphaTest(flag)         — (no era GL_SetMode)
//   GL_DrawTexture → RenderBitmap(id,x,y,w,h,u0,v0,u1,v1,fx,fy)
//   UI_RenderText → RenderText
//   UI_RenderInputField → RenderInputText
//   GL_SetBlendAdditive → EnableAlphaBlend()            — (no era Frame_UpdateTimer)
//   GL_ResetState → DisableAlphaBlend()           — (no era GL_ResetBlend)
//   FUN_0051af50 → RenderErrorMessage()          — (no era Chat_Render)
//   FUN_004f64d0 → Scene_MapTick()               — (no era UI_Render)
//   UI_RenderNotices → RenderNotices()               — (no era StatusBar_Render)
//   UI_RenderChatLogOverlay → sub_480980 (chat log render)  — DAT_005590ac=g_bUseChatListBox
//   FUN_004c14e0 → RenderDebugWindow()
//   FUN_004c3530 → RenderHelpWindow()
//   FUN_004bffa0 → RenderCursor()                — (no era Minimap_Render)
//   FUN_0051e0c0 → RenderInfomation3D()          — (no era Cursor_Render)
//   GL_End2D → EndBitmap()                   — 2x glPopMatrix (balancea BeginBitmap+BeginSprite)
//   GL_EndOpenGL → EndOpengl()                   — pop MV + pop PROJ (balancea BeginOpengl)
//
// ── GLOBALS ───────────────────────────────────────────────────────────────────
//   DAT_083a7c49   — InitLogIn (guard)
//   DAT_083a7c14   — login sub-state
//   DAT_083a42ea   — FogEnable
//   DAT_07e11d6e   — byte flag reset
//   DAT_00559c8c   — InputTextWidth
//   DAT_00559c78   — m_dwTextColor
//   DAT_00559c80   — m_dwBackColor
//   DAT_055c9fec   — m_hFontDC (HDC usado para GetTextExtentPointA)
//   DAT_055ca00c   — g_hFont
//   DAT_005616a4   — base Y panel credenciales
//   DAT_083a427c/4278 — MouseX/MouseY
//   DAT_083a7ac8   — tick base progress bar
//   DAT_083a7af4   — fade state
//   DAT_005615e8   — fade counter
//   DAT_0056156c   — WindowWidth
//   DAT_005590ac   — g_bUseChatListBox
//   DAT_005615c0   — g_GameState
//   DAT_083a4320   — retry counter

#include "stdafx.h"
#include "Scene/Scene_Login.h"
#include "Scene/Scene_Login_ServerSelect.h"

extern "C" { void DbgLogPublic(const char*); }

// Scene_Login @ 0x00521630
uint Scene_Login(void)
{
    char     cVar1;
    int      iVar2, iVar3;
    DWORD    DVar4;
    uint     uVar6, uVar7;
    char    *pcVar8, *pcVar9;
    tagSIZE *ptVar10;
    tagSIZE  tStack_74, tStack_6c;
    char     acStack_64[100];

    if (DAT_083a7c49 == '\0')
        return 0;

    DAT_083a42ea = 0;
    DAT_07e11d6e = 0;
    glClearColor(0, 0, 0, 1.0f);
    GL_BeginViewport(0, 0x50, 0x280, 0x140);   // BeginOpengl(0, 80, 640, 320)

    // ── 3D background render (orden exacto de IDA) ───────────────────────────
    FUN_004fd800();    // Terrain_Render
    FUN_0045ab00();    // Entity_RenderAll_3D
    FUN_00500970();    // RenderBugs
    FUN_0046c3e0();    // Trail_RenderAll
    GL_BeginSprite();    // BeginSprite — push MV, loadIdentity
    FUN_00479730();    // RenderSprites
    FUN_00478c00();    // RenderParticles
    glPopMatrix();     // balancea BeginSprite
    GL_Begin2D();    // BeginBitmap — setup 2D ortho

    // ── Font / color setup ────────────────────────────────────────────────────
    DAT_00559c8c = 0x77;                  // InputTextWidth = 119
    GL_SetBlendSrcOver('\x01');                 // EnableAlphaTest(1)
    glColor3f(1.0f, 1.0f, 1.0f);
    DAT_00559c78 = 0xffd2e6ff;            // m_dwTextColor
    DAT_00559c80 = 0;                     // m_dwBackColor
    SelectObject((HDC)(uintptr_t)DAT_055c9fec, (HGDIOBJ)(uintptr_t)DAT_055ca00c);

    // ── Credential input panel (sub-states 2..3) ─────────────────────────────
    iVar3 = DAT_005616a4;
    if ((1 < DAT_083a7c14) && (DAT_083a7c14 < 4)) {
        tStack_74.cx = DAT_005616a4;
        GL_DrawTexture(0xc, 195.0f, (float)DAT_005616a4, 250.0f, 216.0f,
                     0.0f, 0.0f, 0.9765625f, 0.84375f, '\x01', '\x01');
        // Username field
        UI_RenderText(0xe3, iVar3 + 0x32, (LPCSTR)DAT_07d4ac7c, (LPSIZE)0x0, '\0', 0);
        UI_RenderInputField(0x127, iVar3 + 0x32, 0);
        // Password field
        UI_RenderText(0xe3, iVar3 + 0x48, (LPCSTR)DAT_07d4ada8, (LPSIZE)0x0, '\0', 0);
        UI_RenderInputField(0x127, iVar3 + 0x48, 1);

        // OK button hit-test
        tStack_74.cx = iVar3 + 0x5f;
        if ((0x142 < DAT_083a427c) &&
            ((DAT_083a427c < 0x189) && (tStack_74.cx <= DAT_083a4278)) &&
            (DAT_083a4278 < iVar3 + 0x72))
        {
            GL_DrawTexture(0xd, 323.0f, (float)tStack_74.cx, 70.0f, 19.0f,
                         0.0f, 0.0f, 0.546875f, 0.59375f, '\x01', '\x01');
        }
        ptVar10 = &tStack_74;
        iVar2 = lstrlenA((LPCSTR)lpString_07d4aed4);
        GetTextExtentPointA(DAT_055c9fec, (LPCSTR)lpString_07d4aed4, iVar2, ptVar10);
        UI_RenderText(0x165 - ((uint)(tStack_74.cx * 0x280) / DAT_0056156c >> 1),
                     iVar3 + 100, (LPCSTR)lpString_07d4aed4, (LPSIZE)0x0, '\0', 0);

        // Exit/Cancel button hit-test
        tStack_74.cx = iVar3 + 0xb4;
        if (((0x11c < DAT_083a427c) && (DAT_083a427c < 0x163)) &&
            ((tStack_74.cx <= DAT_083a4278) && (DAT_083a4278 < iVar3 + 200)))
        {
            GL_DrawTexture(0xe, 285.0f, (float)tStack_74.cx, 70.0f, 20.0f,
                         0.0f, 0.0f, 0.546875f, 0.625f, '\x01', '\x01');
        }
        ptVar10 = &tStack_74;
        iVar2 = lstrlenA((LPCSTR)lpString_07d4b000);
        GetTextExtentPointA(DAT_055c9fec, (LPCSTR)lpString_07d4b000, iVar2, ptVar10);
        UI_RenderText(0x13f - ((uint)(tStack_74.cx * 0x280) / DAT_0056156c >> 1),
                     iVar3 + 0xb9, (LPCSTR)lpString_07d4b000, (LPSIZE)0x0, '\0', 0);
    }

    // ── Version strings (bottom of screen) ───────────────────────────────────
    DAT_00559c8c = 0x100;                 // InputTextWidth = 256
    glColor3f(0.8f, 0.7f, 0.6f);
    uVar6 = 0xffffffff;
    DAT_00559c78 = 0xffffffff;

    // Version string 1 (memcpy loop generado por MSVC)
    pcVar8 = DAT_07d4b12c;
    do {
        pcVar9 = pcVar8;
        if (uVar6 == 0) break;
        uVar6--;
        pcVar9 = pcVar8 + 1;
        cVar1  = *pcVar8;
        pcVar8 = pcVar9;
    } while (cVar1 != '\0');
    uVar6 = ~uVar6;
    pcVar8 = pcVar9 - uVar6;
    pcVar9 = acStack_64;
    for (uVar7 = uVar6 >> 2; uVar7 != 0; uVar7--) {
        *(undefined4 *)pcVar9 = *(undefined4 *)pcVar8;
        pcVar8 += 4; pcVar9 += 4;
    }
    DAT_00559c80 = 0x80000000;
    for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6--) {
        *pcVar9 = *pcVar8; pcVar8++; pcVar9++;
    }
    ptVar10 = &tStack_6c;
    iVar3   = lstrlenA(acStack_64);
    GetTextExtentPointA(DAT_055c9fec, acStack_64, iVar3, ptVar10);
    UI_RenderText(0x14f - (uint)(tStack_6c.cx * 0x280) / DAT_0056156c,
                 0x1df - (uint)(tStack_6c.cy * 0x280) / DAT_0056156c,
                 acStack_64, (LPSIZE)0x0, '\0', 0);

    // Version string 2
    uVar6  = 0xffffffff;
    pcVar8 = DAT_07d4b258;
    do {
        pcVar9 = pcVar8;
        if (uVar6 == 0) break;
        uVar6--;
        pcVar9 = pcVar8 + 1;
        cVar1  = *pcVar8;
        pcVar8 = pcVar9;
    } while (cVar1 != '\0');
    uVar6 = ~uVar6;
    pcVar8 = pcVar9 - uVar6;
    pcVar9 = acStack_64;
    for (uVar7 = uVar6 >> 2; uVar7 != 0; uVar7--) {
        *(undefined4 *)pcVar9 = *(undefined4 *)pcVar8;
        pcVar8 += 4; pcVar9 += 4;
    }
    for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6--) {
        *pcVar9 = *pcVar8; pcVar8++; pcVar9++;
    }
    ptVar10 = &tStack_6c;
    iVar3   = lstrlenA(acStack_64);
    GetTextExtentPointA(DAT_055c9fec, acStack_64, iVar3, ptVar10);
    UI_RenderText(0, 0x1df - (uint)(tStack_6c.cy * 0x280) / DAT_0056156c,
                 acStack_64, (LPSIZE)0x0, '\0', 0);

    // Footer center text (copyright)
    crt_sprintf(acStack_64, DAT_07d4b384);
    ptVar10 = &tStack_6c;
    iVar3   = lstrlenA(acStack_64);
    GetTextExtentPointA(DAT_055c9fec, acStack_64, iVar3, ptVar10);
    UI_RenderText(0x14f, 0x1df - (uint)(tStack_6c.cy * 0x280) / DAT_0056156c,
                 acStack_64, (LPSIZE)0x0, '\0', 0);

    // ── Sub-state dispatch ────────────────────────────────────────────────────
    if (DAT_083a7c14 == 0)
        Scene_Login_ServerSelect();

    if ((DAT_083a7c14 == 1) || (DAT_083a7c14 == 0xc)) {
        // Connecting / Timeout: progress bar + cancel button
        glColor3f(0.5f, 0.5f, 0.5f);
        GL_DrawTexture(0xf0, 213.0f, 228.0f, 213.0f, 64.0f,
                     0.0f, 0.0f, 0.83203125f, 1.0f, '\x01', '\x01');
        glColor3f(1.0f, 1.0f, 1.0f);
        GL_SetBlendAdditive();   // EnableAlphaBlend

        // Progress bar track
        GL_DrawTexture(0x13, 220.0f, 240.0f, 200.0f, 16.0f,
                     0.0f, 0.0f, 0.78125f, 1.0f, '\x01', '\x01');

        // Progress bar fill
        DVar4 = GetTickCount();
        if (DVar4 - DAT_083a7ac8 < 15000) {
            DVar4 = GetTickCount();
            iVar3 = DVar4 - DAT_083a7ac8;
        } else {
            iVar3 = 15000;
        }
        tStack_74.cx = (LONG)(float)(((uint)(iVar3 * 200) / 75000) * 5);
        GL_DrawTexture(0x14, 220.0f, 240.0f, (float)tStack_74.cx, 16.0f,
                     0.0f, 0.0f, (float)tStack_74.cx * _DAT_00552b7c, 1.0f, '\x01', '\x01');
        GL_ResetState();   // DisableAlphaBlend

        // Cancel button (0xf3=normal, 0xf4=highlighted)
        if (((((DAT_083a427c < 0x11d) || (0x162 < DAT_083a427c)) ||
              (DAT_083a4278 < 0x106)) || (0x119 < DAT_083a4278)) ||
            ((DAT_083a7c14 == 0xc) && (DAT_083a4320 < 4)))
        {
            iVar3 = 0xf3;
        } else {
            iVar3 = 0xf4;
        }
        GL_DrawTexture(iVar3, 285.0f, 262.0f, 70.0f, 20.0f,
                     0.0f, 0.0f, 0.546875f, 0.625f, '\x01', '\x01');

        GL_SetBlendSrcOver('\x01');
        ptVar10 = &tStack_74;
        DAT_00559c80 = 0x80000000;
        // IDA 0x00521630 L134-136: these reference GlobalText[471] (localized
        // "Conectando..." status string), not the standalone empty buffer
        // lpString_07d4c518 that Ghidra emitted.
        iVar3 = lstrlenA((LPCSTR)GlobalText[471]);
        GetTextExtentPointA(DAT_055c9fec, (LPCSTR)GlobalText[471], iVar3, ptVar10);
        UI_RenderText(0x140 - ((uint)(tStack_74.cx * 0x280) / DAT_0056156c >> 1), 0xd0,
                     (LPCSTR)GlobalText[471], (LPSIZE)0x0, '\0', 0);
    }

    // ── Fade-in effect ────────────────────────────────────────────────────────
    if (DAT_083a7af4 == 1) {
        if (DAT_005615e8 < 0x10) {
            tStack_74.cx = (LONG)((float)(0x10 - DAT_005615e8) * _DAT_005526dc);
            glColor3f((float)tStack_74.cx,
                      (float)tStack_74.cx * _DAT_005526e8,
                      (float)tStack_74.cx * _DAT_00552530);
        } else {
            DAT_083a7af4 = 2;
        }
    }

    // ── Final subsystems (nombres corregidos desde IDA) ──────────────────────
    FUN_0051af50();    // RenderErrorMessage
    FUN_004f64d0();    // Scene_MapTick
    UI_RenderNotices();    // RenderNotices
    if ((DAT_005590ac == 1) || (DAT_005615c0 != 5))
        UI_RenderChatLogOverlay();   // sub_480980 — chat log render
    FUN_004c14e0();    // RenderDebugWindow
    FUN_004c3530();    // RenderHelpWindow
    FUN_004bffa0();    // RenderCursor
    FUN_0051e0c0();    // RenderInfomation3D

    // ── Teardown (port exacto de IDA: EndBitmap + EndOpengl) ─────────────────
    GL_End2D();    // EndBitmap  — 2x glPopMatrix
    GL_EndOpenGL();    // EndOpengl  — pop MV + pop PROJ
    return 1;
}
