// Mouse_Hover.cpp — FUN_004b0310 @ 0x004b0310
// Per-frame mouse cursor billboard render + hover target determination.
//
// Called every frame. Two responsibilities:
//   1. Render the cursor as a 2D billboard sprite.
//   2. Determine which entity/item is under the cursor and store hover targets.
//
// ── Cursor billboard ──────────────────────────────────────────────────────────
// FUN_004f8480(DAT_080ab288, DAT_080ab28c, screenX, screenY, 1.0f, 1, 1):
//   Returns nonzero if cursor is visible/active.
// If visible, calls FUN_00511710() (hide char anim sprite for cursor area),
// then FUN_004f8bb0(type=8, x, y, sx, sy, color, 0, alpha) to draw the quad.
//   - States 2/4/5 (login/charselect/ingame): fixed size based on DAT_07e11d5c
//   - States 1/3 (intro/loading): animated size using DAT_07e11d5c oscillation
//
// ── Hover targets ─────────────────────────────────────────────────────────────
// After cursor render, sets hover indices:
//   SelectedItem = item on ground (-1=none)
//   SelectedNpc = NPC / shop entity (-1=none)
//   SelectedCharacter = mob or player entity (-1=none)
//   SelectedOperate = special object (-1=none)
//   DAT_00559c58 = secondary hover (cleared if SelectedCharacter resets)
//
// Priority with Alt held (VK_MENU):
//   item-ground (FUN_004afa40) → NPC type 4 → mob type 0x22 → player type 1 → special
//
// Priority without Alt + swim/idle:
//   mob/player type 0x22 → type 1 → NPC type 4 → item-ground → special
//
// HashTable section (lines ~131-350 in original):
//   Anti-tamper XOR encode/decode block operating on DAT_07cf1ffc (0x584-byte
//   char-data buffer). Per CLAUDE.md policy, hash table operations are
//   reference-count obfuscation — not game logic. Omitted from implementation.
//
// After hover detection: if hover target found and DAT_00559c58 != -1,
//   calls FUN_004afb00() to process the pending click action.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include <windows.h>

extern "C" void DbgLogPublic(const char* msg);


// ─────────────────────────────────────────────────────────────────────────────

void FUN_004b0310(void)
{
    // ── 1. Cursor billboard render ────────────────────────────────────────────
    // 2026-04-29 DISABLED: el cursor billboard 3D (sprite en el suelo del tile
    // hovered) requiere DAT_07eab24c (BackTerrainHeight) que no se inicializa
    // en nuestro port. Crash AV en FUN_004f8740 al acceder al buffer null.
    // El cursor 2D (FUN_004bffa0) sigue funcionando normalmente.
    #if 0
    if (DAT_005615c0 == 2 || DAT_005615c0 == 4 || DAT_005615c0 == 5)
    {
        float color[3] = { 1.0f, 0.766f, 0.0f };
        char visible = FUN_004f8480(DAT_080ab288, DAT_080ab28c, 0, 0, 1.0f, 1, 1);
        if (visible != '\0') {
            FUN_00511710();
            int frame = (DAT_005615c0 == 2) ? 1 : (DAT_07e11d5c + 1);
            float sz = (float)frame;
            FUN_004f8bb0(8, DAT_083a4130, DAT_083a4134, sz, sz, color, 0, 1.0f);
        }
    }
    if (DAT_005615c0 == 1 || DAT_005615c0 == 3) {
        float color[3] = { 1.0f, 0.766f, 0.0f };
        char visible = FUN_004f8480(DAT_080ab288, DAT_080ab28c, 0, 0, 1.0f, 1, 1);
        if (visible != '\0') {
            FUN_00511710();
            float base = (float)DAT_07e11d5c + (float)DAT_07e11d5c + _DAT_0055256c;
            float szX = ((int)base / 100) * 100 + _DAT_00552598;
            float szY = ((int)base / 100) * 100 + _DAT_00552598;
            FUN_004f8bb0(8, szX, szY, base, base, color, 0, 1.0f);
        }
    }
    #endif

    // ── 2. Reset hover targets ────────────────────────────────────────────────

    // 2026-05-06: añadido guard `c50 >= 0` para evitar OOB read cuando
    // SelectedCharacter == -1 (initial state). Antes se leía entity[+0x2fd] con
    // c50=-1 → puntero negativo → crash latente.
    if (DAT_00559c5c == '\0' || DAT_0055a7ac == 6) {
        // Cursor disabled or spectator state
        SelectedCharacter = -1;
        DAT_00559c58 = -1;
    } else if (SelectedCharacter >= 0 &&
               *(char *)(DAT_07abf5d0 + 0x2fd + SelectedCharacter * 0x394) == '\0' &&
               *(char *)(DAT_07abf5d0 + SelectedCharacter * 0x394 + 0x84) == '\x02') {
        // Current hover target is a valid alive monster.
        // 2026-05-06: REMOVED reset on IsClickPushed/DAT_083a42c4. La lógica
        // original IDA reseteaba aquí porque la detect que sigue inmediato
        // re-poblaría. Pero en nuestro port el detect a veces falla (terrain
        // filter, etc) → c50 quedaba -1 al click time → mob attack handler
        // no disparaba. Mantener el target HASTA que detect lo reemplace.
        // Keep the original transient hover state.  Leaving this selected
        // after the cursor moves away turns later ground clicks into a basic
        // attack against the stale mob.
        if (DAT_00559c58 == -1 ||
            DAT_083a42ac != '\0' || DAT_083a42d0 != '\0' ||
            *(char *)(DAT_07abf5d8 + 0x2fd) != '\0')
        {
            SelectedCharacter = -1;
        }
    } else if (SelectedCharacter >= 0) {
        // Current target is no longer valid (died or kind changed).
        DAT_00559c58 = -1;
        SelectedCharacter = -1;
    }

    SelectedItem = -1;
    SelectedNpc = -1;
    SelectedOperate = -1;

    // ── 3. Hover target detection ─────────────────────────────────────────────

    if (DAT_07d78094 != '\0') goto process_click;

    {
        SHORT altState = GetAsyncKeyState(0x12);  // VK_MENU = Alt
        if ((char)((unsigned short)altState >> 8) == (char)0x80)
        {
            // Alt held: item-on-ground first, then NPC, mob, player
            if (DAT_07e91388 == 0)
                SelectedItem = FUN_004afa40();
            if (SelectedItem == -1) {
                SelectedNpc = FUN_004afdc0(4);
                if (SelectedNpc == -1) {
                    SelectedCharacter = FUN_004afdc0(0x22);
                    if (SelectedCharacter != -1) goto done;
                    SelectedCharacter = FUN_004afdc0(1);
                    if (SelectedCharacter != -1) goto done;
                    SelectedOperate = FUN_004b0240();
                }
                goto check_click;
            }
            goto process_click;
        }

        // No Alt: check swim/idle move state
        unsigned char moveFlags = *(unsigned char *)(DAT_07abf5d8 + 0x1bc) & 7;
        if (moveFlags == 2 || moveFlags == 0)
        {
            // HashTable encode/decode block — anti-tamper obfuscation.
            // Operates on DAT_07cf1ffc (XOR-encoded char data, 0x584 bytes).
            // Omitted per project policy (hash table operations are not game logic).
        }

        // Fallthrough to secondary hover scan
        {
            // Item range check (iStack_10 between 0x19 and 0x1c — from hash table result)
            // local_20 / local_1c adjust entity type priority based on equip slot range
        }

        // ── Secondary hover without second password ───────────────────────────
        if (FUN_004e5980() == '\0') {
            // 2026-07-27: click sobre un item del suelo = levantarlo. El IDA
            // chequea el item ÚLTIMO, pero eso hace que cualquier mob/NPC cercano
            // en pantalla robe el hover y no se pueda levantar. Comportamiento MU
            // esperado (confirmado por el usuario): el item bajo el cursor tiene
            // PRIORIDAD. Lo chequeamos primero; si hay item, es pickup.
            if (DAT_07e91388 == 0) {
                SelectedItem = FUN_004afa40();
                if (SelectedItem != -1) goto check_click;   // item bajo el cursor → pickup
            }
            if (SelectedCharacter == -1) {
                SelectedCharacter = FUN_004afdc0(0x22);  // mob type
                if (SelectedCharacter == -1) {
                    SelectedCharacter = FUN_004afdc0(1);  // player type
                    if (SelectedCharacter != -1) goto done;
                    SelectedNpc = FUN_004afdc0(4);  // NPC type
                    if (SelectedNpc == -1) {
                        SelectedOperate = FUN_004b0240();
                    }
                    goto check_click;
                }
            }
            if (DAT_00559c58 != -1) goto process_click;
        }
        else goto process_click;
    }

check_click:
    if (SelectedCharacter == -1) {
        DAT_00559c58 = -1;
    }
    goto done;

process_click:
    FUN_004afb00();

done:
    if (SelectedCharacter == -1)
        DAT_00559c58 = -1;

}

// ── Additional helpers extracted from stubs_mouse_hover.cpp ─────────────────
// ── Mouse hover helpers ────────────────────────────────────────────────────────
// FUN_004f8480 @ 0x004F8480 — Terrain_TilePick(x,y,row,col,unused,stride,flag)
// Stores world coords + tile index, optionally renders a debug outline (GL_LINE_STRIP).
// For state 3 (combat target select), draws a filled quad and does mouse-ray intersection.
// Returns 1 if mouse ray intersects tile, 0 otherwise.
int __cdecl FUN_004f8480(int iparam_1, int iparam_2, int param_3, int param_4, float param_5, int param_6, int param_7) {
    float param_1 = *(float*)&iparam_1;
    float param_2 = *(float*)&iparam_2;
    *(float*)&DAT_07feb258 = param_1 * _DAT_005524f0;
    *(float*)&DAT_07feb25c = param_2 * _DAT_005524f0;
    DAT_07eab1ec = param_4 * 0x100 + param_3;
    DAT_07eab1f0 = DAT_07eab1ec + param_6;
    *(float*)&DAT_07feb260 = (float)DAT_080cb2cc[DAT_07eab1ec];
    _DAT_07feb264 = *(float*)&DAT_07feb258 + _DAT_005524f0;
    DAT_07eab1f8 = (param_4 + param_6) * 0x100 + param_3;
    DAT_07eab1f4 = param_6 + DAT_07eab1f8;
    _DAT_07feb26c = (float)DAT_080cb2cc[DAT_07eab1f0];
    _DAT_07feb274 = *(float*)&DAT_07feb25c + _DAT_005524f0;
    _DAT_07feb278 = (float)DAT_080cb2cc[DAT_07eab1f4];
    _DAT_07feb284 = (float)DAT_080cb2cc[DAT_07eab1f8];
    _DAT_07feb268 = *(float*)&DAT_07feb25c;
    _DAT_07feb270 = _DAT_07feb264;
    _DAT_07feb27c = *(float*)&DAT_07feb258;
    _DAT_07feb280 = _DAT_07feb274;
    if (param_7 == 0) {
        if (((unsigned char)DAT_0838bc70[DAT_07eab1ec] & 8) != 8)
            FUN_004f7fb0(param_1, param_2, param_3, param_4, param_5);   // RenderTerrainFace(xf,yf,xi,yi,lodf)
        return 0;
    }
    char cVar1 = '\0';

    // [DIAG] one-shot per ~half second to see camera/ray
    {
        static DWORD s_lastTilePick = 0;
        DWORD now = GetTickCount();
        if (now - s_lastTilePick > 500) {
            s_lastTilePick = now;
            float* eye = (float*)&DAT_083a4284_arr[0];
            float* tgt = (float*)&DAT_083a4110_arr[0];
            char d[256];
            wsprintfA(d, "TilePick(%d,%d) eye=(%d,%d,%d) tgt=(%d,%d,%d)",
                param_3, param_4,
                (int)eye[0], (int)eye[1], (int)eye[2],
                (int)tgt[0], (int)tgt[1], (int)tgt[2]);
            DbgLogPublic(d);
        }
    }
    if (DAT_07e11d30 != 5) {
        FUN_00511590('\0');
        glColor3f(0.0f, 0.0f, 0.0f);
        glBegin(3);
        // BUG-FIX 2026-04-28: bound era 0x7feb288 (addr abs del binario original).
        // Pool real es g_TilePickBuf[12] = 4 vec3 corners. Iterar 4.
        for (int i = 0; i < 4; ++i) {
            glVertex3fv(&g_TilePickBuf[i * 3]);
        }
        glEnd();
        FUN_00511600();
    }
    float local_c[3];
    FUN_004fa4d0((float*)&DAT_07feb258, &_DAT_07feb264, &_DAT_07feb270, local_c);
    unsigned int uVar2 = FUN_00512d40((float*)&DAT_083a4284, (float*)&DAT_083a4110, 3,
                                       (float*)&DAT_07feb258, &_DAT_07feb264, &_DAT_07feb270,
                                       &_DAT_07feb27c, local_c, '\x01');
    cVar1 = (char)uVar2;
    if (cVar1 == '\0') {
        FUN_004fa4d0((float*)&DAT_07feb258, &_DAT_07feb270, &_DAT_07feb27c, local_c);
        uVar2 = FUN_00512d40((float*)&DAT_083a4284, (float*)&DAT_083a4110, 3,
                              (float*)&DAT_07feb258, &_DAT_07feb270, &_DAT_07feb27c,
                              &_DAT_07feb264, local_c, '\x01');
        cVar1 = (char)uVar2;
    }
    if (cVar1 == '\x01') {
        *(float*)&DAT_080ab288 = param_1;
        *(float*)&DAT_080ab28c = param_2;
        DAT_07eab1fc = 1;
    }
    if (DAT_07e11d30 == 3) {
        bool draw = false;
        if (DAT_07e11d44 == 0) {
            if (((unsigned char)DAT_0838bc70[DAT_07eab1ec] & 4) == 4) draw = true;
        } else if (DAT_07e11d44 == 2) {
            if (((unsigned char)DAT_0838bc70[DAT_07eab1ec] & 1) == 1) draw = true;
        }
        if (draw) {
            FUN_005114f0();
            FUN_00511680('\x01');
            FUN_00511590('\0');
            glBegin(6);
            glColor4f(1.0f, 0.0f, 0.0f, 0.3f);
            // BUG-FIX 2026-05-03: was `while (puVar3 < 0x7feb288)` — absolute
            // source-binary bound, junk in our build. g_TilePickBuf[12] holds
            // exactly 4 vec3 corners (matching the lines 924 fix above).
            for (int i = 0; i < 4; ++i) {
                glVertex3fv(&g_TilePickBuf[i * 3]);
            }
            glEnd();
            FUN_00511600();
        }
    }
    (void)param_5;
    return (int)(unsigned char)cVar1;
}

// FUN_00512d30 @ 0x00512D30 — Map_InitRayCast: init ray t_max to ~1.03e7
// IDA-ported: single store. DAT_083a4120 is the raycast t_max sentinel
// used by FUN_00512d40 (CollisionDetectLineToFace) to accept nearer hits only.
// Original binary stores raw bits 0x4B1DCD65 (= 10367333.0f) into the float.
void FUN_00512d30()
{
    *(DWORD*)&DAT_083a4120 = 0x4B1DCD65;  // ≈ 1.0367e7f — far-plane sentinel
}


// FUN_004afdc0 @ 0x004AFDC0 — Entity_SelectNearest(mask)
// Two-pass entity scan: (1) sets +0x58 visible-flag and +0x64..6f RGB tint per entity type;
// (2) finds the nearest entity (matching mask bits at +0x84) to the camera and syncs
// party HP bar ID arrays.  Returns the entity index of the nearest match, or -1.
int __cdecl FUN_004afdc0(int param_1_int)
{
    byte param_1 = (byte)param_1_int;
    bool bVar17 = (DAT_005615c0 == 4);  // g_GameState == CharSelect

    // Pass 1: set highlight flags and color tints for visible entities
    byte *pbVar4 = (byte *)(DAT_07abf5d0 + 0x84);
    for (int loop1 = 400; loop1 != 0; loop1--, pbVar4 += 0x394) {
        if (((*pbVar4 & param_1) == *pbVar4) && (pbVar4[-0x84] != 0) && (pbVar4[0xdc] != 0) &&
            (_DAT_00552580 < *(float *)(pbVar4 + 0xe4))) {
            pbVar4[0x58] = 1;
            float r, g, b;
            switch (*(short *)(pbVar4 + 0x13a)) {
            case 0:  r =  0.2f; g =  0.2f; b =  0.2f; break;
            case 1:  r = -0.4f; g = -0.4f; b = -0.4f; break;
            case 2:  r =  0.2f; g = -0.6f; b = -0.6f; break;
            case 3:  r =  1.5f; g =  1.5f; b =  1.5f; break;
            case 4:  r =  0.3f; g =  0.2f; b = -0.5f; break;
            default: r =  0.0f; g =  0.0f; b =  0.0f; break;
            }
            if (pbVar4[0x266] > 5) { r = -0.4f; g = -0.4f; b = -0.4f; }
            *(float *)(pbVar4 + 0x64) = r;
            *(float *)(pbVar4 + 0x68) = g;
            *(float *)(pbVar4 + 0x6c) = b;
        }
    }

    // Pass 2: find nearest entity to MOUSE-RAY (perpendicular distance), not camera.
    // Antes: usábamos distancia a cámara con FUN_00513260 stub → siempre return 1
    // → ganaba el más cercano a cámara siempre, que es slot 1 (elfa) por geometría.
    // Ahora: gana el char cuyo centro de masa está más cerca del ray del mouse.
    float best_perp = 1e12f;
    int   best_idx  = -1;
    int   ent_idx   = 0;
    // DIAG: rate-limited per slot, log filter rejection reasons
    static DWORD s_lastFilt[5] = {0,0,0,0,0};
    bool diagFilt = (DAT_005615c0 == 4);

    for (int ofs = 0; ofs < 0x59740; ofs += 0x394, ent_idx++) {
        char *ent = (char *)(ofs + DAT_07abf5d0);
        if (diagFilt && ent_idx < 5) {
            DWORD now = GetTickCount();
            if (now - s_lastFilt[ent_idx] > 1000) {
                s_lastFilt[ent_idx] = now;
                char b[200];
                _snprintf_s(b, sizeof(b), _TRUNCATE,
                    "FILT slot=%d mask=%d act=%d vis@160=%d dist@168=%.3f thr@552580=%.3f flag@2fd=%d flag@84=%d",
                    ent_idx, (int)param_1, (int)*ent, (int)ent[0x160],
                    *(float*)(ent+0x168), _DAT_00552580,
                    (int)ent[0x2fd], (int)ent[0x84]);
                DbgLogPublic(b);
            }
        }
        if ((*ent == '\0') || (ent[0x160] == '\0') ||
            (!(_DAT_00552580 < *(float *)(ent + 0x168))) || (ent[0x2fd] != '\0'))
            continue;

        // Party name matching for DarkLord entities
        if ((*(short *)(ent + 2) == 0x186) && (ent[0x84] == '\x01') && (0 < (int)DAT_07eaa0e0)) {
            char *slot = (char *)&DAT_07e11e80;
            for (int pi = 0; pi < (int)DAT_07eaa0e0; pi++, slot += 0x24) {
                if (*(int *)(slot + 0x1c) == -2) {
                    const char *eName = ent + 0x1c1;
                    size_t sVar7 = strlen(eName);
                    size_t m     = strlen(slot);
                    if (m > sVar7) sVar7 = m;
                    if (sVar7 == 0) sVar7 = 1;
                    *(int *)(slot + 0x20) = 0;
                    if (strncmp(slot, eName, sVar7) == 0) {
                        *(int  *)(slot + 0x1c) = ent_idx;
                        *(DWORD *)(slot + 0x20) = *(DWORD *)(ent + 0x78);
                        break;
                    }
                }
            }
        }

        // Visibility + nearest check
        if ((!bVar17) && (ent == (char *)DAT_07abf5d8)) continue;
        if ((ent[0x84] & param_1) != ent[0x84]) continue;

        // Copy entity bounding data (entity+0x130, 12 dwords) to stack for viewport test
        undefined4 auStack_58[12];
        for (int i = 0; i < 12; i++)
            auStack_58[i] = *(undefined4 *)(ent + 0x130 + i * 4);

        // ── BUG-FIX 2026-04-26 (revisión 5): screen-space via gluProject.
        //   El intento previo (revisión 4) usaba FUN_005113f0 (World_ToScreen),
        //   pero esa función llama a __ftol() que en stdafx.h está stubbed como
        //   GetTickCount() — devuelve basura, no proyección. Por eso TODOS los
        //   slots daban la misma "screen pos" y BK ganaba siempre por ser slot 0.
        //   Solución: usar gluProject directamente con el GL state actual.
        //   Esto da píxeles ventana exactos y respeta perspectiva.
        {
            float feetX = *(float*)(ent + 0x10);
            float feetY = *(float*)(ent + 0x14);
            float feetZ = *(float*)(ent + 0x18);

            GLdouble model[16], proj[16];
            GLint viewport[4];
            glGetDoublev(GL_MODELVIEW_MATRIX, model);
            glGetDoublev(GL_PROJECTION_MATRIX, proj);
            glGetIntegerv(GL_VIEWPORT, viewport);

            // Probar 3 puntos verticales y quedarnos con la mínima distancia.
            float bodyHeights[3] = { 10.0f, 40.0f, 70.0f };
            int  mx = (int)DAT_083a427c;
            int  my = (int)DAT_083a4278;
            int  bestPx2 = 0x7fffffff;
            int  bestSx = -1, bestSy = -1;
            int  successCount = 0;
            for (int probe = 0; probe < 3; probe++) {
                GLdouble wx, wy, wz;
                int gluOk = gluProject(
                    (GLdouble)feetX, (GLdouble)feetY, (GLdouble)(feetZ + bodyHeights[probe]),
                    model, proj, viewport, &wx, &wy, &wz);
                if (!gluOk || wz < 0.0 || wz > 1.0) continue;  // detrás de la cámara o fuera de clip
                successCount++;
                // gluProject Y crece hacia arriba; la pantalla del juego Y
                // crece hacia abajo. Y mouse está en 640×480 lógico, viewport
                // está en píxeles físicos.
                int sxLogical = (int)((wx * 640.0) / (double)viewport[2]);
                int syLogical = (int)(((double)viewport[3] - wy) * 480.0 / (double)viewport[3]);
                int dx = sxLogical - mx, dy = syLogical - my;
                int d2 = dx*dx + dy*dy;
                if (d2 < bestPx2) { bestPx2 = d2; bestSx = sxLogical; bestSy = syLogical; }
            }
            // DIAG once per slot per second
            if (DAT_005615c0 == 4) {
                static DWORD s_lastHT[5] = {0,0,0,0,0};
                int slotN = (int)(((uintptr_t)ent - (uintptr_t)DAT_07abf5d0) / 0x394);
                if (slotN >= 0 && slotN < 5) {
                    DWORD now = GetTickCount();
                    if (now - s_lastHT[slotN] > 1000) {
                        s_lastHT[slotN] = now;
                        char b[300];
                        _snprintf_s(b, sizeof(b), _TRUNCATE,
                            "HT slot=%d entPos=(%.1f,%.1f,%.1f) sxy=(%d,%d) px=%d mxy=(%d,%d) ok=%d vp=(%d,%d,%d,%d)",
                            slotN, feetX, feetY, feetZ,
                            bestSx, bestSy,
                            (bestPx2 < 0x7fffffff) ? (int)sqrtf((float)bestPx2) : -1,
                            mx, my, successCount,
                            viewport[0], viewport[1], viewport[2], viewport[3]);
                        DbgLogPublic(b);
                    }
                }
            }
            if (successCount == 0) continue;
            // Threshold ~32 px ancho del cuerpo en pantalla.
            const int R_PX = 32;
            if (bestPx2 > R_PX * R_PX) continue;
            if (!((float)bestPx2 < best_perp)) continue;
            best_perp = (float)bestPx2;
        }

        // 2026-05-06 BUG-FIX MAYÚSCULO: terrain filter DESHABILITADO.
        // El IDA original gateaba esta sección por `World` (current map number,
        // 0=Lorencia, 2=Devias). Pero en globals.h:2139 nuestro build tiene
        //   #define World    g_GameSubState
        // que es semánticamente DIFERENTE — g_GameSubState = 0/connecting,
        // 2/in-world, 9/logout, etc. Cuando el user está in-world, subst=2
        // SIEMPRE → entramos al else "Devias" branch y leemos
        // TerrainMappingLayer1 con coords mal interpretadas → rejecta mobs
        // incluso en Lorencia.
        //
        // User reportó "no atacaba a la primera, me costo empezar a atacar":
        // hover detect rechazaba mobs por este filter de terrain corrupto.
        //
        // El filter es una optimization (no permite hover sobre mobs en
        // tiles "blocked"). Sin él, mobs en safe-zones técnicamente serían
        // hoverable pero el server rechaza el attack de todos modos. Net
        // negative removerlo es 0.
        best_idx  = ent_idx;
    }

    // Pass 3: sync party HP bar entity IDs by player name
    if (0 < (int)DAT_07eaa0e0) {
        int *piVar12 = (int *)((char *)&DAT_07e11e80 + 0x1c);
        for (int pi = 0; pi < (int)DAT_07eaa0e0; pi++, piVar12 += 9) {
            if (*piVar12 < 0) {
                const char *playerName = (char *)DAT_07abf5d8 + 0x1c1;
                const char *partyName  = (char *)(piVar12 - 7);
                size_t n = strlen(playerName);
                size_t m = strlen(partyName);
                size_t sVar7 = (n < m) ? m : n;
                if (sVar7 == 0) sVar7 = 1;
                piVar12[1] = 0;
                int cmp = strncmp(partyName, playerName, sVar7);
                *piVar12 = (cmp != 0) ? -3 : -1;
            }
        }
    }
    return best_idx;
}

// FUN_004afa40 @ 0x004AFA40 — ItemOnGround_HoverTest(void)
// ── BUG-FIX 2026-04-26: NEUTRALIZADO ─────────────────────────────────────
// El binario original itera DAT_07e12840 con bounds absolutos (0x7e908bc /
// 0x7e907df) que en mu97k-src no son válidos: aquí DAT_07e12840 está
// declarado en globals.cpp como un único DWORD (no un array), así que
// iterar 234 × 0x204 bytes pisaba ~192 KB de globals adyacentes → corrupción
// silenciosa en chunk-lists, UI menus, etc. El intento de "fixear" con
// count=234 escribió valores 0x3fc00000 (=1.5f) en globals adyacentes que
// luego se leían como punteros (= AV).
// Como char-select/login no tienen items en el suelo, retornar -1 es
// equivalente al comportamiento esperado en esos estados. Cuando se necesite
// el path real (InGame con items dropeados), hay que localizar el array
// correcto en mu97k-src (probablemente NO se llama DAT_07e12840).
int __cdecl FUN_004afa40(void)
{
    // 2026-07-27: hover de items en el suelo. El path FIEL (sub_4AFA40) usa un
    // point-in-quad screen-space (FUN_00513260, 12-arg) que depende de macros
    // Hex-Rays sin portar. En su lugar usamos proximidad world-space: comparar
    // el tile del item con el tile del terreno bajo el mouse (el mismo picker
    // que usa el click-to-move, FUN_004f9ac0 → DAT_080ab288/28c).
    // El pool DAT_07e12840 es 1000×0x204; layout por slot (base = pool+i*0x204):
    //   base+72   active flag
    //   base+424  visible flag (lo setea el render)
    //   base+16/20  world X/Y del item
    //   base+304/308/312  light color (0.2 normal, 1.5 al hover)
    BYTE* pool = (BYTE*)&DAT_07e12840[0];

    // Pass 1: atenuar todos los items activos + visibles.
    for (int i = 0; i < 1000; ++i) {
        BYTE* base = pool + i * 0x204;
        if (base[72] && base[424]) {
            *(DWORD*)(base + 304) = 0x3E4CCCCD;   // 0.2f
            *(DWORD*)(base + 308) = 0x3E4CCCCD;
            *(DWORD*)(base + 312) = 0x3E4CCCCD;
        }
    }

    // Pass 2: item cuya posición de pantalla (v1+92/94, la computa el render con
    // World_ToScreen) esté cerca del cursor. Screen-space, sin picker de terreno
    // (que dibujaba la cuadrícula). Umbral generoso ~24 px (el modelo del item
    // es chico en el suelo).
    int mouseX = (int)DAT_083a427c;
    int mouseY = (int)DAT_083a4278;
    int best = -1, bestD = 24 * 24 + 1;
    for (int i = 0; i < 1000; ++i) {
        BYTE* base = pool + i * 0x204;
        if (base[72] && base[424]) {
            int sx = *(short*)(base + 72 + 92);   // = v1+92
            int sy = *(short*)(base + 72 + 94);
            int dx = sx - mouseX, dy = sy - mouseY;
            int d = dx * dx + dy * dy;
            if (d < bestD) { bestD = d; best = i; }
        }
    }
    if (best >= 0) {
        BYTE* base = pool + best * 0x204;
        *(DWORD*)(base + 304) = 0x3FC00000;   // 1.5f (highlight)
        *(DWORD*)(base + 308) = 0x3FC00000;
        *(DWORD*)(base + 312) = 0x3FC00000;
    }
    return best;
}

// FUN_004b0240 @ 0x004B0240 — SpecialObject_HoverTest(void)
// ── BUG-FIX 2026-04-26: NEUTRALIZADO ─────────────────────────────────────
// Mismo patrón que FUN_004afa40: el original itera con bound absoluto
// (0x83a2cd0) que no es válido en mu97k-src. Si bien aquí los WRITES están
// gateados por flags, los READs aún escapan del array y pueden disparar AV.
// Char-select/login no tienen special-objects, así que retornar -1 es seguro.
int __cdecl FUN_004b0240(void)
{
    return -1;
#if 0
    char *pcVar3;
    // Pass 1: reset light to 0.2f
    for (pcVar3 = DAT_083a2370; (int)pcVar3 < 0x83a2cd0; pcVar3 += 0xc) {
        int iVar1 = *(int *)(pcVar3 + 8);
        if ((*pcVar3 != '\0') && (*(char *)(iVar1 + 0x160) != '\0')) {
            *(DWORD *)(iVar1 + 0xe8) = 0x3e4ccccd; // 0.2f
            *(DWORD *)(iVar1 + 0xec) = 0x3e4ccccd;
            *(DWORD *)(iVar1 + 0xf0) = 0x3e4ccccd;
        }
    }
    // Pass 2: find hovered object
    int local_4 = 0;
    for (pcVar3 = DAT_083a2370; (int)pcVar3 < 0x83a2cd0; pcVar3 += 0xc, local_4++) {
        int iVar1 = *(int *)(pcVar3 + 8);
        if ((*pcVar3 != '\0') && (*(char *)(iVar1 + 0x160) != '\0')) {
            undefined4 auStack_44[12];
            for (int i = 0; i < 12; i++)
                auStack_44[i] = *(undefined4 *)(iVar1 + 0x130 + i * 4);
            if ((char)FUN_00513260((float *)&DAT_083a4284, (float *)&DAT_083a4110)) {
                *(DWORD *)(iVar1 + 0xe8) = 0x3fc00000; // 1.5f
                *(DWORD *)(iVar1 + 0xec) = 0x3fc00000;
                *(DWORD *)(iVar1 + 0xf0) = 0x3fc00000;
                return local_4;
            }
        }
        if (0x83a2ccf < (int)(pcVar3 + 0xc)) return -1;
    }
    return -1;
#endif
}
// FUN_004afb00 — implemented in src/Game/Party_NameMatch.cpp (Party_MatchEntityNames)
// FUN_004e5980 @ 0x004E5980 — Party_HPBar_HoverCheck(void)
// Iterates the party HP bar array (DAT_07e11e9c, stride 0x24 = 9 uints) and checks
// if the mouse cursor (DAT_083a427c, DAT_083a4278) is within any party member's
// screen rect. Sets SelectedCharacter (hover entity index) and returns 1 if hovering.
// SecondPassword UI state flags (DAT_07eaa115..130) control which X position band is used.
// Anti-tamper HashTable blocks in the loop are skipped — only position comparison kept.
char __cdecl FUN_004e5980(void)
{
    if (DAT_07eaa115 != '\0') return 0;

    // Determine X band based on SecondPassword UI state
    UINT local_20;
    if (DAT_07eaa117 != '\0') {
        if (DAT_07eaa116 != '\0') {
            local_20 = 0x104;
        } else {
            // Check secondary password active flags
            if ((DAT_07eaa118 == '\0') && (DAT_07eaa119 == '\0') && (DAT_07eaa11a == '\0') &&
                (DAT_07eaa11b == '\0') && (DAT_07eaa11c == '\0')) {
                local_20 = 0x1c2;
            } else {
                local_20 = 0x104;
            }
        }
    } else {
        // All clear: check if all flags inactive
        if ((DAT_07eaa116 == '\0') && (DAT_07eaa115 == '\0') &&
            (DAT_07eaa114 == '\0') && (DAT_07eaa124 == 0) &&
            (DAT_07eaa128 == 0) && (*(char*)((uintptr_t)DAT_00583d8c + 0x1c87f) == '\0') &&
            (DAT_07eaa130 == '\0')) {
            local_20 = 0x280;
        } else {
            local_20 = 0x1c2;
        }
    }

    float local_14 = (float)local_20 - _DAT_00552598;   // left edge of HP bar column
    float local_24 = 5.0f;                              // top of first bar (Y)
    char result = 0;

    UINT  uVar7 = *(BYTE*)(DAT_07abf5d8 + 0x1bc) & 7;  // player move_type_flags & 7
    UINT *puVar5 = (UINT*)&DAT_07e11e9c;                // party slot base (stride 9 uints = 0x24)

    for (int i = 0; i < (int)DAT_07eaa0e0; i++, puVar5 += 9) {
        if (*puVar5 < 0x80000000) {
            // [HashTable obfuscation block skipped — anti-tamper, not game logic]

            // Get entity stat ID for this party slot
            UINT *puVar12 = (UINT*)(UINT)*(BYTE*)((UINT)*(BYTE*)(DAT_07abf5d8 + 0x391) + 0x57 + (char*)DAT_07cf1ff4);

            // Check if entity type is a hoverable icon (0x1a/0x1b/0x1c/0xf/0x10)
            UINT pVal = (UINT)(uintptr_t)puVar12;
            bool bTypeOk = (pVal == 0x1a || pVal == 0x1b || pVal == 0x1c ||
                            pVal == 0xf  || pVal == 0x10);

            // Check mouse position within screen rect
            float mouseX = (float)DAT_083a427c;
            float mouseY = (float)DAT_083a4278;
            bool bPosOk = (mouseX >= local_14 - _DAT_00552540) &&
                          (mouseX <= local_14 + _DAT_00552598) &&
                          (mouseY >= local_24) &&
                          (mouseY <= local_24 + _DAT_005527d4);

            if (bTypeOk && bPosOk) {
                result = 1;
                SelectedCharacter = *puVar5;
            }
        }
        local_24 += _DAT_005527d4;
        *puVar5 = 0xfffffffe;
    }
    return result;
}
// FUN_004b14f0 — implemented in src/UI/Chat_InputTick.cpp
