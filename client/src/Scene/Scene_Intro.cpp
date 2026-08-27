// Scene_Intro.cpp
// Scene_Intro @ 0x005137A0  (230 lines, decompile completo)
//
// Pantalla de intro: logo Webzen + badge "Everyone". Al terminar:
//   DAT_083a410c == '\0' → g_GameState = 2 (Login, flujo normal)
//   DAT_083a410c != '\0' → g_GameState = 5 (InGame directo, modo bypass/debug)
//
// ── DECOMPILE COMPLETO ────────────────────────────────────────────────────────
//
//   void __cdecl Scene_Intro(HDC param_1)
//   {
//     // Reset array DAT_083a7cd0 (stride 0x38, hasta 0x83bba00)
//     puVar3 = (undefined1*)&DAT_083a7cd0;
//     do {
//       *(undefined4*)(puVar3 + 4) = 0;
//       *puVar3 = 0;
//       puVar3 += 0x38;
//     } while ((int)puVar3 < 0x83bba00);
//
//     FUN_0050f690();                          → World_Init()
//     Input_ClearState(1);                         → CharList_Init(1)
//
//     FUN_00529740("Local/Webzenlogo.jpg", 0xc, 0x2600, 0x2900, 0, '\x01');
//     FUN_00529740("Local/Everyone.jpg",   0xd, 0x2600, 0x2900, 0, '\x01');
//
//     DAT_083a42ea = 0;
//     GL_BeginViewport(0, 0, 0x280, 0x1e0);       // Viewport_Set(0,0,640,480)
//     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//     GL_Begin2D();                           // GL_SetupOrtho2D()
//     glColor3f(1.0, 1.0);
//
//     // Logo Webzen centrado (usa DAT_0056156c = screen_width)
//     GL_DrawTexture(0xc,
//       (float)DAT_0056156c * 0.5f - _DAT_005529bc,
//       (float)DAT_0056156c * _DAT_00552d3c - _DAT_00552d38,
//       256.0, 195.0, 0.0, 0.0, 1.0, 0.76171875, '\0', '\0');
//
//     // Badge "Everyone" (esquina derecha)
//     GL_DrawTexture(0xd,
//       (float)DAT_0056156c - _DAT_00552908,
//       0.0, 120.0, 60.0, 0.0, 0.0, 0.9375, 0.9375, '\0', '\0');
//
//     // Debug overlay (solo si bypass activo)
//     if (DAT_083a410c != '\0') {
//       SelectObject(DAT_055c9fec, DAT_055ca00c);
//       UI_RenderText(0, 0, &DAT_005617a0, NULL, '\0', 0);
//     }
//
//     GL_End2D();   // GL_End2D()
//     GL_PopMatrixAll();
//     glFlush();
//     SwapBuffers(param_1);
//
//     FUN_0052a050(0xd);   // Texture_Unload("Everyone.jpg")
//     FUN_0052a050(0xc);   // Texture_Unload("Webzenlogo.jpg")
//
//     // PATH NORMAL: ir a Login
//     if (DAT_083a410c == '\0') {
//       FUN_00405540(&DAT_055c9bf0, "> Loading ok...");
//       DAT_005615c0 = 2;   // g_GameState = Login
//       Scene_LoadGameAssets() (IDA: FUN_00510320);
//       return;
//     }
//
//     // PATH BYPASS: ir directo a InGame
//     DAT_005615c0 = 5;   // g_GameState = InGame
//     DAT_083a7c10 = 1;   // render enable flag
//     Scene_LoadGameAssets() (IDA: FUN_00510320);
//     FUN_0050e5a0();      // World_Load()
//     DAT_05826cac = 0;
//
//     // Spawn entidad local en posición hardcodeada
//     // FUN_0045f930(0, 0, 0, 5414.4f, 21981.5f)
//     //   → Entity_Create(type=0, ..., world_x=0x44a8c000, world_y=0x46a9ec00)
//     // + HashTable tracking (anti-tamper, omitido)
//   }
//
// ── LAYOUT TEXTURAS ───────────────────────────────────────────────────────────
//
//   0xc  → Webzenlogo.jpg  256×195 UV(0,0,1.0,0.762)  centrado en pantalla
//   0xd  → Everyone.jpg    120×60  UV(0,0,0.9375,0.9375) esquina superior derecha
//
// ── PATHS ─────────────────────────────────────────────────────────────────────
//
//   DAT_083a410c == '\0':  → Login (estado normal del juego)
//   DAT_083a410c != '\0':  → InGame directo (debug/autotest, bypasea login)
//                              + World_Load() + Entity_Create en pos hardcodeada
//
// ── GLOBALS ───────────────────────────────────────────────────────────────────
//
//   DAT_083a7cd0  — array reseteado al inicio (stride 0x38, hasta 0x83bba00)
//   DAT_083a410c  — bypass flag (0=normal, 1=bypass a InGame)
//   DAT_005615c0  — g_GameState
//   DAT_083a7c10  — render enable flag
//   DAT_05826cac  — counter reset
//   DAT_0056156c  — screen_width (para posicionamiento centrado)
//   DAT_005617a0  — debug string (mostrado si bypass activo)
//   _DAT_005529bc — offset X logo (centering adjust)
//   _DAT_00552d3c — factor Y logo
//   _DAT_00552d38 — offset Y logo
//   _DAT_00552908 — offset X "Everyone" badge
//
// ── FUNCIÓN CROSS-REFERENCE ───────────────────────────────────────────────────
//
//   FUN_0050f690  → World_Init()
//   Input_ClearState  → CharList_Init(mode)
//   FUN_00529740  → Texture_Load(path, id, w, h, flag, mipmap)
//   GL_BeginViewport  → Viewport_Set(x, y, w, h)
//   GL_Begin2D  → GL_SetupOrtho2D()
//   GL_DrawTexture  → Texture_Draw2D(id, x, y, w, h, u0, v0, u1, v1, fx, fy)
//   GL_End2D  → GL_End2D()
//   FUN_0052a050  → Texture_Unload(id)
//   Scene_LoadGameAssets (IDA: FUN_00510320) → shared asset loader
//   FUN_0050e5a0  → World_Load()
//   FUN_0045f930  → Entity_Create(type, ?, ?, world_x, world_y)
//   FUN_00405540  → Log(hashtable, msg)

#include "stdafx.h"
#include "Scene/Scene_Intro.h"

// Debug trace (defined in WinMain.cpp)
extern "C" { void DbgLogPublic(const char* msg); }
#define DBG(s) DbgLogPublic(s)

// Scene_Intro @ 0x005137A0
// Splash: Webzen logo + ESRB badge. Transitions to Login (normal) or InGame (bypass/debug).
void __cdecl Scene_Intro(HDC param_1)
{
    undefined4 *puVar1;
    char        cVar2;
    undefined1 *puVar3;
    uint        uVar4;
    void       *pvVar5;
    undefined4 *puVar6;
    int         iVar7, iVar8;
    byte       *pbVar9, *pbVar10, *pbVar11;
    bool        bVar12, bVar13;
    uint        uVar14;
    int         iStack_10;
    undefined4 *local_c;
    undefined1 *local_8;
    uint        uStack_4;
    DWORD       stack_ec;   // local hash-compare buffer (4 bytes at EBP-0x14)
    DWORD       stack_e8;   // local hash-compare buffer (4 bytes at EBP-0x18)

    DBG("Scene_Intro: memset bitmaps");
    // Reset Bitmaps table (stride 0x38, ~1414 entries = 0x13D30 bytes)
    puVar3 = (undefined1*)&DAT_083a7cd0;
    {
        // NOTE: loop bound must stay within g_BitmapsRaw[0x13D30]; starts at offset 0x30
        undefined1 *endPtr = (undefined1*)&g_BitmapsRaw[0] + 0x13D30;
        do {
            *(undefined4 *)(puVar3 + 4) = 0;
            *puVar3 = 0;
            puVar3 = puVar3 + 0x38;
        } while (puVar3 + 0x38 <= endPtr);
    }

    DBG("Scene_Intro: before Font_Init");
    FUN_0050f690();    // World_Init
    DBG("Scene_Intro: after Font_Init, before ClearInput");
    Input_ClearState(1);   // CharList_Init(1)
    DBG("Scene_Intro: after ClearInput, before Texture_Load Webzenlogo");

    // Load splash textures
    FUN_00529740(s_Local_Webzenlogo_jpg_00561774, 0xc, 0x2600, 0x2900, 0, '\x01');
    DBG("Scene_Intro: after Webzenlogo load");
    FUN_00529740(s_Local_Everyone_jpg_0056178c,   0xd, 0x2600, 0x2900, 0, '\x01');
    DBG("Scene_Intro: after Everyone load");

    DAT_083a42ea = 0;
    // Nota: el iTitle.wav (id 4) lo dispara el original con PlayBuffer(4,...)
    // en Game_SceneUpdate al entrar a Login — ruteado via FUN_00404bc0 +
    // kUISounds mapping en Sound_DS3D.cpp. NO tocar aquí (sonaría 2x).
    GL_BeginViewport(0, 0, 0x280, 0x1e0);   // Viewport_Set(0,0,640,480)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    GL_Begin2D();                       // GL_SetupOrtho2D
    glColor3f(1.0f, 1.0f, 1.0f);
    DBG("Scene_Intro: after GL_SetupOrtho2D");

    // Webzen logo — centered
    uStack_4 = 0;
    local_8 = (undefined1 *)DAT_0056156c;
    GL_DrawTexture(0xc,
        (float)DAT_0056156c * _DAT_00552504 - _DAT_005529bc,
        (float)DAT_0056156c * _DAT_00552d3c - _DAT_00552d38,
        256.0f, 195.0f, 0.0f, 0.0f, 1.0f, 0.76171875f, '\0', '\0');

    // ESRB "Everyone" badge — top right
    uStack_4 = 0;
    local_8 = (undefined1 *)DAT_0056156c;
    GL_DrawTexture(0xd,
        (float)DAT_0056156c - _DAT_00552908,
        0.0f, 120.0f, 60.0f, 0.0f, 0.0f, 0.9375f, 0.9375f, '\0', '\0');

    // Debug overlay (only in bypass mode)
    if (DAT_083a410c != '\0') {
        SelectObject((HDC)(uintptr_t)DAT_055c9fec, (HGDIOBJ)(uintptr_t)DAT_055ca00c);
        UI_RenderText(0, 0, (LPCSTR)&DAT_005617a0, (LPSIZE)0x0, '\0', 0);
    }

    DBG("Scene_Intro: before swap");
    GL_End2D();           // GL_End2D
    GL_PopMatrixAll();
    glFlush();
    SwapBuffers(param_1);
    DBG("Scene_Intro: after swap");

    // Unload splash textures immediately after display
    FUN_0052a050(0xd);   // Texture_Unload(Everyone)
    FUN_0052a050(0xc);   // Texture_Unload(Webzenlogo)

    // Normal path → Login
    if (DAT_083a410c == '\0') {
        DBG("Scene_Intro: entering normal path, calling OpenBasicData");
        FUN_00405540(&DAT_055c9bf0, "> Loading ok...");
        DAT_005615c0 = 2;   // g_GameState = Login
        Scene_LoadGameAssets();
        DBG("Scene_Intro: OpenBasicData returned");
        return;
    }

    // Bypass/debug path → skip login, go straight to InGame
    DAT_005615c0 = 5;    // g_GameState = InGame
    DAT_083a7c10 = 1;
    Scene_LoadGameAssets();
    FUN_0050e5a0();      // World_Load
    DAT_05826cac = 0;

    // Spawn local entity at hardcoded position (5414.4f, 21981.5f)
    // 0x44a8c000 = 1350.0f (x), 0x46a9ec00 = 21742.0f (y)
    local_8 = (undefined1 *)FUN_0045f930(0, 0, 0, 1350.0f, 21742.0f, 0.0f);
    puVar1  = (undefined4 *)(local_8 + 0x388);
    local_c = puVar1;
    uVar4   = (**(code **)(DAT_055c9bc8 + 0xc))(puVar1);

    // HashTable anti-tamper tracking (ref-count register/unregister, not game logic)
    uVar14 = 0;
    if (DAT_055c9bd4 != 0) {
        do {
            bVar12 = false; iVar8 = 0; bVar13 = true;
            pbVar11 = (byte *)(DAT_055c9bd0 + uVar4 * 4);
            iVar7 = 4;
            pbVar9 = (byte*)&stack_ec; pbVar10 = pbVar11;
            do {
                if (iVar7 == 0) break;
                iVar7--;
                bVar12 = *pbVar9 < *pbVar10;
                bVar13 = *pbVar9 == *pbVar10;
                pbVar9++; pbVar10++;
            } while (bVar13);
            if (!bVar13) iVar8 = (1 - (uint)bVar12) - (uint)(bVar12 != 0);
            if (iVar8 == 0) goto LAB_00513a36;
            iVar7 = 4; bVar12 = false; iVar8 = 0; bVar13 = true;
            pbVar9 = (byte *)&iStack_10;
            do {
                if (iVar7 == 0) break;
                iVar7--;
                bVar12 = *pbVar9 < *pbVar11;
                bVar13 = *pbVar9 == *pbVar11;
                pbVar9++; pbVar11++;
            } while (bVar13);
            if (!bVar13) iVar8 = (1 - (uint)bVar12) - (uint)(bVar12 != 0);
            if (iVar8 == 0) {
                if (uVar4 == 0xffffffff) goto LAB_00513a36;
                puVar6 = (undefined4 *)FUN_00404280(&DAT_055c9bc8, puVar1);
                cVar2  = *(char *)(puVar6 + 1);
                *(byte *)(puVar6 + 1) = cVar2 + 1U;
                if ((byte)(cVar2 + 1U) < 2)
                    FUN_00409e20(puVar1, puVar6);
            }
            uVar4 = (uVar4 + 1) % DAT_055c9bd4;
            uVar14++;
        } while (uVar14 < DAT_055c9bd4);
    }
    FUN_00405540(&DAT_055c9bf0, s_Hash_table_full______GetIndex_00558108);

LAB_00513a36:
    pvVar5 = operator_new(5);
    *(undefined1 *)((int)pvVar5 + 4) = 1;
    FUN_00403f80(&DAT_055c9bc8, pvVar5, puVar1);

LAB_00513a50:
    iVar7 = DAT_055c9bc8;
    *puVar1 = 0xd;
    uVar4 = (**(code **)(iVar7 + 0xc))(puVar1);
    uStack_4 = 0;
    if (DAT_055c9bd4 != 0) {
        do {
            iVar8 = 4; pbVar11 = (byte *)(DAT_055c9bd0 + uVar4 * 4);
            bVar12 = false; iVar7 = 0; bVar13 = true;
            pbVar9 = (byte*)&stack_ec; pbVar10 = pbVar11;
            do {
                if (iVar8 == 0) break; iVar8--;
                bVar12 = *pbVar9 < *pbVar10; bVar13 = *pbVar9 == *pbVar10;
                pbVar9++; pbVar10++;
            } while (bVar13);
            if (!bVar13) iVar7 = (1 - (uint)bVar12) - (uint)(bVar12 != 0);
            if (iVar7 == 0) goto LAB_00513b4e;
            iVar8 = 4; bVar12 = false; iVar7 = 0; bVar13 = true;
            pbVar9 = (byte*)&stack_e8;
            do {
                if (iVar8 == 0) break; iVar8--;
                bVar12 = *pbVar9 < *pbVar11; bVar13 = *pbVar9 == *pbVar11;
                pbVar9++; pbVar11++;
            } while (bVar13);
            if (!bVar13) iVar7 = (1 - (uint)bVar12) - (uint)(bVar12 != 0);
            if (iVar7 == 0) {
                if (uVar4 != 0xffffffff) {
                    puVar6 = (undefined4 *)FUN_00404280(&DAT_055c9bc8, puVar1);
                    cVar2  = *(char *)(puVar6 + 1);
                    *(char *)(puVar6 + 1) = cVar2 - 1;
                    if ((char)(cVar2 - 1) == '\0')
                        FUN_00423760(puVar6, puVar1);
                }
                goto LAB_00513b4e;
            }
            uVar4 = (uVar4 + 1) % DAT_055c9bd4;
            uStack_4++;
        } while (uStack_4 < DAT_055c9bd4);
    }
    FUN_00405540(&DAT_055c9bf0, s_Hash_table_full______GetIndex_00558108);

LAB_00513b4e:
    puVar1 = (undefined4 *)(iStack_10 + 0x38c);
    uVar4  = HashTable_GetIndex(&DAT_055c9bc8, puVar1);
    if (uVar4 == 0xffffffff) {
        pvVar5 = operator_new(5);
        *(undefined1 *)((int)pvVar5 + 4) = 1;
        FUN_00403f80(&DAT_055c9bc8, pvVar5, puVar1);
    } else {
        uVar4  = HashTable_GetIndex(&DAT_055c9bc8, puVar1);
        puVar6 = (uVar4 == 0xffffffff) ? (undefined4 *)0x0
                                       : *(undefined4 **)(DAT_055c9bcc + uVar4 * 4);
        cVar2  = *(char *)(puVar6 + 1);
        *(byte *)(puVar6 + 1) = cVar2 + 1U;
        if ((byte)(cVar2 + 1U) < 2)
            FUN_00409e20(puVar1, puVar6);
    }
    *puVar1 = 0xd9;
    uVar4 = HashTable_GetIndex(&DAT_055c9bc8, puVar1);
    if (uVar4 != 0xffffffff) {
        uVar4  = HashTable_GetIndex(&DAT_055c9bc8, puVar1);
        puVar6 = (uVar4 == 0xffffffff) ? (undefined4 *)0x0
                                       : *(undefined4 **)(DAT_055c9bcc + uVar4 * 4);
        cVar2  = *(char *)(puVar6 + 1);
        *(char *)(puVar6 + 1) = cVar2 - 1;
        if ((char)(cVar2 - 1) == '\0')
            FUN_00423760(puVar6, puVar1);
    }
}
