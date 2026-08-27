// Scene_CharSelect.cpp
// Scene_CharSelect @ 0x00523B30  (410 lines, decompile completo)
//
// Pantalla de selección de personaje: fondo 3D + UI 2D.
// Brillo oscilante por sinusoide, órbita de partículas, name labels con color por clase.
//
// ── SIGNATURA ─────────────────────────────────────────────────────────────────
//
//   int Scene_CharSelect(void)
//   returns: low byte != 0 → SwapBuffers; 0 → skip
//
// ── GUARDS ────────────────────────────────────────────────────────────────────
//
//   if (CharSelectSceneInitialized == '\0') return 0; // IDA: DAT_083a7c4b
//   if (0x32 >= DAT_05826cb0) { ... }   // timer guard → solo setup parcial
//
// ── BRILLO OSCILANTE ──────────────────────────────────────────────────────────
//
//   fVar11 = fsin(DAT_05826e08 * _DAT_005528e0)
//   local_70 = (float)(fVar11 * _DAT_005526e4 + _DAT_005526e8)  // range ~0.3..1.0
//   if (DAT_005616b0 != -1) local_70 = 0.2;   // char hovereado → dim otros más
//
//   // Setear color por slot:
//   for (iVar7 = 0; iVar7 < 5; iVar7++) {
//     entity = DAT_07abf5d0 + iVar7 * 0x394;
//     if (iVar7 == DAT_005616ac || iVar7 == SelectedCharacter) {
//       color = (1.0, 1.0, 1.0);    // seleccionado/hover → full bright
//     } else {
//       color = (local_70 * 0.5f, local_70 * 0.5f, local_70 * 0.5f);
//     }
//     entity[0xec..0xf0] = color;   // RGB floats de la entidad
//     if (entity.class == 0x333) entity.y_offset = 20.0;   // Dark Knight offset
//   }
//
// ── PARTÍCULAS ÓRBITALES ──────────────────────────────────────────────────────
//
//   if (DAT_005616ac != -1 && entity[selected].active) {
//     GL_SetBlendAdditive();   → Frame_UpdateTimer()
//     local_6c = ftol() % 0xe10;   // modulo 3600
//     local_70 = (float)local_6c * _DAT_00552a00;   // ángulo en radianes
//     FUN_00473ea0(0x4f1, pos, 0x420c0000, 0x425c0000, 0x43160000, +local_70, 0, 0.0);
//     FUN_00473ea0(0x4f1, pos, 0x420c0000, 0x425c0000, 0x43160000, -local_70, 0, 0.0);
//     // 0x4f1 = tipo partícula orbital, ±ángulo = dos partículas en órbita opuesta
//   }
//
// ── RENDER 3D ─────────────────────────────────────────────────────────────────
//
//   glClearColor(0,0,0,1.0);
//   GL_BeginViewport(0, 0, 0x280, 0x1e0);
//   Camera_BuildMouseRay(DAT_083a427c, DAT_083a4278, &DAT_083a4110); → Camera_Update(mx,my,mat)
//   FUN_004fd800();     → Terrain_Render()
//   _DAT_07abf138 = 1.0; _DAT_07abf13c = 0.9; _DAT_07abf140 = 0.8;
//   if (DAT_005616b0 == -1): Mouse_UpdateHoverTargets(); → CharPreview_Render()
//   FUN_0045ab00();     → Entity_RenderAll_3D()
//   FUN_00500970();     → Entity_Render_Sprites()
//   FUN_0046c3e0();     → Particle_Render()
//   GL_SetBlendSrcOver('\x01'); GL_SetMode(1)
//   GL_BindTextureSlot(0x15); glColor4f(1,1,1,0.8);
//   GL_BeginSprite();     → SkillEffect_Render_2()
//   Render_DrawSpritePool();     → Portal_Render()
//   FUN_00478c00();     → ItemDrop_Render_2()
//   glPopMatrix();
//   GL_Begin2D();     → GL_SetupOrtho2D()
//
// ── NAME LABELS (cuando DAT_005616b0 == -1: ninguno hovereado) ───────────────
//
//   for (iVar9 = 0; iVar9 < 0x11e4; iVar9 += 0x394):
//     entity = DAT_07abf5d0 + iVar9
//     if (entity[0] != '\0'):
//       Camera_ProjectWorldToScreen(&pos, &screenX, &screenY)   → World_ToScreen(pos, &x, &y)
//       flags = entity[0x1c0]
//       if   flags & 1:  color = 0x8000ffff (cyan)     // Elf?
//       elif flags & 0x12: color = 0x80ffff00 (yellow)  // GM?
//       elif flags & 4:  color = 0x80ffffff (white)
//       elif flags & 8:  color = 0x80ff0000 (red)       // PK
//       else:            color = 0xffffc8b4 (gold)       // normal
//       DrawText(screenX - cx/2, screenY - cy - 3, class_name, font_small)
//       DrawText(screenX - cx/2, screenY,           "%s %d", font_normal)
//
// ── WARNING TEXT (DAT_083a7c4d != '\0') ──────────────────────────────────────
//
//   color = 0x80ffff00 (yellow)
//   DrawText_Centered(0x140, 0x14a, lpString_07d49c14)   // "Cannot create..."
//   DrawText_Centered(0x140, 0x15c, lpString_07d49d40)
//
// ── SERVER INFO (bottom right, color 0xff67bfdf) ────────────────────────────
//
//   DrawText_Centered(0x1b8, 0x186, &lpString_00561a3c)
//   DrawText_Centered(0x1b8, 0x19a, &lpString_00561a58)
//   DrawText_Centered(0x1b8, 0x1ae, &lpString_00561a68)
//
// ── PANEL INFO (estados 0x1b..0x1c: create character) ────────────────────────
//
//   Texture_Draw2D(0x16, 214, iVar9,        213, 256, ...)  // panel principal
//   Texture_Draw2D(0x17, 214, iVar9+0x100,  213, 119, ...)  // panel inferior
//   Flechas de stats (tex 0x1a/0x1b, 0x1c/0x1d) con hit-test mouse
//   Botón OK (tex 0x18 @ 235, iVar9+0x15d, 72×21) — hover tex 0x18/0x19
//   Botón Cancel (tex 0x19 @ 335, iVar9+0x15d, 72×21)
//   Class shadow/highlight si DAT_07abf20c & 0xff == 0: glColor4f(0,0,0,0.3)
//   DrawText(0x11d, iVar9+0xb4, 0) si server seleccionado
//   DrawText_ClassLine(0x11d, iVar9+200, DAT_07d2b494 + class*300)
//
// ── RENDER CHAR PREVIEW (panel create, iVar9 >= 0) ───────────────────────────
//
//   (oscila fsin → _DAT_07abf138..140 = brillo)
//   fStack_5c = -8.0; uStack_58 = 0xc4480000; fStack_54 = 79.0;
//   DAT_00561554 = 0x41200000;
//   Matrix_BuildFromEuler(&angles, mat);   → Matrix_FromEuler()
//   Vector_InverseRotate(&pos, mat, &screenPt); → Transform + project
//   _DAT_083a42d4 = screenPt.x + _DAT_07abf060;
//   _DAT_083a42d8 = screenPt.y + _DAT_07abf064;
//   GL_BeginViewport(0x11d, iVar9+0x5a, 0x4a, 0x4f);
//   FUN_00456770(&entity, &entity, NULL);   → Entity_UpdateRender()
//   GL_PopMatrixAll();
//
// ── PANEL SERVER SELECT (DAT_005616b0 == -1, bottom) ─────────────────────────
//
//   // Dimmer basado en si hay slots vacíos:
//   if any_slot_empty: color=1.0 else: color=0.5
//   Texture_Draw2D(0x10, 221, iVar9, 199, 109, 0, 0, 0.777, 0.852, ...)
//   Texture_Draw2D(0x11, 286, iVar9+0x4b, 70, 25, ...) si mouse en zona
//   // Botones Left/Right char:
//   Texture_Draw2D(0x12, DAT_005616a8, 116, 205, 88, 0, 0, 0.801, 0.688, ...)
//   Texture_Draw2D(0x13, fVar1+0x80, 148, 70, 19, ...) si mouse en zona
//   Texture_Draw2D(0x14, -(int)DAT_005616a8+0x1b2, 116, 205, 88, ...)
//   Texture_Draw2D(0x15, -(int)DAT_005616a8+0x1b9, 148, 70, 19, ...) si mouse
//
// ── SUBSISTEMAS FINALES ───────────────────────────────────────────────────────
//
//   FUN_005239a0()  → CharSelect_UpdateInput()
//   FUN_0051af50()  → Chat_Render()
//   FUN_004f64d0()  → UI_Render()
//   UI_RenderNotices()  → StatusBar_Render()
//   UI_RenderChatLogOverlay()  → Mouse_Render()
//   UI_UpdateFpsCounter/4c3530/4bffa0/0051e0c0/5124b0 → UI finalizadores
//   GL_PopMatrixAll()
//   return CONCAT31(..., 1)  → low byte 1 = SwapBuffers
//
// ── GLOBALS ───────────────────────────────────────────────────────────────────
//
//   CharSelectSceneInitialized (IDA: DAT_083a7c4b) — init flag (guard)
//   DAT_05826cb0   — timer guard (> 0x32 para renderizar)
//   DAT_083a42ea   — frame counter reset
//   DAT_05826e08   — frame time accumulator (usado en fsin para brillo)
//   DAT_005616ac   — selected char index (-1=ninguno)
//   DAT_005616b0   — hovered char index (-1=ninguno)
//   SelectedCharacter   — slot local player index
//   DAT_083a021c   — puntero a entidad del char hovereado
//   DAT_083a4110   — camera matrix output
//   DAT_083a427c   — mouse X
//   DAT_083a4278   — mouse Y
//   DAT_083a7c4d   — warning flag
//   DAT_07abf20c   — char class byte (low byte = slot)
//   DAT_0056156c   — screen_width
//   DAT_005597c4   — render sub-mode flag
//   DAT_00561554   — camera param
//   _DAT_07abf138..140 — RGB color del preview local
//   _DAT_007abf060..068 — camera offset
//   DAT_005616a8   — posición X botones left/right
//   DAT_07d2b494   — tabla de nombres de clase (stride 300 bytes)
//
// ── FUNCIÓN CROSS-REFERENCE ───────────────────────────────────────────────────
//
//   FUN_004fd800   → Terrain_Render()
//   FUN_0045ab00   → Entity_RenderAll_3D()
//   FUN_00500970   → Entity_Render_Sprites()
//   FUN_0046c3e0   → Particle_Render()
//   FUN_00473ea0   → Particle_Spawn(type, pos, r0, r1, r2, angle, ?, z)
//   Camera_BuildMouseRay   → Camera_Update(mx, my, mat)
//   Camera_ProjectWorldToScreen   → World_ToScreen(pos, &x, &y)
//   Mouse_UpdateHoverTargets   → CharPreview_Render()
//   FUN_005239a0   → CharSelect_UpdateInput()
//   FUN_00456770   → Entity_UpdateRender(entity, entity, is_local)
//   Vector_InverseRotate   → Matrix_TransformVec(pos, mat, out)
//   Matrix_BuildFromEuler   → Matrix_FromEuler(angles, out)
//   GL_BindTextureSlot   → Particle_SetTexture(type)
//   GL_SetBlendSrcOver   → GL_SetMode(mode)
//   GL_SetBlendAdditive   → Frame_UpdateTimer()

#include "stdafx.h"
#include "Scene/Scene_CharSelect.h"

extern "C" { void DbgLogPublic(const char*); }

// Scene_CharSelect @ 0x00523B30
// Renders the character selection screen (3D world + 2D UI).
// Returns low byte 1 = SwapBuffers, 0 = skip.

int Scene_CharSelect(void)
{
    float     fVar1;
    byte      bVar2;
    uint3     uVar6 = 0;
    float    *pfVar3;
    int       iVar4, iVar7, iVar9;
    char     *pcVar5;
    uint      uVar8;
    bool      bVar10;
    float10   fVar11;
    ulonglong uVar12;
    undefined4 uVar13, uVar14, uVar16;
    tagSIZE   tStack_68;
    float     local_70, fStack_60, fStack_5c, fStack_54;
    int       local_6c;
    undefined4 uStack_58;
    CHAR      aCStack_50[32];
    float     afStack_30[12];

    if (CharSelectSceneInitialized == '\0')
        return (uint)uVar6 << 8;

    if (0x32 < DAT_05826cb0) {
        DAT_083a42ea = 0;
        DAT_07e11d6e = 0;
        // BUG-FIX: 0x3f800000 son los bits de 1.0f, no la magnitud.
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        GL_BeginViewport(0, 0, 0x280, 0x1e0);
        Camera_BuildMouseRay(DAT_083a427c, DAT_083a4278, (float *)&DAT_083a4110);

        // Oscillating brightness: fsin on frame timer → range ~0.3..1.0
        iVar9  = DAT_083a021c;
        fVar11 = (float10)sinl((long double)DAT_05826e08 * (long double)_DAT_005528e0);
        local_70 = (float)(fVar11 * (float10)_DAT_005526e4 + (float10)_DAT_005526e8);
        if (-1 < DAT_005616b0)
            local_70 = 0.2f;   // hovereado → dim los demás más

        // Set hovered entity brightness
        if (DAT_083a021c != 0) {
            *(float *)(DAT_083a021c + 0xe8) = local_70;
            *(float *)(iVar9 + 0xec) = local_70;
            *(float *)(iVar9 + 0xf0) = local_70;
            *(float *)(iVar9 + 0x68) = local_70;
        }

        FUN_004fd800();   // Terrain_Render

        _DAT_07abf138 = 1.0f;
        _DAT_07abf13c = 0.9f;
        _DAT_07abf140 = 0.8f;

        if (DAT_005616b0 == -1)
            Mouse_UpdateHoverTargets();   // CharPreview_Render

        // Per-slot color: selected/hover → full bright; others → dimmed
        iVar4 = DAT_005616ac;
        iVar9 = SelectedCharacter;
        iVar7 = 0;
        pfVar3 = (float *)(DAT_07abf5d0 + 0xec);
        _DAT_0055a7c0 = 0;
        do {
            if ((iVar7 == iVar4) || (iVar7 == iVar9)) {
                pfVar3[-1] = 1.0f; *pfVar3 = 1.0f; pfVar3[1] = 1.0f;
            } else {
                fVar1 = local_70 * _DAT_00552504;
                pfVar3[-1] = fVar1; *pfVar3 = fVar1; pfVar3[1] = fVar1;
            }
            // Dark Knight class fix: y_offset = 20
            if (*(short *)(pfVar3 + 0x73) == 0x333)
                pfVar3[-0x35] = 20.0f;
            iVar7++;
            pfVar3 = pfVar3 + 0xe5;
        } while (iVar7 < 5);

        // ── DIAG: log 5 slots once a second so we can verify MG entity state ──
        {
            static DWORD s_lastDiag = 0;
            DWORD now = GetTickCount();
            if (now - s_lastDiag > 1000) {
                s_lastDiag = now;
                for (int s = 0; s < 5; ++s) {
                    BYTE* e = (BYTE*)(DAT_07abf5d0 + s * 0x394);
                    char buf[260];
                    _snprintf_s(buf, sizeof(buf), _TRUNCATE,
                        "CSEL slot=%d act=%d cls=%d lvl=%d name='%s' "
                        "pos=(%.1f,%.1f,%.1f) vis@160=%d flags@1c0=0x%x "
                        "color=(%.2f,%.2f,%.2f) y_off@b0=%.1f "
                        "wing=0x%x",
                        s, e[0], e[0x1bc], *(WORD*)(e+0x1be),
                        (const char*)(e+0x1c1),
                        *(float*)(e+0x10), *(float*)(e+0x14), *(float*)(e+0x18),
                        e[0x160], e[0x1c0],
                        *(float*)(e+0xe8), *(float*)(e+0xec), *(float*)(e+0xf0),
                        *(float*)(e+0xb0),
                        *(WORD*)(e+0x2a0));
                    DbgLogPublic(buf);
                }
            }
        }
        // ── DIAG: dump camera + current-camera state once/sec
        {
            static DWORD s_lastCAM = 0;
            DWORD now = GetTickCount();
            if (now - s_lastCAM > 1000) {
                s_lastCAM = now;
                char b[260];
                _snprintf_s(b, sizeof(b), _TRUNCATE,
                    "CAM pos=(%.1f,%.1f,%.1f) ang=(%.2f,%.2f,%.2f) "
                    "curpos=(%.1f,%.1f,%.1f) curang=(%.2f,%.2f,%.2f)",
                    CameraPosition[0], CameraPosition[1], CameraPosition[2],
                    CameraAngle[0], CameraAngle[1], CameraAngle[2],
                    CurrentCameraPosition[0], CurrentCameraPosition[1], CurrentCameraPosition[2],
                    CurrentCameraAngle[0], CurrentCameraAngle[1], CurrentCameraAngle[2]);
                DbgLogPublic(b);
            }
        }
        FUN_0045ab00();   // Entity_RenderAll_3D

        // Orbital particles for selected char
        if ((DAT_005616ac != -1) && (((char*)DAT_07abf5d0)[DAT_005616ac * 0x394] != '\0')) {
            GL_SetBlendAdditive();   // Frame_UpdateTimer
            uVar12  = (unsigned int)(*(float*)&DAT_05826e08);
            local_6c = (int)((longlong)((ulonglong)(uint)((int)uVar12 >> 0x1f) << 0x20 | uVar12 & 0xffffffff) % 0xe10);
            local_70 = (float)local_6c * _DAT_00552a00;
            FUN_00473ea0(0x4f1, (float *)(DAT_07abf5d0 + DAT_005616ac * 0x394 + 0x10),
                         0x420c0000, 0x425c0000, 0x43160000,  local_70, 0, 0.0f);
            FUN_00473ea0(0x4f1, (float *)(DAT_07abf5d0 + DAT_005616ac * 0x394 + 0x10),
                         0x420c0000, 0x425c0000, 0x43160000, -local_70, 0, 0.0f);
        }

        FUN_00500970();                    // Entity_Render_Sprites
        FUN_0046c3e0();                    // Particle_Render
        GL_SetBlendSrcOver('\x01');              // GL_SetMode(1)
        GL_BindTextureSlot(0x15);
        glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
        GL_BeginSprite();                    // SkillEffect_Render_2
        Render_DrawSpritePool();                    // Portal_Render
        FUN_00478c00();                    // ItemDrop_Render_2
        glPopMatrix();
        GL_Begin2D();                    // GL_SetupOrtho2D
        glColor3f(1.0f, 1.0f, 1.0f);
        GL_SetBlendSrcOver('\x01');
        GL_ResetState();

        // ── Name labels (no char hovered) ────────────────────────────────────
        {
            static DWORD s_lastNL = 0;
            DWORD now = GetTickCount();
            if (now - s_lastNL > 1000) {
                s_lastNL = now;
                char b[200];
                _snprintf_s(b, sizeof(b), _TRUNCATE,
                    "NAMES_GUARD DAT_005616b0=%d (will %s)",
                    (int)DAT_005616b0,
                    (DAT_005616b0 == -1) ? "RENDER NAMES" : "SKIP");
                DbgLogPublic(b);
            }
        }
        if (DAT_005616b0 == -1) {
            GL_SetBlendSrcOver('\x01');
            glColor3f(1.0f, 1.0f, 1.0f);
            iVar9 = 0;
            pcVar5 = (char*)DAT_07abf5d0;
            do {
                if (pcVar5[iVar9] != '\0') {
                    // ── BUG-FIX: fStack_5c/uStack_58/fStack_54 son 3 variables LOCALES
                    // separadas. MSVC no garantiza que estén contiguas en stack, así que
                    // pasar &fStack_5c a Camera_ProjectWorldToScreen (que lee [0]/[1]/[2]) producía
                    // proyecciones erráticas (TPos[2] no matcheaba con M*input).
                    // Usamos un array contiguo posBuf[3] para garantizar layout.
                    float posBuf[3];
                    posBuf[0] = *(float *)(pcVar5 + iVar9 + 0x10);
                    posBuf[1] = *(float *)(pcVar5 + iVar9 + 0x14);
                    posBuf[2] = *(float *)(pcVar5 + iVar9 + 0x18) + _DAT_00552974;
                    fStack_5c = posBuf[0];
                    uStack_58 = *(undefined4 *)&posBuf[1];
                    fStack_54 = posBuf[2];
                    {
                        static DWORD s_lastNI[5] = {0};
                        int slotIdx = iVar9 / 0x394;
                        DWORD now2 = GetTickCount();
                        if (slotIdx >= 0 && slotIdx < 5 && now2 - s_lastNI[slotIdx] > 1000) {
                            s_lastNI[slotIdx] = now2;
                            char b[400];
                            const char* nm = (const char*)((BYTE*)DAT_07abf5d0 + iVar9 + 0x1c1);
                            float* M = (float*)&DAT_083a4140;
                            _snprintf_s(b, sizeof(b), _TRUNCATE,
                                "NAME_PRE slot=%d name='%s' world=(%.1f,%.1f,%.1f) zoff=%.1f "
                                "M=(%.3f,%.3f,%.3f,%.3f|%.3f,%.3f,%.3f,%.3f|%.3f,%.3f,%.3f,%.3f) "
                                "ScrCtr=(%d,%d) Persp=(%.6f,%.6f) Win=(%d,%d)",
                                slotIdx, nm,
                                fStack_5c, *(float*)&uStack_58, fStack_54,
                                _DAT_00552974,
                                M[0],M[1],M[2],M[3], M[4],M[5],M[6],M[7], M[8],M[9],M[10],M[11],
                                (int)ViewportCenterX, (int)ViewportCenterY,
                                _DAT_083a42a4, _DAT_083a42a8,
                                (int)DAT_0056156c, (int)DAT_00561570);
                            DbgLogPublic(b);
                        }
                    }
                    // ── BUG-FIX: local_70 está declarado float (línea 202) pero
                    // Camera_ProjectWorldToScreen escribe un int en él vía cast pointer. La lectura
                    // posterior `(int)local_70` hace conversión FPU float→int sobre
                    // el bit-pattern denormal, dando ~0 y poniendo los nombres en
                    // y=-15. Usamos un int local separado para la proyección.
                    int nameProjY = 0;
                    Camera_ProjectWorldToScreen(posBuf, &local_6c, &nameProjY);
                    {
                        static DWORD s_lastNO[5] = {0};
                        int slotIdx2 = iVar9 / 0x394;
                        DWORD now3 = GetTickCount();
                        if (slotIdx2 >= 0 && slotIdx2 < 5 && now3 - s_lastNO[slotIdx2] > 1000) {
                            s_lastNO[slotIdx2] = now3;
                            char b2[160];
                            _snprintf_s(b2, sizeof(b2), _TRUNCATE,
                                "NAME_POST slot=%d proj=(%d,%d)",
                                slotIdx2, (int)local_6c, nameProjY);
                            DbgLogPublic(b2);
                        }
                    }
                    bVar2 = ((byte*)DAT_07abf5d0)[iVar9 + 0x1c0];
                    // Color by class/flags
                    if      (bVar2 & 1)    { DAT_00559c80 = 0x8000ffff; DAT_00559c78 = 0xff000000; }
                    else if (bVar2 & 0x12) { DAT_00559c80 = 0x80ffff00; DAT_00559c78 = 0xff000000; }
                    else if (bVar2 & 4)    { DAT_00559c80 = 0x80ffffff; DAT_00559c78 = 0xff000000; }
                    else if (bVar2 & 8)    { DAT_00559c80 = 0x80ff0000; DAT_00559c78 = 0xff000000; }
                    else                   { DAT_00559c80 = 0x80000000; DAT_00559c78 = 0xffffc8b4; }
                    // Name (top, bold font) — IDA L194-198: sprintf(String, "%s", entity+449)
                    SelectObject((HDC)(uintptr_t)DAT_055c9fec, (HGDIOBJ)(uintptr_t)DAT_055ca010);
                    {
                        const char* charName = (const char*)((BYTE*)DAT_07abf5d0 + iVar9 + 0x1c1);
                        crt_sprintf(aCStack_50, DAT_00561a30, charName);
                    }
                    iVar4 = lstrlenA(aCStack_50);
                    GetTextExtentPointA((HDC)(uintptr_t)DAT_055c9fec, aCStack_50, iVar4, &tStack_68);
                    UI_RenderText(local_6c - (uint)((tStack_68.cx / 2) * 0x280) / DAT_0056156c,
                                 (nameProjY - tStack_68.cy) + -3,
                                 aCStack_50, (LPSIZE)0x0, '\0', 3);
                    // Class name + level (bottom, normal font) — IDA L202-209:
                    //   sprintf(String, "%s %d",
                    //           ClassNameTable + 300 * ((klass & 7) + 4 * (klass >> 3)),
                    //           level)
                    // klass byte at entity+0x1bc (444): DW=0, DK=1, ELF=2, MG=3, SM=8, BK=9, ME=10
                    DAT_00559c80 = 0x80000000; DAT_00559c78 = 0xffffffff;
                    SelectObject((HDC)(uintptr_t)DAT_055c9fec, (HGDIOBJ)(uintptr_t)DAT_055ca00c);
                    {
                        BYTE klass = ((BYTE*)DAT_07abf5d0)[iVar9 + 0x1bc];
                        WORD lvl   = *(WORD*)((BYTE*)DAT_07abf5d0 + iVar9 + 0x1be);
                        int  cidx  = (klass & 7) + 4 * (klass >> 3);
                        // 0.97k tiene DW/DK/ELF/MG y evolved SM/BK/ME (no DL).
                        static const char* kClassNames[16] = {
                            /* 0 */ "Dark Wizard",
                            /* 1 */ "Dark Knight",
                            /* 2 */ "Fairy Elf",
                            /* 3 */ "Magic Gladiator",
                            /* 4 */ "Soul Master",
                            /* 5 */ "Blade Knight",
                            /* 6 */ "Muse Elf",
                            /* 7 */ "",
                            /* 8.. */ "", "", "", "", "", "", "", ""
                        };
                        const char* cn = kClassNames[cidx & 0xF];
                        crt_sprintf(aCStack_50, s__s__d_00561a34, cn, (int)lvl);
                    }
                    iVar4 = lstrlenA(aCStack_50);
                    GetTextExtentPointA((HDC)(uintptr_t)DAT_055c9fec, aCStack_50, iVar4, &tStack_68);
                    UI_RenderText(local_6c - (uint)((tStack_68.cx / 2) * 0x280) / DAT_0056156c,
                                 nameProjY, aCStack_50, (LPSIZE)0x0, '\0', 0);
                    pcVar5 = (char*)DAT_07abf5d0;
                }
                iVar9 += 0x394;
            } while (iVar9 < 0x11e4);
        }

        // ── Warning text ──────────────────────────────────────────────────────
        if (DAT_083a7c4d != '\0') {
            DAT_00559c80 = 0x80ffff00; DAT_00559c78 = 0xff000000;
            SelectObject((HDC)(uintptr_t)DAT_055c9fec, (HGDIOBJ)(uintptr_t)DAT_055ca00c);
            iVar9 = lstrlenA((LPCSTR)lpString_07d49c14);
            GetTextExtentPointA((HDC)(uintptr_t)DAT_055c9fec, (LPCSTR)lpString_07d49c14, iVar9, &tStack_68);
            UI_RenderText(0x140 - ((uint)(tStack_68.cx * 0x280) / DAT_0056156c >> 1), 0x14a,
                         (LPCSTR)lpString_07d49c14, (LPSIZE)0x0, '\0', 0);
            iVar9 = lstrlenA((LPCSTR)lpString_07d49d40);
            GetTextExtentPointA((HDC)(uintptr_t)DAT_055c9fec, (LPCSTR)lpString_07d49d40, iVar9, &tStack_68);
            UI_RenderText(0x140 - ((uint)(tStack_68.cx * 0x280) / DAT_0056156c >> 1), 0x15c,
                         (LPCSTR)lpString_07d49d40, (LPSIZE)0x0, '\0', 0);
        }

        // ── Server info text (bottom right, blue) ─────────────────────────────
        DAT_00559c80 = 0x80000000; DAT_00559c78 = 0xff67bfdf;
        SelectObject((HDC)(uintptr_t)DAT_055c9fec, (HGDIOBJ)(uintptr_t)DAT_055ca00c);
        iVar9 = lstrlenA((LPCSTR)&lpString_00561a3c);
        GetTextExtentPointA((HDC)(uintptr_t)DAT_055c9fec, (LPCSTR)&lpString_00561a3c, iVar9, &tStack_68);
        UI_RenderText(0x1b8 - ((uint)(tStack_68.cx * 0x280) / DAT_0056156c >> 1), 0x186,
                     (LPCSTR)&lpString_00561a3c, (LPSIZE)0x0, '\0', 0);
        iVar9 = lstrlenA((LPCSTR)&lpString_00561a58);
        GetTextExtentPointA((HDC)(uintptr_t)DAT_055c9fec, (LPCSTR)&lpString_00561a58, iVar9, &tStack_68);
        UI_RenderText(0x1b8 - ((uint)(tStack_68.cx * 0x280) / DAT_0056156c >> 1), 0x19a,
                     (LPCSTR)&lpString_00561a58, (LPSIZE)0x0, '\0', 0);
        iVar9 = lstrlenA((LPCSTR)&lpString_00561a68);
        GetTextExtentPointA((HDC)(uintptr_t)DAT_055c9fec, (LPCSTR)&lpString_00561a68, iVar9, &tStack_68);
        UI_RenderText(0x1b8 - ((uint)(tStack_68.cx * 0x280) / DAT_0056156c >> 1), 0x1ae,
                     (LPCSTR)&lpString_00561a68, (LPSIZE)0x0, '\0', 0);

        GL_End2D();
        GL_PopMatrixAll();

        // ── Create character panel (states 0x1b..0x1c) ────────────────────────
        if ((0x1a < DAT_083a7c14) && (DAT_083a7c14 < 0x1d)) {
            GL_BeginViewport(0, 0, 0x280, 0x1e0);
            GL_Begin2D();
            GL_SetBlendSrcOver('\x01');
            glColor3f(1.0f, 1.0f, 1.0f);

            iVar9 = DAT_005616a4;
            uVar8 = 2;
            iVar4 = 5;
            pcVar5 = (char*)DAT_07abf5d0;
            do {
                if ((*pcVar5 != '\0') && (0xdb < *(ushort *)(pcVar5 + 0x1be)))
                    uVar8 = 3;
                pcVar5 += 0x394;
            } while (--iVar4 != 0);

            local_6c = DAT_005616a4;
            GL_DrawTexture(0x16, 214.0f, (float)DAT_005616a4, 213.0f, 256.0f, 0.0f, 0.0f, 0.83203125f, 1.0f, '\x01', '\x01');
            local_6c = iVar9 + 0x100;
            GL_DrawTexture(0x17, 214.0f, (float)local_6c, 213.0f, 119.0f, 0.0f, 0.0f, 0.83203125f, 0.9296875f, '\x01', '\x01');

            fVar1 = (float)(iVar9 + 0xc4);
            local_70 = fVar1;
            // Arrow up (stat increment): tex 0x1a or 0x1b depending on hover
            if ((((0x16a < DAT_083a427c) && (DAT_083a427c < 0x17f)) && ((int)fVar1 <= DAT_083a4278)) &&
                ((DAT_083a4278 < iVar9 + 0xd7) && (DAT_083a42c4 != '\0')))
            {
                GL_DrawTexture(0x1a, 363.0f, (float)(int)fVar1, 20.0f, 19.0f, 0.0f, 0.0f, 0.625f, 0.59375f, '\x01', '\x01');
            }
            if ((char)DAT_07abf20c == '\0') {
                // BUG-FIX: 0x3f4ccccd = 0.8f bits, pasado como int → 1062836429.0f.
                glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
                GL_DrawRect(364.0f, (float)(int)local_70, 20.0f, 19.0f);
                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            }
            local_70 = fVar1;
            if (((0x17f < DAT_083a427c) && (DAT_083a427c < 0x194)) &&
                (((int)fVar1 <= DAT_083a4278) && ((DAT_083a4278 < iVar9 + 0xd7) && (DAT_083a42c4 != '\0'))))
            {
                GL_DrawTexture(0x1b, 384.0f, (float)(int)fVar1, 20.0f, 19.0f, 0.0f, 0.0f, 0.625f, 0.59375f, '\x01', '\x01');
            }
            if (uVar8 <= (DAT_07abf20c & 0xff)) {
                // BUG-FIX 2026-07-17: la Y del overlay de "flecha derecha bloqueada" era
                // 0.0f (se dibujaba fuera de la flecha) → la flecha no se veía bloqueada
                // al llegar al máximo de clase disponible (ej: Fairy Elf sin poder ir a MG).
                // IDA RenderColor(384, dialogY+196, ...) → usa la misma Y que la izquierda.
                glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
                GL_DrawRect(384.0f, (float)(int)local_70, 20.0f, 19.0f);
                glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            }

            GL_SetBlendSrcOver('\x01');
            fVar1 = (float)(iVar9 + 0xd7);
            // Arrow down (stat decrement): tex 0x1c or 0x1d
            if ((((0x16a < DAT_083a427c) && (DAT_083a427c < 0x17f)) && ((int)fVar1 <= DAT_083a4278)) &&
                ((DAT_083a4278 < iVar9 + 0xe9) && (DAT_083a42c4 != '\0')))
            {
                local_70 = fVar1;
                GL_DrawTexture(0x1c, 363.0f, (float)(int)fVar1, 20.0f, 18.0f, 0.0f, 0.0f, 0.625f, 0.5625f, '\x01', '\x01');
            }
            if (((0x17f < DAT_083a427c) && (DAT_083a427c < 0x194)) &&
                (((int)fVar1 <= DAT_083a4278) && ((DAT_083a4278 < iVar9 + 0xe9) && (DAT_083a42c4 != '\0'))))
            {
                local_70 = fVar1;
                GL_DrawTexture(0x1d, 384.0f, (float)(int)fVar1, 20.0f, 18.0f, 0.0f, 0.0f, 0.625f, 0.5625f, '\x01', '\x01');
            }

            // OK button (tex 0x18/0x19)
            fVar1 = (float)(iVar9 + 0x15d);
            if ((((0xea < DAT_083a427c) && (DAT_083a427c < 0x133)) && ((int)fVar1 <= DAT_083a4278)) &&
                (DAT_083a4278 < iVar9 + 0x172))
            {
                local_70 = fVar1;
                GL_DrawTexture(0x18, 235.0f, (float)(int)fVar1, 72.0f, 21.0f, 0.0f, 0.0f, 0.5625f, 0.65625f, '\x01', '\x01');
            }
            local_70 = fVar1;
            if (((0x14e < DAT_083a427c) && (DAT_083a427c < 0x197)) &&
                (((int)fVar1 <= DAT_083a4278) && (DAT_083a4278 < iVar9 + 0x172)))
            {
                GL_DrawTexture(0x19, 335.0f, (float)(int)fVar1, 72.0f, 21.0f, 0.0f, 0.0f, 0.5625f, 0.65625f, '\x01', '\x01');
            }

            SelectObject((HDC)(uintptr_t)DAT_055c9fec, (HGDIOBJ)(uintptr_t)DAT_055ca00c);
            DAT_00559c8c = 0x49;
            // BUG-FIX 2026-07-17: DAT_005616b0 es DWORD (unsigned); `-1 < DAT_005616b0`
            // convertía -1 a 0xFFFFFFFF → comparación SIEMPRE falsa → RenderInputText
            // (el campo del nombre) nunca se dibujaba. IDA: `if (dword_5616B0 >= 0)`.
            if ((int)DAT_005616b0 >= 0)
                UI_RenderInputField(0x11d, iVar9 + 0xb4, 0);
            UI_RenderText(0x11d, iVar9 + 200, DAT_07d2b494 + (DAT_07abf20c & 0xff) * 300, (LPSIZE)0x0, '\0', 0);

            GL_End2D();
            GL_PopMatrixAll();

            // Character preview sub-render
            if (-1 < iVar9) {
                DAT_005597c4 = 0;
                fVar11 = (float10)sinl((long double)DAT_05826e08 * (long double)_DAT_005528e0);
                DAT_00561554 = 0x41200000;   // CameraFOV = 10.0
                DAT_083a42b8 = 0; DAT_083a42bc = 0; DAT_083a42c0 = 0;   // CameraAngle = {0,0,0}
                fVar11 = fVar11 * (float10)_DAT_005526e4 + (float10)_DAT_0055256c;
                _DAT_07abf138 = (float)fVar11;
                _DAT_07abf13c = (float)(fVar11 * (float10)_DAT_00552530);
                _DAT_07abf140 = (float)(fVar11 * (float10)_DAT_00552504);
                Matrix_BuildFromEuler((float *)&DAT_083a42b8, afStack_30);   // AngleMatrix(CameraAngle, matrix)
                // BUG-FIX 2026-07-17: la entrada in1={-8,-800,79} y la salida de
                // VectorIRotate estaban en locals SEPARADOS no contiguos (fStack_5c/
                // uStack_58/fStack_54 y tStack_68.cx/cy + fStack_60), y la salida se leía
                // con (float)cast (convert) en vez de reinterpret de los bits float →
                // CameraPosition del preview quedaba en una posición basura → el char se
                // renderizaba fuera del viewport (recuadro vacío). Arrays contiguos + IDA-fiel.
                {
                    float camIn[3]  = { -8.0f, -800.0f, 79.0f };   // in1
                    float camOut[3];                                // VectorIRotate out
                    Vector_InverseRotate(camIn, afStack_30, camOut);        // VectorIRotate(in1, matrix, out)
                    _DAT_083a42d4 = camOut[0] + _DAT_07abf060;      // CameraPosition[0]
                    DAT_083a42b8 = 0xc2b40000;                      // CameraAngle[0] = -90.0
                    _DAT_083a42d8 = camOut[1] + _DAT_07abf064;      // CameraPosition[1]
                    _DAT_083a42dc = camOut[2] + _DAT_07abf068;      // CameraPosition[2]
                }
                GL_BeginViewport(0x11d, iVar9 + 0x5a, 0x4a, 0x4f);
                // BUG-FIX 2026-04-20: _DAT_07abf0?? están tipados `float`;
                // asignar 0x40a00000 / 0x3f800000 hace int→float (1e9), no 5.0f / 1.0f
                _DAT_07abf06c = 0.0f; _DAT_07abf070 = 5.0f; _DAT_07abf05c = 1.0f;
                _DAT_07abf138 = 0.8f; _DAT_07abf13c = 0.8f; _DAT_07abf140 = 0.8f;
                FUN_00456770((undefined4 *)&DAT_07abf050, (undefined4 *)&DAT_07abf050, (undefined4 *)0x0);
                GL_PopMatrixAll();
                DAT_005597c4 = 1;
            }
        }

        // ── Bottom panels / buttons ───────────────────────────────────────────
        GL_BeginViewport(0, 0, 0x280, 0x1e0);
        // Reset entity[0] world pos
        DAT_07abf5d8 = (char*)DAT_07abf5d0;
        pcVar5 = (char*)DAT_07abf5d0;
        pcVar5[0x10] = '\0'; pcVar5[0x11] = '\0'; pcVar5[0x12] = '\0'; pcVar5[0x13] = '\0';
        pcVar5[0x14] = '\0'; pcVar5[0x15] = '\0'; pcVar5[0x16] = '\0'; pcVar5[0x17] = '\0';
        GL_Begin2D();

        if (DAT_005616b0 == -1) {
            GL_SetBlendSrcOver('\x01');
            // Check if any slot is empty
            bVar10 = false;
            iVar9 = 5;
            pcVar5 = (char*)DAT_07abf5d0;
            do {
                if (*pcVar5 == '\0') bVar10 = true;
                pcVar5 += 0x394;
            } while (--iVar9 != 0);

            // Create button brightness
            // BUG-FIX (2026-04-21): uVar* are uint holding bit-patterns 0x3f800000 (1.0f)
            // / 0x3f000000 (0.5f). glColor3f expects GLfloat → int→float conv gives
            // 1065353216.0f / 1056964608.0f → OpenGL clamps to 1.0 → always white
            // regardless of empty-slot state. Use float literals.
            float fR, fG, fB;
            if (bVar10) { fR = fG = fB = 1.0f; }
            else         { fR = fG = fB = 0.5f; }
            glColor3f(fR, fG, fB);

            iVar9 = DAT_005616a4;
            local_6c = DAT_005616a4;
            GL_DrawTexture(0x10, 221.0f, (float)DAT_005616a4, 199.0f, 109.0f, 0.0f, 0.0f, 0.77734375f, 0.8515625f, '\x01', '\x01');
            local_70 = (float)(iVar9 + 0x4b);
            // BUG-FIX 2026-04-26: el hover-text "NEW CHARACTER" (sprite 0x11) se
            // mostraba aunque la cuenta tuviera los 5 slots ocupados. El brightness
            // del botón base (sprite 0x10) ya gateaba en `bVar10` (hay slot vacío),
            // pero el render del label hover faltaba el mismo guard. Original solo
            // muestra hover si quedaba algún slot libre.
            if (bVar10 && ((0x11d < DAT_083a427c) && (DAT_083a427c < 0x164)) &&
                (((int)local_70 <= DAT_083a4278) && (DAT_083a4278 < iVar9 + 100)))
            {
                GL_DrawTexture(0x11, 286.0f, (float)(int)local_70, 70.0f, 25.0f, 0.0f, 0.0f, 0.546875f, 0.78125f, '\x01', '\x01');
            }

            bVar10 = (DAT_005616ac == -1);
            // Delete char button brightness (same BUG-FIX as above — bit-pattern→float conv)
            if (bVar10) { fR = fG = fB = 0.5f; }
            else         { fR = fG = fB = 1.0f; }
            glColor3f(fR, fG, fB);

            // Left/right navigation arrows
            fVar1 = DAT_005616a8;
            local_70 = DAT_005616a8;
            GL_DrawTexture(0x12, (float)(int)DAT_005616a8, 116.0f, 205.0f, 88.0f, 0.0f, 0.0f, 0.80078125f, 0.6875f, '\x01', '\x01');
            local_70 = (float)((int)fVar1 + 0x80);
            if ((((!bVar10) && ((int)local_70 <= DAT_083a427c)) && (DAT_083a427c < (int)fVar1 + 0xc6)) &&
                ((0x93 < DAT_083a4278) && (DAT_083a4278 < 0xa7)))
            {
                GL_DrawTexture(0x13, (float)(int)local_70, 148.0f, 70.0f, 19.0f, 0.0f, 0.0f, 0.546875f, 0.59375f, '\x01', '\x01');
            }

            iVar9 = -(int)DAT_005616a8;
            local_70 = (float)(iVar9 + 0x1b2);
            GL_DrawTexture(0x14, (float)(int)local_70, 116.0f, 205.0f, 88.0f, 0.0f, 0.0f, 0.80078125f, 0.6875f, '\x01', '\x01');
            local_70 = (float)(iVar9 + 0x1b9);
            if (((!bVar10) && ((int)local_70 <= DAT_083a427c)) &&
                ((DAT_083a427c < iVar9 + 0x1ff) && ((0x93 < DAT_083a4278) && (DAT_083a4278 < 0xa7))))
            {
                GL_DrawTexture(0x15, (float)(int)local_70, 148.0f, 70.0f, 19.0f, 0.0f, 0.0f, 0.546875f, 0.59375f, '\x01', '\x01');
            }
        }

        // ── Final subsystems ──────────────────────────────────────────────────
        FUN_005239a0();   // CharSelect_UpdateInput
        FUN_0051af50();   // Chat_Render
        FUN_004f64d0();   // UI_Render
        UI_RenderNotices();   // StatusBar_Render
        if ((DAT_005590ac == 1) || (DAT_005615c0 != 5))
            UI_RenderChatLogOverlay();   // Mouse_Render
        UI_UpdateFpsCounter();
        FUN_004c3530();
        Cursor_Render();
        FUN_0051e0c0();
        GL_End2D();
        uVar13 = GL_PopMatrixAll();
        return ((uint)uVar13 & 0xFFFFFF00u) | 1u;
    }
    return (uint)uVar6 << 8;
}
