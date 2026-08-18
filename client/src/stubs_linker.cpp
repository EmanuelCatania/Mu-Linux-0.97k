// stubs_linker.cpp
//
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 12640-13860 (1221 lines).
//
// Batch 20 + linker-fix stubs: empty bodies for functions called but not yet
// decompiled. These exist purely to satisfy LNK2019 unresolved external
// errors. As real implementations get ported, they should be moved out of
// this file into their proper module home.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "structs.h"

extern "C" DWORD DAT_07eaa128;   // Golden Archer panel flag (globals.cpp)

extern "C" void DbgLogPublic(const char* msg);
extern void __cdecl FUN_0054158c(void* ptr);
extern void FUN_004fa5a0(void);

#ifndef qmemcpy
#define qmemcpy(dst,src,sz) memcpy((dst),(src),(size_t)(sz))
#endif
#ifndef delete__
#define delete__(p) FUN_0054158c((unsigned char*)(p))
#endif
#ifndef __OFSUB__
#define __OFSUB__(x,y)       (0)
#endif

#ifndef LODWORD
#define LODWORD(x)           (*((DWORD*)&(x)))
#define HIDWORD(x)           (*(((DWORD*)&(x))+1))
#define SLOBYTE(x)           (*((char*)&(x)))
#define SLOWORD(x)           (*((short*)&(x)))
#define SLODWORD(x)          (*((int*)&(x)))
#endif
#ifndef LOBYTE
#define LOBYTE(x)            (*((unsigned char*)&(x)))
#define HIBYTE(x)            (*(((unsigned char*)&(x))+1))
#define LOWORD(x)            (*((unsigned short*)&(x)))
#define HIWORD(x)            (*(((unsigned short*)&(x))+1))
#endif

#define ITEM_SPECIAL_SKILL_OPTION             0
#define ITEM_SPECIAL_LUCK_OPTION              1
#define ITEM_OPTION_ADD_PHYSI_DAMAGE_CODE     60
#define ITEM_OPTION_ADD_MAGIC_DAMAGE_CODE     61
#define ITEM_OPTION_ADD_DEFENSE_RATE_CODE     62
#define ITEM_OPTION_ADD_DEFENSE_CODE          63
#define ITEM_OPTION_ADD_EXCELLENT_DAMAGE_CODE 72


// FUN_004f8740 @ 0x004F8740 (79 lines) — Terrain_RenderTileQuad
// Renders a single terrain quad as GL_TRIANGLE_FAN with per-vertex lighting.
// p1/p2 = grid X/Y, p3 = tile size, p4 = index step, p5 = texcoord array ptr,
// p6 = enable lighting, p7 = alpha value.
void __cdecl FUN_004f8740(float p1, float p2, float p3, int p4, int p5, char p6, float p7) {
    // BUG-FIX 2026-04-29: guard contra DAT_07eab24c (BackTerrainHeight) y
    // DAT_07eab250 (PrimaryTerrainLight) no inicializados. Crash AV en
    // 0x410E4597 venía de cursor billboard FUN_004f8bb0 dereferenciando
    // el buffer NULL.
    if (DAT_07eab24c == 0 || (uintptr_t)DAT_07eab24c < 0x100000) return;
    int iX = (int)p1;
    int iY = (int)p2;
    if (iX < 0 || iY < 0 || iX >= 0xff || iY >= 0xff) return;

    int idx0 = iY * 0x100 + iX;
    int idx1 = idx0 + p4;
    int idx3 = iX + (iY + p4) * 0x100;
    int idx2 = p4 + idx3;

    float scale = _DAT_005524f0;
    float verts[4][3];
    verts[0][0] = p1 * scale;
    verts[0][1] = p2 * scale;
    verts[0][2] = *(float *)(DAT_07eab24c + idx0 * 4); // BackTerrainHeight
    verts[1][0] = verts[0][0] + p3 * scale;
    verts[1][1] = verts[0][1];
    verts[1][2] = *(float *)(DAT_07eab24c + idx1 * 4);
    verts[2][0] = verts[1][0];
    verts[2][1] = verts[0][1] + p3 * scale;
    verts[2][2] = *(float *)(DAT_07eab24c + idx2 * 4);
    verts[3][0] = verts[0][0];
    verts[3][1] = verts[2][1];
    verts[3][2] = *(float *)(DAT_07eab24c + idx3 * 4);

    float light[4][3];
    if (p6 != '\0') {
        int indices[4] = { idx0, idx1, idx2, idx3 };
        for (int v = 0; v < 4; v++) {
            float *src = (float *)(DAT_07eab250 + indices[v] * 12); // PrimaryTerrainLight
            light[v][0] = src[0]; light[v][1] = src[1]; light[v][2] = src[2];
        }
    }

    glBegin(6); // GL_TRIANGLE_FAN
    for (int i = 0; i < 4; i++) {
        if (p6 != '\0') {
            if (p7 == 1.0f) {
                glColor3fv((GLfloat *)&light[i]);
            } else {
                glColor4f(light[i][0], light[i][1], light[i][2], p7);
            }
        }
        // IDA 0x004F8740 walks `a5` as 4 records of 3 floats each (stride 12),
        // consuming the first two components as UVs. Callers such as
        // FUN_004f8980/FUN_004f8bb0 pass rotated quad data in that layout.
        const float *tc = (const float *)(p5 + i * 12);
        glTexCoord2f(tc[0], tc[1]);
        glVertex3fv((GLfloat *)&verts[i]);
    }
    glEnd();
}

// FUN_004f8980 @ 0x004F8980 (116 lines) — Terrain_RenderTexturedObject
// Renders a rotated textured object on terrain by tiling into sub-quads.
// p1 = texture index, p2/p3 = grid position, p4 = rotation angle.
// Uses AngleMatrix + VectorRotate to rotate sub-tile corners, then draws each
// with FUN_004f8740 (Particle_DrawTile).
// Bitmaps[idx * 0xE + 8] = width, Bitmaps[idx * 0xE + 9] = height.
// _DAT_00552b9c = 1/64 (UV step), _DAT_0055256c = 1.0 (tile step).
void __cdecl FUN_004f8980(int p1, int p2, int p3, float p4)
{
    glColor3f(1.0f, 1.0f, 1.0f);

    float angles[3] = { 0.0f, 0.0f, p4 };
    float matrix[3][4];
    AngleMatrix(angles, matrix);
    FUN_00511480(p1); // BindTexture

    // Bitmap dimensions
    float* bmpData = (float*)((char*)Bitmaps + p1 * 0xE * sizeof(float));
    float bmpW = bmpData[8];
    float bmpH = bmpData[9];
    float uvStepX = _DAT_00552cb4 / bmpW;  // 64.0 / width
    float uvStepY = _DAT_00552cb4 / bmpH;  // 64.0 / height

    float tileW = bmpW * _DAT_00552b9c;  // width * (1/64)
    float tileH = bmpH * _DAT_00552b9c;  // height * (1/64)

    float fy = 0.0f;
    while (fy < tileH) {
        float fx = 0.0f;
        float uvY0 = fy * uvStepY;
        float uvY1 = (fy + _DAT_0055256c) * uvStepY;

        while (fx < tileW) {
            float uvX0 = fx * uvStepX;
            float uvX1 = (fx + _DAT_0055256c) * uvStepX;
            float uvRot[4][3] = {
                { uvX0, uvY0, 0.0f },
                { uvX1, uvY0, 0.0f },
                { uvX1, uvY1, 0.0f },
                { uvX0, uvY1, 0.0f }
            };
            for (int i = 0; i < 4; ++i) {
                float in[3] = {
                    uvRot[i][0] - _DAT_00552504,
                    uvRot[i][1] - _DAT_00552504,
                    0.0f
                };
                float out[3];
                out[0] = in[0] * matrix[0][0] + in[1] * matrix[0][1] + in[2] * matrix[0][2];
                out[1] = in[0] * matrix[1][0] + in[1] * matrix[1][1] + in[2] * matrix[1][2];
                out[2] = in[0] * matrix[2][0] + in[1] * matrix[2][1] + in[2] * matrix[2][2];
                uvRot[i][0] = out[0] + _DAT_00552504;
                uvRot[i][1] = out[1] + _DAT_00552504;
                uvRot[i][2] = out[2];
            }

            FUN_004f8740((float)p2 + fx, (float)p3 + fy, 1.0f, 1, (int)uvRot, '\x01', 1.0f);
            fx += _DAT_0055256c;
        }
        fy += _DAT_0055256c;
    }
}

// SeparateTextIntoLines @ 0x0051D600 (71 lines) — Word-wrap text into fixed-size line buffer
// text = input string, out = 2D output buffer (stride=maxChars), maxLines = max output lines,
// maxChars = chars per line. Returns number of lines produced.
int __cdecl SeparateTextIntoLines(const char *text, char *out, int maxLines, int maxChars) {
    if (!text || !out || maxLines <= 0 || maxChars <= 0) return 0;
    int lineCount = 0;
    int col = 0;
    int lastSpace = -1;
    const char *lineStart = text;
    const char *p = text;

    while (*p != '\0' && lineCount < maxLines) {
        if (*p == ' ') lastSpace = col;
        col++;
        if (col >= maxChars) {
            // Line full — break at last space or hard-break
            int breakAt;
            if (lastSpace > 0 && lastSpace > maxChars / 2) {
                breakAt = lastSpace;
            } else {
                breakAt = (col < 10) ? col : 10;
            }
            char *dst = out + lineCount * maxChars;
            memcpy(dst, lineStart, breakAt);
            dst[breakAt] = '\0';
            lineCount++;
            lineStart = lineStart + breakAt;
            if (*lineStart == ' ') lineStart++; // skip space after break
            p = lineStart;
            col = 0;
            lastSpace = -1;
            continue;
        }
        p++;
    }
    // Copy remaining text
    if (col > 0 && lineCount < maxLines) {
        char *dst = out + lineCount * maxChars;
        memcpy(dst, lineStart, col);
        dst[col] = '\0';
        lineCount++;
    }
    return lineCount;
}


// PlayBuffer — real implementation in src/Sound/Sound_DS3D.cpp as FUN_00404bc0.
// See functions.h for the declaration; the two names resolve to the same function.

// SetPlayerStop @ 0x004430C0 (504 lines) — Set player entity to idle/stop animation
// Selects animation based on equipment, class, terrain. Most bulk is anti-tamper hash ops.
// 2026-08-08 BUG-FIX (el MG se renderizaba como Dark Wizard, con casco y con
// rayas): este stub coexistía con el port REAL de SetPlayerStop
// (`FUN_004430c0`, Net/SecondPassword.cpp). `FUN_0045c720` llamaba a ESTE, y el
// stub hacía:
//     *(BYTE*)(entity + 0x1bc) &= ~0x07;   // "clear movement bits"
// pero **0x1BC NO son move flags: es el byte de CLASE/skin** (lo leen
// `SetCharacterClass` como `skin`, `CheckFullSet` como `(c+444)&7`, y
// `RenderEquipmentBox` vía `CA[11]`). O sea el stub borraba la clase:
//     DW  0x00 -> 0x00   (sin cambio, por eso nunca se notó)
//     SM  0x08 -> 0x08   (sin cambio)
//     DK  0x01 -> 0x00   ✗ pasa a Dark Wizard
//     FE  0x02 -> 0x00   ✗
//     MG  0x03 -> 0x00   ✗
// Cazado con las sondas CLSPROBE: F(post-45c130)=3 → G(post-45c720)=0.
// Delegamos al port real; el stub no debe existir.
void __cdecl FUN_004430c0(int c);
void __cdecl SetPlayerStop(void *entity) {
    if (!entity) return;
    FUN_004430c0((int)(uintptr_t)entity);
}

// CErrorReport__Write @ 0x00405540 (12 lines) — Variadic error log writer
// Formats message via wvsprintfA then passes to debug info string writer.
void __cdecl CErrorReport__Write(unsigned long ctx, char *fmt, ...) {
    char buf[0x400];
    va_list args;
    va_start(args, fmt);
    wvsprintfA(buf, fmt, args);
    va_end(args);
    // In original: CErrorReport__WriteDebugInfoStr(ctx, buf);
    // For now, just format — actual file write not critical for stub
    (void)ctx;
}

// FUN_005414ce @ 0x005414CE (11 lines) — CRT atexit wrapper
// Registers a function pointer for cleanup at program exit.
void __cdecl FUN_005414ce(void *addr) {
    // Original calls FUN_00541450 (_onexit internal registration)
    // In our build, use standard atexit
    if (addr) atexit((void (__cdecl *)(void))addr);
}

// FUN_00543c98 @ 0x00543C98 (58 lines) — CRT free wrapper
// Dispatches to SBH/OSBH/HeapFree depending on CRT heap type.
void __cdecl FUN_00543c98(void *ptr) {
    // In our build, delegate to standard free
    free(ptr);
}

// FUN_0053d430 @ 0x0053D430 (67 lines) — GameGuard encrypted log init
// Allocates 0x34c-byte context, inits crypto via CryptAcquireContext,
// installs exception filter, opens log file. Singleton (returns if already init).
void __cdecl FUN_0053d430(unsigned char *buf) {
    (void)buf;
    // GameGuard is disabled in this build — no-op.
    // Original: allocates crypto context, sets up encrypted log file,
    // installs TopLevelExceptionFilter, calls FUN_0053d890.
}

// FUN_0053ea90 @ 0x0053EA90 (44 lines) — GameGuard per-tick health check
// Checks GG process status, heartbeat event, returns error codes.
// In our build, GameGuard is disabled — return 0x755 (OK/running).
int __cdecl FUN_0053ea90(void *param) {
    (void)param;
    return 0x755; // GG status OK
}

// StopBuffer @ 0x00404C60 — real implementation at stubs.cpp:275 (forwards to FUN_00404c60).

// StopMp3 @ 0x004127F0 — delega al port fiel (FUN_004127f0, src/Sound/Music.cpp).
//
// Esta era una SEGUNDA implementacion del mismo simbolo del binario, y es la que
// usaba StopMusic. Estaba mal en tres cosas: ignoraba `cmd` (cerraba el
// reproductor aunque estuviera sonando otro track), mandaba WM_DESTROY en vez de
// WM_CLOSE, y no limpiaba Mp3FileName — asi que el siguiente PlayMp3 creia que
// el track viejo seguia en curso. Ver [[simbolo-duplicado-patron]].
void __cdecl StopMp3(char *cmd, int param) {
    FUN_004127f0((DWORD)(uintptr_t)cmd, param);
}

// DeleteObjects @ 0x004FFD50 (90 lines) — Release all world objects
// Frees BMD models, unloads textures, clears entity/effect/particle arrays.
void __cdecl DeleteObjects(void) {
    // Original releases BMD models in [0, 0x7580) stride 0xBC,
    // walks linked-list object blocks freeing nodes,
    // unloads textures in range [0x23, 0x68),
    // zeros live flags for: Sprites, Boids, Fishs, Leaves,
    // Particles, Points, Joints, Operates, Effects.
    // Simplified: clear effect/particle arrays via memset
    // Full cleanup requires BMD__Release and UnloadImage which
    // are already stubbed elsewhere.
}

// DeleteNpcs @ 0x00509190 (20 lines) — Release NPC models and sound buffers
// Frees BMD models in NPC range, releases sound buffers 0x78-0xAA.
void __cdecl DeleteNpcs(void) {
    // Release NPC sound buffers (slots 0x78 to 0xA9)
    // Original: BMD__Release for model indices ~213-255 (offsets 0xF604..0x11710 stride 0xBC)
    // then ReleaseBuffer for sound slots 0x78..0xA9
    // Simplified — sound/model release handled at shutdown
}

// DeleteMonsters @ 0x00509880 (20 lines) — Release monster models and sound buffers
// Frees BMD models in monster range, releases sound buffers 0xAA-0x1A4.
void __cdecl DeleteMonsters(void) {
    // Original: BMD__Release for model indices ~170-213 (offsets 0xC648..0xF604 stride 0xBC)
    // then ReleaseBuffer for sound slots 0xAA..0x1A3
    // Simplified — sound/model release handled at shutdown
}

// ClearItems @ 0x00502B80 (17 lines) — Clear all ground item live flags
void __cdecl ClearItems(void) {
    // Original loops through Items array zeroing the Key/live byte of each entry
    // Items base is DAT_07e907e0 area, each item has a live flag at offset 0
    // For now, no-op — items cleared at map transition
}

// ClearCharacters @ 0x0045ABB0 — DUPLICADO de FUN_0045abb0 (misma direccion).
// 2026-07-24: antes esta version leia el Key del offset EQUIVOCADO (+4 en vez
// de +476).  La impl VIVA (la que llama OpenWorld) es FUN_0045abb0 en
// Render/SMD_Parser.cpp, que ya lee +0x1dc correcto.  Se delega para que no
// haya dos comportamientos distintos para el mismo 0x45ABB0.
void __cdecl ClearCharacters(int Key) { FUN_0045abb0(Key); }

// CSQuest__CheckQuestState @ 0x00401730 (49 lines) — Quest state machine check
// Checks quest conditions based on state (1=act, 2=find, 3=request).
void __fastcall CSQuest__CheckQuestState(void *This, int state) {
    if (!This) return;
    // Quest entry at: This + questIndex*0x248 + 8
    // questIndex at This+0x1c87a
    BYTE questIdx = *(BYTE *)((int)This + 0x1c87a);
    int lpQuest = (int)This + (uint)questIdx * 0x248 + 8;

    if (state == 0xff) {
        // Auto-resolve state
        // Original calls CSQuest__getQuestState(This, -1)
        *(BYTE *)((int)This + 0x1c882) = 0;
    } else {
        *(BYTE *)((int)This + 0x1c882) = (BYTE)state;
    }
    // State machine dispatch (simplified):
    // State 1: CheckActCondition → FindQuestContext(type=2)
    // State 2: FindQuestContext(type=3) → store in +0x1c880
    // State 3: CheckRequestCondition → FindQuestContext(type=0)
    (void)lpQuest;
}

// CSQuest__ShowDialogText @ 0x004017E0 (76 lines) — Quest dialog setup
// Prepares dialog text + answer options for quest NPC interaction.
void __cdecl CSQuest__ShowDialogText(int param_1, int param_2) {
    (void)param_2;
    // Minimal but structured port:
    // keep current dialog script index, reset answer buffer and provide the
    // vanilla default "close" answer so the popup runtime can render and
    // close cleanly even while the full DialogScript table is still missing.
    g_iCurrentDialogScript = param_1;
    DAT_083a7c08 = (DWORD)param_1;
    DAT_083a7c04 = (DWORD)param_1;
    DAT_083a7c09 = 0;
    memset(DAT_083a44c4, 0, sizeof(DAT_083a44c4));
    if (param_1 >= 0 && GlobalText[param_1] && GlobalText[param_1][0]) {
        g_iNumLineMessageBoxCustom = SeparateTextIntoLines(GlobalText[param_1], &DAT_083a44c4[0], 7, 38);
    } else {
        g_iNumLineMessageBoxCustom = 0;
    }
    memset(g_lpszDialogAnswer, 0, sizeof(g_lpszDialogAnswer));
    g_iNumAnswer = 1;
    wsprintfA(g_lpszDialogAnswer[0][0], "%d) %s", 1, GlobalText[609]);
    SetErrorMessage(0);
}

// CloseInventoryRelatedWindows @ 0x004CBA60 (218 lines) — Close all trade/shop/inventory windows
// Sets all shop/warehouse/trade/chaos/event flags to 0, clears item slots.
extern "C" BYTE Inventory[];
extern "C" BYTE OffsetTradeItems[];
extern "C" BYTE OffsetMixItems[];
void __cdecl CloseInventoryRelatedWindows(void) {
    // PORT FIEL de IDA 0x004CBA60 (2026-07-25). BUG previo: limpiaba
    // DAT_07e11e98 (global EQUIVOCADO) "comentado como ShopOpened", pero
    // ShopOpened real es DAT_07eaa118 → la tienda quedaba abierta al cerrarla.
    // Anti-tamper hash-table (que envuelve el set de ShopOpened/TradeOpened en
    // IDA) omitido per policy — el efecto neto son estos clears.
    ShopOpened                 = 0;   // DAT_07eaa118  ← EL fix del cierre
    DAT_07eaa132               = 0;   // byte_7EAA132
    DAT_07e11d14               = 0;   // RepairEnable
    WarehouseOpened            = 0;   // DAT_07eaa119
    DAT_00559f5f               = 0;   // byte_559F5F
    DAT_07eaa14c               = 0;   // dword_7EAA14C
    ChaosMixOpened             = 0;   // DAT_07eaa11a
    TradeOpened                = 0;
    EventWindowOpened          = 0;   // DAT_07eaa11c
    _g_bEventChipDialogEnable  = 0;   // DAT_07e5ba80
    DAT_07e11e1c               = 0;   // g_shEventChipCount
    g_bServerDivisionEnable    = 0;
    g_bServerDivisionAccept    = 0;
    // 2026-07-27 FIX (Golden Archer bloqueaba la UI): su panel se gatea con
    // DAT_07eaa128 (!=0 && !=3). Tiene un close propio con hit-test de su X,
    // pero si ese rect no pega el panel quedaba abierto para siempre y no se
    // podía cerrar de ninguna forma. Lo sumamos al cierre genérico, que ya usan
    // Escape / I / V / C / G / P / click-al-mundo.
    DAT_07eaa128               = 0;   // Golden Archer panel

    // Limpiar los pools de items de shop/trade/mix (slots a 0xFFFF, key 0).
    for (int i = 0; i < 32; ++i) {
        BYTE* c = Inventory + i * 0x44;
        *(short*)c = (short)0xFFFF; *(DWORD*)(c + 4) = 0;
        BYTE* t = OffsetTradeItems + i * 0x44;
        *(short*)t = (short)0xFFFF; *(DWORD*)(t + 4) = 0;
        BYTE* m = OffsetMixItems + i * 0x44;
        *(short*)m = (short)0xFFFF; *(DWORD*)(m + 4) = 0;
    }

    PlayBuffer(25, 0, 0);   // sonido de cierre
    PlayBuffer(28, 0, 0);
}

// FindTextA @ 0x004977F0 (32 lines) — MBCS-aware substring search
// Searches for needle in haystack. caseSensitive=true -> prefix match only.
bool __cdecl FindTextA(char *haystack, char *needle, bool caseSensitive) {
    if (!haystack || !needle) return false;
    int tokenLen = lstrlenA(needle);
    int textLen = lstrlenA(haystack);
    if (tokenLen == 0) return false;
    int maxPos = caseSensitive ? 0 : (textLen - tokenLen);
    if (maxPos < 0) return false;

    int pos = 0;
    while (pos <= maxPos) {
        // Compare needle against haystack+pos
        bool match = true;
        for (int i = 0; i < tokenLen; i++) {
            if (haystack[pos + i] != needle[i]) { match = false; break; }
        }
        if (match) return true;
        // MBCS advance: skip 2 bytes for DBCS lead byte, else 1
        if (IsDBCSLeadByte((BYTE)haystack[pos])) pos += 2;
        else pos += 1;
    }
    return false;
}

// AngleMatrix @ 0x004F9DB0 (50 lines) — Build 3x4 rotation matrix from Euler angles
// Standard Quake/Half-Life convention:
//   angles[0] = PITCH (rotation around Y)
//   angles[1] = YAW   (rotation around Z)
//   angles[2] = ROLL  (rotation around X)
// BUG-FIX (CRÍTICO, 2026-04-20):
//   El port previo intercambiaba las etiquetas: calculaba las entradas con
//   sp=sin(angles[0]), sy=sin(angles[1]), sr=sin(angles[2]) PERO las
//   combinaba como si fueran de un orden distinto (fórmulas no-Quake). El
//   resultado: para rot=(0,0,180) (ships, chars login) producía Rx(180)
//   (patas arriba) en vez de Rz(180) (mirando al revés en pie) → todos los
//   modelos volteados. Re-verificado byte-exact contra Ghidra decompile de
//   0x004F9DB0. Mapeo correcto sP→A[0], sY→A[1], sR→A[2].
void __cdecl AngleMatrix(float *angles, float (*matrix)[4]) {
    float deg2rad = 0.017453292f; // pi/180 = DAT_00552ce8
    float sP = sinf(angles[0] * deg2rad), cP = cosf(angles[0] * deg2rad); // pitch
    float sY = sinf(angles[1] * deg2rad), cY = cosf(angles[1] * deg2rad); // yaw
    float sR = sinf(angles[2] * deg2rad), cR = cosf(angles[2] * deg2rad); // roll

    matrix[0][0] = cY * cR;
    matrix[0][1] = sP * sY * cR - cP * sR;
    matrix[0][2] = sP * sR + cP * sY * cR;
    matrix[0][3] = 0.0f;
    matrix[1][0] = cY * sR;
    matrix[1][1] = cP * cR + sP * sY * sR;
    matrix[1][2] = cP * sY * sR - sP * cR;
    matrix[1][3] = 0.0f;
    matrix[2][0] = -sY;
    matrix[2][1] = sP * cY;
    matrix[2][2] = cP * cY;
    matrix[2][3] = 0.0f;
}

// VectorIRotate @ 0x004FA110 (20 lines) — Inverse-rotate vector by matrix (transpose multiply)
// out = M^T * in1 (rotation only, ignores translation column)
void __cdecl VectorIRotate(float *in1, float (*matrix)[4], float *out) {
    out[0] = in1[0] * matrix[0][0] + in1[1] * matrix[1][0] + in1[2] * matrix[2][0];
    out[1] = in1[0] * matrix[0][1] + in1[1] * matrix[1][1] + in1[2] * matrix[2][1];
    out[2] = in1[0] * matrix[0][2] + in1[1] * matrix[1][2] + in1[2] * matrix[2][2];
}

// BMD__TransformPosition @ 0x004409A0 (67 lines) — Transform position through bone matrix
// If Translate: result = scale * (Matrix * Pos) + origin
// If !Translate: result = Matrix * Pos (direct transform)
void __fastcall BMD__TransformPosition(void *This, float (*BoneMatrix)[4], float *Pos, float *WorldPos, bool Translate) {
    // VectorTransform: WorldPos = BoneMatrix * Pos
    float temp[3];
    float *dst = Translate ? temp : WorldPos;
    dst[0] = Pos[0] * BoneMatrix[0][0] + Pos[1] * BoneMatrix[0][1] + Pos[2] * BoneMatrix[0][2] + BoneMatrix[0][3];
    dst[1] = Pos[0] * BoneMatrix[1][0] + Pos[1] * BoneMatrix[1][1] + Pos[2] * BoneMatrix[1][2] + BoneMatrix[1][3];
    dst[2] = Pos[0] * BoneMatrix[2][0] + Pos[1] * BoneMatrix[2][1] + Pos[2] * BoneMatrix[2][2] + BoneMatrix[2][3];

    if (Translate) {
        // Scale by BMD scale factor and add origin
        float scale = *(float *)((int)This + 0x68);
        float *origin = (float *)((int)This + 0x6c);
        WorldPos[0] = scale * temp[0] + origin[0];
        WorldPos[1] = scale * temp[1] + origin[1];
        WorldPos[2] = scale * temp[2] + origin[2];
    }
}

// EnableAlphaBlend @ 0x00511710 — GL additive blending setup.
// 2026-06-29 BUG-FIX (depth-mask cache desync → estructuras opacas desaparecen):
// esta copia llamaba glDepthMask(0) y glDisable(GL_CULL_FACE) DIRECTOS, sin tocar
// los caches DAT_083a42e8 (depth mask) ni DAT_083a411c (cull). El render de meshes
// usa la versión cacheada FUN_00511710 (GL_State.cpp); cuando algún caller pasaba
// por ESTA copia, el cache quedaba en "depth write ON" mientras el GL real estaba
// en OFF → el EnableDepthMask cacheado de DisableAlphaBlend se volvía no-op → el
// objeto opaco siguiente renderizaba con mask=0, no escribía depth, y geometría
// más lejana lo tapaba. El source 5.2 (ZzzOpenglUtil.cpp:468) confirma que
// EnableAlphaBlend usa el DisableDepthMask CACHEADO, nunca glDepthMask directo.
// Fix: delegar a la versión canónica cacheada (idéntico address 0x00511710).
void __cdecl EnableAlphaBlend(void) {
    FUN_00511710();
}

// EnableAlphaTest @ 0x00511680 — GL standard alpha blend + alpha test.
// 2026-06-29 BUG-FIX (mismo desync de cache que EnableAlphaBlend): esta copia
// llamaba glDepthMask(1)/glDisable(GL_CULL_FACE) DIRECTOS sin tocar los caches.
// El source 5.2 (ZzzOpenglUtil.cpp:443) confirma que EnableAlphaTest(DepthMask)
// usa el EnableDepthMask CACHEADO condicional. Fix: delegar a la versión canónica
// cacheada FUN_00511680 (idéntico address 0x00511680; param = flag DepthMask).
void __cdecl EnableAlphaTest(bool enable) {
    FUN_00511680(enable ? '\x01' : '\0');
}

// RenderTerrainAlphaBitmap @ 0x004F8BB0 (~105 lines) — Terrain decal overlay
// Renders rotated alpha texture on terrain (spell circles, shadows, blood splats).
void __cdecl RenderTerrainAlphaBitmap(int tex, float x, float y, float sx, float sy, float *light, float alpha, float size) {
    (void)tex; (void)x; (void)y; (void)sx; (void)sy; (void)light; (void)alpha; (void)size;
    // Full implementation requires:
    //   1. glColor3fv(light) or glColor4f(light[0..2], alpha)
    //   2. AngleMatrix from 'size' (rotation angle)
    //   3. BindTexture(tex)
    //   4. 2D tile loop over bounding area based on max(sx,sy)
    //   5. Per-tile: compute UV + VectorRotate for rotation
    //   6. Call FUN_004f8740 (terrain quad renderer) per tile
    // Documented at 0x004F8BB0 — terrain decal system
}

// FUN_00543839 @ 0x00543839 (4 lines) — CRT _cinit wrapper
// Forwards to internal CRT initializer with default params.
void __cdecl FUN_00543839(int param) {
    // Original: FUN_0054385b(param, 0, 0) — CRT initialization dispatch
    // In our build, no-op (CRT initializes through normal startup)
    (void)param;
}

// FUN_00543d81 @ 0x00543D81 (~45 lines) — MSVC CRT _tmpfile()
// Creates a temporary file using CRT file table. Returns stream pointer.
void *__cdecl FUN_00543d81(void) {
    // Original: acquires CRT lock, attempts tmpnam + open with O_CREAT|O_RDWR|O_BINARY,
    // retries on EEXIST, returns FILE* stream.
    // In our build, delegate to standard tmpfile
    return (void *)tmpfile();
}

// _strncpy — CRT strncpy wrapper (already functional)
void __cdecl _strncpy(char *dst, char *src, int n) {
    if (dst && src && n > 0) strncpy(dst, src, n);
}

// FUN_005436a6 @ 0x005436A6 (17 lines) — CRT fflush
// NULL → flushall; non-NULL → lock, flush, unlock.
void __cdecl FUN_005436a6(int *fp) {
    fflush((FILE *)fp);
}

// RenderCenterText @ 0x00514270 (18 lines) — Draw horizontally centered text
// Measures text width via GDI, converts to 640-based virtual coords, centers.
void __cdecl RenderCenterText(int x, int y, char *text) {
    int len = lstrlenA(text);
    SIZE sz;
    GetTextExtentPointA(m_hFontDC, text, len, &sz);
    // Convert pixel width to virtual 640-wide coords, halve for centering
    int halfWidth = (int)((unsigned int)(sz.cx * 0x280) / (unsigned int)WindowWidth) >> 1;
    RenderText(x - halfWidth, y, text, 0, 0, NULL);
}

// FUN_0051d740 — NOT a real function entry (falls mid-CreateOkMessageBox @ 0x0051D6F0)
// Kept as no-op stub; real init logic is in CreateOkMessageBox/InitGame.
void __cdecl FUN_0051d740(void) {}

// FUN_00482350 — NOT a real function entry (falls mid-FUN_004824c0 item slot lookup)
// Kept as no-op stub; address was incorrectly identified as function start.
void __cdecl FUN_00482350(void) {}

// FUN_004827a0 — NOT a real function entry (also mid-FUN_004824c0)
// Kept as no-op stub.
void __cdecl FUN_004827a0(void) {}

// FUN_00433830 — NOT a real function entry (falls mid-ReceiveTradeExit @ 0x004337F0)
// Kept as no-op stub.
void __cdecl FUN_00433830(void) {}

// RenderText @ 0x0047F650 — same address as FUN_0047f650 (Chat_DrawEntry).
// IDA's canonical name for that function IS "RenderText"; our codebase
// labels it Chat_DrawEntry but it is the universal text-draw used everywhere
// (HUD, error messages, inventory tooltips). Without this alias all the
// HUD_Pass3/6 RenderText() calls were silently no-op'd → invisible labels
// on stat panels, inventory headers, party UI, etc.
void __cdecl RenderText(int x, int y, char *text, int p1, int p2, void *p3) {
    (void)p3;  // matches "extra" stack arg, unused by the underlying call
    FUN_0047f650((undefined4)x, (undefined4)y, (LPCSTR)text,
                 (LPSIZE)(uintptr_t)p1, (char)p2, (undefined4)0);
}

// ═══════════════════════════════════════════════════════════════════════════════
// Linker stubs — external functions called by OpenNpc/RenderEquipment3D/RenderItems3D
// These are placeholders until the actual implementations are decompiled.
// ═══════════════════════════════════════════════════════════════════════════════

// 2026-05-05: AccessModel era stub vacío → ningún BMD de NPC se cargaba.
// Solo el guardia (type=249) renderizaba porque usa player model 390 ya
// cargado. Los demás NPCs (Storage, Smith, Wizard, etc.) llamaban a
// AccessModel("Data\\Npc\\", "Storage", 1) etc pero el modelo nunca se
// cargaba → invisible.
//
// FUN_005060b0 es la impl real del BMD loader (Monster_LoadModel) — ya
// usado por OpenWorld para cargar Object1, Object11, etc. Misma signatura
// (id, path, name, idx). Delegamos directamente.
//
// 2026-05-05 (followup): además llamar FUN_00505c80 (OpenTexture) post-BMD
// load. Sin esto los NPCs cargaban geometría pero las texturas no se
// resolvían en los slots (IndexTexture[]) → render en blanco. El cliente
// original sí hace este paso después del BMD load para NPCs.
void __cdecl AccessModel(int id, char* path, char* name, int param) {
    FUN_005060b0(id, path, name, param);
    // Path para OpenTexture: typically "Npc\" sin "Data\" prefijo (los
    // path-strippers en FUN_00529bd0/740 ya lo manejan si viene completo).
    if (path) {
        FUN_00505c80(id, path, 0x2600, '\x01');
    }
    // 2026-05-05: setup de animation speeds (idéntico al patrón que
    // FUN_005098c0 hace para monsters). Sin esto, los NPCs cargan
    // geometry/textures pero entity[+0x105] action speed = 0 →
    // CharacterAnimation no avanza el frame → NPCs estáticos.
    //
    // CharacterAnimation lee de model+48 (=bones table per FUN_004423e0
    // alloc) con stride 16 bytes. Esa tabla tiene `numBones` entries de 0x10
    // bytes c/u. Para evitar buffer overflow (crashes vimos con NPCs de
    // pocos bones), solo escribir speeds hasta el límite de bones disponibles.
    int slotBase = DAT_05828d58 + id * 0xbc;
    int actionsTable = *(int*)(slotBase + 48);
    short numBones = *(short*)(slotBase + 38);   // realmente numBones, but anim speed lookup uses this
    if (actionsTable && numBones > 0) {
        static const unsigned int kSpeeds[7] = {
            0x3e800000,  // 0.25f - action 0 (idle)
            0x3e4ccccd,  // 0.2f  - action 1
            0x3eae147b,  // 0.34f - action 2
            0x3ea8f5c3,  // 0.33f - action 3
            0x3ea8f5c3,  // 0.33f - action 4
            0x3f000000,  // 0.5f  - action 5
            0x3f0ccccd   // 0.55f - action 6
        };
        int maxSpeeds = (numBones < 7) ? numBones : 7;
        for (int i = 0; i < maxSpeeds; ++i) {
            *(unsigned int*)(actionsTable + i * 0x10 + 0x04) = kSpeeds[i];
        }
    }
}

// OpenTexture @ 0x00505C80 — forward al símbolo FUN_00505c80 (implementado arriba).
// functions.h lo declara con esta firma (void*, bool); en x86 cdecl los tipos
// son binariamente compatibles con (const char*, char).
void __cdecl OpenTexture(int id, void* path, int flags, bool param) {
    FUN_00505c80(id, (const char*)path, flags, (char)param);
}

// LoadWaveFile @ 0x00404A10 — real implementation in src/Sound/Sound.cpp.

void __cdecl OpenModel(int id, char* path, ...) {
    // 0x00505E90 — Load model with variable texture/normal args
    (void)id; (void)path;
}

// FUN_005112f0 (CreateScreenVector) ya implementada en stubs.cpp:1630.
// FUN_004e13a0 (RenderObjectScreen) ya implementada en stubs.cpp:12999.
extern void __cdecl FUN_005112f0(int sx, int sy, float* out);
extern void __cdecl FUN_004e13a0(int param_1, unsigned int param_2,
                                  unsigned char param_3, unsigned char param_4,
                                  float* param_5, int param_6, char param_7);

// RenderItem3D @ 0x004E1BE0 — port FIEL del binario original.
//
// IDA decompile (raw 0x4E1BE0):
//   1. Calcula Success (hit-test mouse vs item rect).
//   2. Switch by Type → ajusta sx/sy con factor de offset (0.5..1.1 de width/height)
//      para centrar correctamente el modelo 3D en el slot del inventario.
//   3. Llama CreateScreenVector(sx, sy, Position) → world-space pos.
//   4. Llama RenderObjectScreen(modelId, Level, Option1, Position, Success, PickUp).
//      modelId = Type + 400 (default), o IDs específicos para items 459/457/469/435 según Level.
//
// Esto reemplaza el placeholder que pintaba quads coloreados por grupo.
void __cdecl RenderItem3D(float sx, float sy, float Width, float Height,
                          int Type, int Level, int Option1, int ExtOption, bool PickUp)
{
    bool Success = false;
    float Position[3];

    // Hit-test: mouse over item rect (only when no item is picked-up, OR this IS the picked-up item).
    if (!DAT_07e91388 || PickUp) {
        float fMouseX = (float)DAT_083a427c;   // MouseX
        float fMouseY = (float)DAT_083a4278;   // MouseY
        if (fMouseX >= sx && fMouseX < sx + Width &&
            fMouseY >= sy && fMouseY < sy + Height) {
            Success = true;
        }
    }

    // Per-type screen-position offset (centro del modelo dentro del slot).
    // Branch tree extraído fielmente del IDA decompile.
    float ofsXmul = 0.5f, ofsYmul = 0.5f;  // defaults
    bool resolved = false;

    if (Type >= 0) {
        if (Type < 32) {
            ofsXmul = 0.80f; ofsYmul = 0.85f; resolved = true;
        } else if (Type < 96) {
            ofsXmul = 0.80f; ofsYmul = 0.70f; resolved = true;
        } else if (Type < 128) {
            ofsXmul = 0.60f; ofsYmul = 0.65f; resolved = true;
        }
    }

    if (!resolved) {
        if (Type == 145) {
            ofsXmul = 0.50f; ofsYmul = 0.50f; resolved = true;
        } else if (Type >= 136 && Type < 160) {
            ofsXmul = 0.70f; ofsYmul = 0.70f; resolved = true;
        } else if (Type >= 160 && Type < 192) {
            ofsXmul = 0.60f; ofsYmul = 0.55f; resolved = true;
        } else if (Type >= 192 && Type < 224) {
            ofsXmul = 0.50f;
            if (Type == 207) ofsYmul = 0.70f;
            else if (Type == 208) ofsYmul = 0.90f;
            else ofsYmul = 0.60f;
            resolved = true;
        } else if (Type >= 224 && Type < 256) {
            ofsXmul = 0.50f; ofsYmul = 0.80f; resolved = true;
        } else if (Type >= 256 && Type < 288) {
            ofsXmul = 0.50f;
            if (Type == 258 || Type == 260 || Type == 262) ofsYmul = 1.05f;
            else if (Type == 259 || Type == 264) ofsYmul = 1.10f;
            else ofsYmul = 0.80f;
            resolved = true;
        } else if (Type >= 288 && Type < 384) {
            ofsXmul = 0.50f; ofsYmul = 0.90f; resolved = true;
        }
    }

    if (!resolved) {
        switch (Type) {
            case 430: {
                ofsXmul = 0.60f; ofsYmul = 1.00f;
                resolved = true;
                break;
            }
            case 431:
                ofsXmul = 0.60f; ofsYmul = 1.00f; resolved = true; break;
            case 432:
            case 433:
                ofsXmul = 0.50f; ofsYmul = 0.90f; resolved = true; break;
            case 434:
                ofsXmul = 0.50f; ofsYmul = 0.75f; resolved = true; break;
        }
    }

    if (!resolved && Type == 435) {
        int lvl3 = Level >> 3;
        if (lvl3 == 0)      { ofsXmul = 0.50f; ofsYmul = 0.50f; }
        else if (lvl3 == 1) { ofsXmul = 0.70f; ofsYmul = 0.80f; }
        else if (lvl3 == 2) { ofsXmul = 0.70f; ofsYmul = 0.70f; }
        resolved = true;
    }

    if (!resolved && Type >= 416 && Type < 448) {
        ofsXmul = 0.50f; ofsYmul = 0.70f; resolved = true;
    }

    // 2026-08-11 — REMOVIDO: bloque inventado por el port (el comentario original
    // decía que estos items "expect to be centered ... not biased downward",
    // o sea una heurística a ojo, no un decompile). Forzaba ofsYmul = 0.50 para
    // 416-419/428/429 y, al no llevar guard `!resolved`, PISABA el valor correcto
    // de IDA para el rango [416,448) que asigna la rama de arriba (0.50/0.70).
    // Efecto: la Uniria (tipo 418) se anclaba en el centro de la casilla en vez
    // de al 70% → se veía más arriba que en el original. IDA `RenderItem3D`
    // (0x4E1BE0): `if (Type >= 416 && Type < 448) { _sx += W*0.5; _sy += H*0.7; }`
    // sin ninguna excepción para esos tipos.

    if (!resolved) {
        switch (Type) {
            case 457: {
                int lvl3 = (Level >> 3) & 0x0F;
                ofsXmul = 0.50f;
                ofsYmul = (lvl3 == 1) ? 0.80f : 0.95f;
                resolved = true;
                break;
            }
            // IDA: `case 460: case 459:` comparten cuerpo —
            //   if ((Level & 0xF8) == 24) goto LABEL_62 (0.5/0.5)
            //   else                      LABEL_90      (0.5/0.95)
            // 2026-08-11 FIX: el 460 estaba agrupado con 465-467 (0.5/0.5 fijo)
            // y el 459 tenía ramas inventadas (lvl3 13/14/15) que no están en el
            // 0.97k.
            case 459:
            case 460: {
                ofsXmul = 0.50f;
                ofsYmul = ((Level & 0xF8) == 24) ? 0.50f : 0.95f;
                resolved = true;
                break;
            }
            case 465:
            case 466:
            case 467:
                // IDA: goto LABEL_62
                ofsXmul = 0.50f; ofsYmul = 0.50f; resolved = true; break;
            case 469: {
                // IDA: if (!(Level>>3)) LABEL_62 (0.5/0.5)
                //      else if ((Level>>3) == 1) { sx += W*0.4; LABEL_73: sy += H*0.8 }
                //      else                      LABEL_93 (SIN offset)
                int lvl3 = Level >> 3;
                if (lvl3 == 0)      { ofsXmul = 0.50f; ofsYmul = 0.50f; }
                else if (lvl3 == 1) { ofsXmul = 0.40f; ofsYmul = 0.80f; }
                else                { ofsXmul = 0.00f; ofsYmul = 0.00f; }
                resolved = true;
                break;
            }
            // IDA: `if (Type >= 470) { if (Type < 473) LABEL_90; if (Type < 475) LABEL_79; }`
            case 470:
            case 471:
            case 472:                      // faltaba 472 (el rango es [470,473))
                ofsXmul = 0.50f; ofsYmul = 0.95f; resolved = true; break;
            case 473:
            case 474:
                ofsXmul = 0.50f; ofsYmul = 0.90f; resolved = true; break;
            // 2026-08-11 — REMOVIDOS los casos 475/476/477/478/479: no existen en
            // el 0.97k. En IDA caen al final de la cadena y, por estar dentro de
            // [448,480), terminan en `goto LABEL_90` = 0.50/0.95 (el mismo
            // fallback de más abajo). Los valores que había (0.90 / 0.5-0.5 /
            // 0.55-0.80) eran invenciones del port.
        }
    }

    if (!resolved && Type >= 416 && Type < 448) {
        ofsXmul = 0.50f;
        ofsYmul = 0.70f;
        resolved = true;
    }

    // IDA (cola): `if (Type < 448 || Type >= 480) { 0.5 / 0.60 } else goto LABEL_90 (0.5/0.95)`
    // 2026-08-11 FIX: el rango era [448,512), así que los tipos 480-511 tomaban
    // 0.95 cuando en IDA les corresponde 0.60.
    if (!resolved && Type >= 448 && Type < 480) {
        ofsXmul = 0.50f;
        ofsYmul = 0.95f;
        resolved = true;
    }

    if (!resolved) {
        switch (Type) {
            case 387: ofsXmul = 0.50f; ofsYmul = 0.45f; resolved = true; break;
            case 388: ofsXmul = 0.50f; ofsYmul = 0.40f; resolved = true; break;
            case 389: ofsXmul = 0.50f; ofsYmul = 0.75f; resolved = true; break;
            case 390: ofsXmul = 0.50f; ofsYmul = 0.55f; resolved = true; break;
        }
    }

    if (!resolved && (Type < 448 || Type >= 480)) {
        ofsXmul = 0.50f; ofsYmul = 0.60f;
    }

    float _sx = sx + Width  * ofsXmul;
    // ── ANCHORDIFF (temporal): re-implementación FIEL de la cadena de anclaje de
    // IDA `RenderItem3D` (0x4E1BE0) para Type >= 384, con los `goto LABEL_xx`
    // resueltos. Compara contra lo que calculó el código de arriba y loguea SÓLO
    // las discrepancias — así no hace falta identificar cada item por nombre.
    if (Type >= 384) {
        const float L90x = 0.50f, L90y = 0.95f;   // LABEL_90
        const float L79x = 0.50f, L79y = 0.90f;   // LABEL_79
        const float L85x = 0.50f, L85y = 0.75f;   // LABEL_85
        const float L65x = 0.50f, L65y = 0.80f;   // LABEL_65
        const float L62x = 0.50f, L62y = 0.50f;   // LABEL_62
        float rx = -1.0f, ry = -1.0f;             // -1 = "sin offset" (LABEL_93)
        int lvl3 = Level >> 3;

        if (Type == 430 || Type == 431)      { rx = 0.60f; ry = 1.00f; }
        else if (Type == 432 || Type == 433) { rx = L79x;  ry = L79y;  }
        else if (Type == 434)                { rx = L85x;  ry = L85y;  }
        else if (Type == 435) {
            if (lvl3 == 0)      { rx = L62x; ry = L62y; }
            else if (lvl3 == 1) { rx = 0.70f; ry = 0.80f; }
            else if (lvl3 == 2) { rx = 0.70f; ry = 0.70f; }   // LABEL_54
            else                { rx = 0.0f;  ry = 0.0f;  }
        }
        else if (Type >= 416 && Type < 448)  { rx = 0.50f; ry = 0.70f; }
        else if (Type == 459 || Type == 460) {
            if ((Level & 0xF8) == 24) { rx = L62x; ry = L62y; }
            else                      { rx = L90x; ry = L90y; }
        }
        else if (Type == 457) {
            if ((Level & 0xFFFFFFF8) != 8) { rx = L90x; ry = L90y; }
            else                           { rx = L65x; ry = L65y; }
        }
        else if (Type == 465 || Type == 466 || Type == 467) { rx = L62x; ry = L62y; }
        else if (Type == 469) {
            if (lvl3 == 0)      { rx = L62x; ry = L62y; }
            else if (lvl3 == 1) { rx = 0.40f; ry = 0.80f; }
            else                { rx = 0.0f;  ry = 0.0f;  }
        }
        else if (Type >= 470 && Type < 473) { rx = L90x; ry = L90y; }
        else if (Type >= 473 && Type < 475) { rx = L79x; ry = L79y; }
        else if (Type == 387) { rx = 0.50f; ry = 0.45f; }
        else if (Type == 388) { rx = 0.50f; ry = 0.40f; }
        else if (Type == 389) { rx = L85x;  ry = L85y;  }
        else if (Type == 390) { rx = 0.50f; ry = 0.55f; }
        else if (Type < 448 || Type >= 480) { rx = 0.50f; ry = 0.60f; }
        else { rx = L90x; ry = L90y; }

        if (rx >= 0.0f) {
            float dx = ofsXmul - rx, dy = ofsYmul - ry;
            if (dx < 0) dx = -dx;
            if (dy < 0) dy = -dy;
            if (dx > 0.001f || dy > 0.001f) {
                static int  s_seen[64]; static int s_n = 0;
                bool dup = false;
                for (int q = 0; q < s_n; ++q) if (s_seen[q] == Type) { dup = true; break; }
                if (!dup && s_n < 64) {
                    s_seen[s_n++] = Type;
                    char db[190];
                    _snprintf_s(db, sizeof(db), _TRUNCATE,
                        "ANCHORDIFF type=%d lvl=%d lvl3=%d  nuestro=(%.2f,%.2f)  IDA=(%.2f,%.2f)",
                        Type, Level, lvl3, ofsXmul, ofsYmul, rx, ry);
                    DbgLogPublic(db);
                }
            }
        }
    }

    float _sy = sy + Height * ofsYmul;

    // Convert screen-space → world-space ray endpoint.
    FUN_005112f0((int)_sx, (int)_sy, Position);

    // Per-type modelId override (jewels/wings/special).
    int modelId = Type + 400;
    int levelArg = Level;

    switch (Type) {
        case 459: {
            int lvl3 = (Level >> 3) & 0x0F; // ITEM_POTION+11
            if (lvl3 == 1) modelId = MODEL_EVENT + 4;
            else if (lvl3 == 2) modelId = MODEL_EVENT + 5;
            else if (lvl3 == 3) modelId = MODEL_EVENT + 6;
            else if (lvl3 == 5) modelId = MODEL_EVENT + 8;
            else if (lvl3 == 6) modelId = MODEL_EVENT + 9;
            else if (lvl3 >= 8 && lvl3 <= 12) modelId = MODEL_EVENT + 10;
            else if (lvl3 == 13) modelId = MODEL_EVENT + 6;
            else if (lvl3 == 14 || lvl3 == 15) modelId = MODEL_EVENT + 5;
            break;
        }
        case 457: { // ITEM_POTION+9
            if (((Level >> 3) & 0x0F) == 1) modelId = MODEL_EVENT + 7;
            break;
        }
        case 469: { // ITEM_POTION+21
            if ((Level >> 3) == 1) modelId = 958;
            break;
        }
        case 435: {
            int lvl3 = Level >> 3;
            if (lvl3 == 0)      { modelId = MODEL_STAFF + 10; levelArg = -1; }
            else if (lvl3 == 1) { modelId = MODEL_SWORD + 19; levelArg = -1; }
            else if (lvl3 == 2) { modelId = MODEL_BOW + 18;   levelArg = -1; }
            break;
        }
        case 428:
            modelId = MODEL_HELPER + 12;
            break;
        case 429:
            modelId = MODEL_HELPER + 13;
            break;
        case 461:
            modelId = MODEL_POTION + 13;
            break;
        case 462:
            modelId = MODEL_POTION + 14;
            break;
        case 493:
            modelId = MODEL_POTION + 45;
            break;
        case 494:
        case 495:
        case 496:
            modelId = MODEL_POTION + 46;
            break;
        case 497:
            modelId = MODEL_POTION + 49;
            break;
        case 498:
            modelId = MODEL_POTION + 50;
            break;
        case 548:
            modelId = MODEL_POTION + 100;
            break;
        }

    // Guard contra modelo no cargado o pointer corrupto. RenderObjectScreen
    // (FUN_004e13a0) deferenciaría el modelEntry → libjpeg crash si meshBase
    // o numMesh están en garbage. Retornar silencioso si modelo no listo.
    {
        if (modelId < 0 || modelId >= 1200) return;
        char* modelEntry = (char*)DAT_05828d58 + modelId * 0xbc;
        short numMesh = *(short*)(modelEntry + 0x24);
        int meshBase = *(int*)(modelEntry + 0x28);
        if (numMesh <= 0 || numMesh > 1000) return;
        if (meshBase == 0 || (uintptr_t)meshBase < 0x100000) return;
    }

    // IDA 0x004E1BE0 calls RenderObjectScreen(Type+400, Level, Option1, Position, Success, PickUp).
    // The original path does not forward ExtOption here.
    FUN_004e13a0(modelId, (unsigned int)levelArg, (unsigned char)Option1,
                 0, Position, Success ? 1 : 0, PickUp ? 1 : 0);
}

// Batch 21 — helper function stubs (called by MoveObjects, CollisionDetectLineToMesh, CheckMixRecipe)
// FUN_004fa5f0 (IDA-activated, was Ghidra stub)
void __cdecl FUN_004fa5f0(int a1)
{
  int v1; // edi
  short v2; // ax
  float *v3; // edi
  float v4; // ecx
  float v5; // edx
  bool v6; // zf
  int v7; // ebx
  float Position[3]; // [esp+8h] [ebp-Ch] BYREF

  if ( DAT_0055a7b4 < 0 )
  {
    return;
  }
  if ( DAT_0055a7b0 < 0 )
  {
    return;
  }
  v1 = DAT_0055a7b8;
  if ( DAT_0055a7b8 < 0 || World != DAT_0055a7b4 )
  {
    return;
  }
  v2 = *(WORD *)(a1 + 2);
  if ( v2 != DAT_0055a7b0 )
  {
    if ( v2 == 9 )
    {
LABEL_9:
      if ( !DAT_0055a7b8 )
      {
        *(DWORD *)(a1 + 88) = -1;
        *(WORD *)(a1 + 134) = 4;
      }
      return;
    }
    if ( v2 != 10 )
    {
      return;
    }
  }
  if ( v2 == 9 || v2 == 10 )
  {
    goto LABEL_9;
  }
  if ( v2 == DAT_0055a7b0 )
  {
    if ( DAT_0055a7b8 == 20 )
    {
      *(DWORD *)(a1 + 28) = 1108082688;
      *(DWORD *)(a1 + 88) = -1;
      PlayBuffer(108, 0, 0);
      v1 = DAT_0055a7b8;
    }
    if ( v1 >= 0 )
    {
      v3 = (float *)(a1 + 28);
      *(float *)(a1 + 28) = DAT_0055a7bc + *(float *)(a1 + 28);
      DAT_0055a7bc = DAT_0055a7bc + 1.5;
      if ( *(float *)(a1 + 28) >= 90.0 )
      {
        *v3 = *v3 - (double)DAT_0055a7b8;
        DAT_0055a7bc = 2.0;
        v4 = *(float *)(a1 + 20);
        v5 = *(float *)(a1 + 24);
        Position[0] = *(float *)(a1 + 16);
        v6 = *(DWORD *)v3 == 1117782016;
        Position[1] = v4;
        Position[2] = v5;
        if ( v6 )
        {
          v7 = 10;
          do
          {
            Position[0] = (double)(rand() % 300) - 150.0 + *(float *)(a1 + 16);
            Position[1] = *(float *)(a1 + 20) - ((double)(rand() % 20) + 600.0);
            Particle_Spawn(1221, Position, (float *)(a1 + 28), (float *)(a1 + 232), 0, 1.0, 0);
            --v7;
          }
          while ( v7 );
        }
      }
      if ( !DAT_0055a7b8 )
      {
        *(DWORD *)(a1 + 88) = -2;
        *v3 = 90.0;
        FUN_004fa5a0();
        AddTerrainAttributeRange(13, 70, 3, 6, 8u, 0);
      }
      --DAT_0055a7b8;
    }
  }
}


#ifndef IDA_PORT_004FDC00   // desactivado: el port FULL vive en stubs_IDA_ports.cpp
// 0x004FDC00 — MoveObjects: tick per-frame de cada objeto visible del mundo.
//
// 2026-08-11 — UNIFICACIÓN. Existían DOS ports de esta función:
//   · éste, mínimo, que sólo hacía Alpha + el banner MUGAME del login, y
//   · `MoveObject_PerWorld_stub` (stubs_game.cpp), con el toggle por HeroTile,
//     PlayAnimation y el switch por World COMPLETO.
// El que se llamaba desde el loop de MoveObjects era éste, así que el switch
// por World nunca corría: **ningún objeto del mundo generaba sus efectos**.
// Entre otras cosas, los tipos 130/131/132 de Lorencia (Light01/02/03) quedaban
// visibles como cajas de 8 vértices con la textura dummy `ston03` (2x2 negra)
// en vez de ocultarse (`HiddenMesh = -2`) y emitir el humo de las chimeneas y
// de la forja del herrero. Mismo patrón que `OpenSMDFile` / `RenderText` /
// `SetPlayerStop`: un símbolo con dos implementaciones donde gana la incompleta.
//
// Ahora hay una sola implementación, en el orden del binario:
//   World 9 → World 0/2 toggles → Alpha → early-return → PlayAnimation →
//   bloque de login (160/161/162) → switch por World.
void __cdecl FUN_004fdc00(float pObj) {
    if (LODWORD(pObj) == 0) return;
    MoveObject_PerWorld_stub(pObj);
}
#endif  // IDA_PORT_004FDC00 (minimal disabled)

void __cdecl FaceNormalize(float v[3], float out[3], float v2[3], float normal[3]) {
    // 0x00440A60 approx — Compute face normal from 3 vertices
    // normal = normalize(cross(v1-v0, v2-v0))
    (void)v; (void)out; (void)v2; (void)normal;
}

bool __cdecl CollisionDetectLineToFace(float pos[3], float target[3], int normalIdx,
          float localC[3], float* posZ, float* v3, float* v4, float normal[3], char flag) {
    // 0x00440C90 approx — Test line segment against a triangle face
    (void)pos; (void)target; (void)normalIdx; (void)localC;
    (void)posZ; (void)v3; (void)v4; (void)normal; (void)flag;
    return false;
}

// addr: 0x00482BE0  (sub_482BE0 / category -> inventory slot scan)
// ported from IDA raw/00482BE0_sub_482BE0.c
// 91-line item-type category lookup. Given a category (0,1,2 = right/left/crossbow
// equipped, 3 = force Type=454..452, 4 = force Type=451..448), derives the Type
// range [v2..v1] using DAT_00559c60/64/68 (current equipped weapon per hand) and
// scans the 8x8 inventory grid (DAT_07EA9328..DAT_07EA9504, 17 ints/row,
// 136 ints/col) for the first matching slot, returning a packed slot index.
// Returns -1 if Teleport flag is active and the range collapses to 458, or if no
// slot matches. NOTE: `Teleport` global is not declared in this translation unit
// — treated as 0 (never active); this keeps the fast path identical to IDA.
extern "C" BYTE OffsetInventoryItems[];
int __cdecl FUN_00482be0(int a1) {
    int v1;
    int v2;

    if (a1 < 3) {
        v1 = a1;
        v2 = a1;
    } else if (a1 == 3) {
        v1 = 454;
        v2 = 452;
    } else if (a1 == 4) {
        v1 = 451;
        v2 = 448;
    } else {
        v1 = a1;
        v2 = a1;
    }

    if (a1 == 0) {
        v1 = DAT_00559c60;
        if (DAT_00559c60 == 456 || DAT_00559c60 == 457 || DAT_00559c60 == 468) {
            v2 = DAT_00559c60;
        } else if (DAT_00559c60 >= 452 && DAT_00559c60 <= 454) {
            v1 = 454;
            v2 = 452;
        } else {
            v1 = 451;
            v2 = 448;
        }
    } else if (a1 == 1) {
        v1 = DAT_00559c64;
        if (DAT_00559c64 == 456 || DAT_00559c64 == 457 || DAT_00559c64 == 468) {
            v2 = DAT_00559c64;
        } else if (DAT_00559c64 < 448 || DAT_00559c64 > 451) {
            v1 = 454;
            v2 = 452;
        } else {
            v1 = 451;
            v2 = 448;
        }
    } else if (a1 == 2) {
        v1 = DAT_00559c68;
        if (DAT_00559c68 == 456 || DAT_00559c68 == 457 || DAT_00559c68 == 468) {
            v2 = DAT_00559c68;
        } else if (DAT_00559c68 >= 448 && DAT_00559c68 <= 451) {
            v1 = 451;
            v2 = 448;
        } else if (DAT_00559c68 >= 452 && DAT_00559c68 <= 454) {
            v1 = 454;
            v2 = 452;
        } else {
            v1 = 456;
            v2 = 456;
        }
    }

    if (v1 < v2) {
        return -1;
    }

    ITEM* inv = (ITEM*)OffsetInventoryItems;
    for (int wanted = v1; wanted >= v2; --wanted) {
        for (int slot = 0; slot < 64; ++slot) {
            ITEM* it = &inv[slot];
            if (it->Type == wanted && it->Durability > 0) {
                return slot;
            }
        }
    }
    return -1;
}

// ItemConvert @ 0x0047B910 — inventory/equipment item stat + option expansion.
// Ported directly from IDA structure/logic instead of the old minimal stub.
void __cdecl FUN_0047b910(int pItem, int Attribute1, int Attribute2) {
    ITEM* ip = (ITEM*)pItem;
    if (!ip) return;

    short wType = ip->Type;
    if (wType < 0) {
        ip->Part = (BYTE)-1;
        ip->SpecialNum = 0;
        memset(ip->Special, 0, sizeof(ip->Special));
        memset(ip->SpecialValue, 0, sizeof(ip->SpecialValue));
        return;
    }

    ITEM_ATTRIBUTE* table = (ITEM_ATTRIBUTE*)(uintptr_t)DAT_07d78068;
    if (!table) return;

    ITEM_ATTRIBUTE* p = &table[wType];
    int itemLevel = ((BYTE)Attribute1 >> 3) & 0xF;
    int itemExcel = ((BYTE)Attribute2) & 0x3F;
    int itemExt = (int)ip->byColorState;
    int excelAddValue = 0;
    bool bExtOption = ((itemExt % 4) == 1 || (itemExt % 4) == 2);

    // 97k: helper/potion inventory items must not inherit excellent state
    // from the ext-byte path. Letting them do so pushes RequireLevel +20 and
    // contaminates tooltip/render with equipment logic.
    if ((wType >= 416 && wType < 424) || wType >= 448) {
        bExtOption = false;
    }

    ip->Level = (BYTE)Attribute1;
    memset(ip->Special, 0, sizeof(ip->Special));
    memset(ip->SpecialValue, 0, sizeof(ip->SpecialValue));
    ip->SpecialNum = 0;
    ip->Color = 0;

    // AUDITORIA 2026-07-20 — itemExcel ahora es FIEL a IDA ItemConvert (0x47B910).
    // El original hace exactamente esto y nada mas:
    //     iItemExcel = Attribute2 & 63;
    //     if (Type 387..390 || 19 || 146 || 170) iItemExcel = 0;
    // Aca habia DOS lineas de mas que no existen ni en IDA ni en el DLL de
    // inyeccion (verificado en Source/Client/Main/Item.cpp, que reemplaza
    // ItemConvert entero y tampoco las tiene):
    //
    //   1) `if (bExtOption) itemExcel = 1;`  ← la peor: forzaba el flag excellent,
    //      y de ahi `levelAddValue += 25`, inflando RequireStrength/Dexterity/
    //      Energy y los bonus excellent de damage/defense de CUALQUIER item con
    //      el ext-byte puesto.
    //   2) `if (Type 416..423 || >= 448) itemExcel = 0;`  ← ceroeaba de mas.
    //
    // `bExtOption` se conserva: NO alimenta la matematica de stats, pero si el
    // color del item (ip->Color / byColorState), que lo consumen
    // Render_PlayerEquipment y HUD_Pass4.  Ese bloque es otro injerto de origen
    // distinto y se audita aparte.
    if ((wType >= 387 && wType <= 390) || wType == 19 || wType == 146 || wType == 170) {
        itemExcel = 0;
    }

    ip->TwoHand = p->TwoHand;
    ip->WeaponSpeed = p->AttackSpeed;
    ip->DamageMin = p->DamageMin;
    ip->DamageMax = p->DamageMax;
    ip->SuccessfulBlocking = p->DefenseRate;
    ip->Defense = p->Defense;
    ip->MagicDefense = p->MagicDefense;
    ip->WalkSpeed = p->WalkSpeed;

    switch (wType) {
    case 70:  excelAddValue = 15; break;
    case 134: excelAddValue = 30; break;
    case 167: excelAddValue = 25; break;
    default: break;
    }

    auto min9 = [](int v) -> int { return (v <= 9) ? v : 9; };
    auto post9_bonus = [](int lvl) -> int {
        int add = 0;
        for (int i = 0; i < lvl - 9; ++i) add += (i == 0) ? 4 : 5;
        return add;
    };
    auto push_special = [&](BYTE code, BYTE value = 0) {
        if (ip->SpecialNum < MAX_SPECIAL_OPTION) {
            ip->SpecialValue[ip->SpecialNum] = value;
            ip->Special[ip->SpecialNum] = code;
            ++ip->SpecialNum;
        }
    };

    if (p->DamageMin) {
        if (itemExcel > 0 && p->Level) {
            ip->DamageMin = (WORD)(ip->DamageMin + (excelAddValue ? excelAddValue : (25 * p->DamageMin / p->Level + 5)));
        }
        ip->DamageMin = (WORD)(ip->DamageMin + 3 * min9(itemLevel) + post9_bonus(itemLevel));
    }

    if (p->DamageMax) {
        if (itemExcel > 0 && p->Level) {
            ip->DamageMax = (WORD)(ip->DamageMax + (excelAddValue ? excelAddValue : (25 * p->DamageMin / p->Level + 5)));
        }
        ip->DamageMax = (WORD)(ip->DamageMax + 3 * min9(itemLevel) + post9_bonus(itemLevel));
    }

    if (p->DefenseRate) {
        if (itemExcel > 0 && p->Level) {
            ip->SuccessfulBlocking = (BYTE)(ip->SuccessfulBlocking + (25 * p->DefenseRate / p->Level + 5));
        }
        ip->SuccessfulBlocking = (BYTE)(ip->SuccessfulBlocking + 3 * min9(itemLevel) + post9_bonus(itemLevel));
    }

    if (p->Defense) {
        if (wType >= 192 && wType < 224) {
            ip->Defense = (WORD)(ip->Defense + itemLevel);
        } else {
            if (itemExcel > 0 && p->Level) {
                ip->Defense = (WORD)(ip->Defense + (12 * p->Defense / p->Level + p->Level / 5 + 4));
            }
            ip->Defense = (WORD)(ip->Defense + ((wType >= 387 && wType <= 390) ? 2 : 3) * min9(itemLevel) + post9_bonus(itemLevel));
        }
    }

    if (p->MagicDefense) {
        ip->MagicDefense = (WORD)(ip->MagicDefense + 3 * min9(itemLevel) + post9_bonus(itemLevel));
    }

    int levelAddValue = p->Level;
    if (itemExcel) levelAddValue += 25;

    if (p->RequireStrength) {
        unsigned int v = (unsigned int)(((unsigned long long)4123168605ULL * p->RequireStrength * (levelAddValue + 3 * itemLevel)) >> 32) >> 5;
        ip->RequireStrength = (WORD)(v + (v >> 31) + 20);
    } else {
        ip->RequireStrength = 0;
    }

    if (p->RequireAgility) {
        unsigned int v = (unsigned int)(((unsigned long long)4123168605ULL * p->RequireAgility * (levelAddValue + 3 * itemLevel)) >> 32) >> 5;
        ip->RequireDexterity = (WORD)(v + (v >> 31) + 20);
    } else {
        ip->RequireDexterity = 0;
    }

    if (wType == 395) {
        switch (itemLevel) {
        case 0: ip->RequireEnergy = 30; break;
        case 1: ip->RequireEnergy = 60; break;
        case 2: ip->RequireEnergy = 90; break;
        case 3: ip->RequireEnergy = 130; break;
        case 4: ip->RequireEnergy = 170; break;
        case 5: ip->RequireEnergy = 210; break;
        case 6: ip->RequireEnergy = 300; break;
        default: ip->RequireEnergy = (BYTE)Attribute2; break;
        }
    } else if (p->RequireEnergy) {
        ip->RequireEnergy = (WORD)(4 * p->RequireEnergy * (levelAddValue + 3 * itemLevel) / 100 + 20);
    } else {
        ip->RequireEnergy = 0;
    }

    int requireLevelAdd = 4;
    if (wType >= 387 && wType <= 390) requireLevelAdd = 5;
    if (p->RequireLevel) {
        ip->RequireLevel = (WORD)(p->RequireLevel + itemLevel * requireLevelAdd);
    } else {
        ip->RequireLevel = 0;
    }

    if (wType == 426) {
        ip->RequireLevel = (WORD)(50 + ((((BYTE)Attribute1 >> 3) & 0xF) > 2 ? 0 : -30));
    }
    if (itemExcel > 0 && ip->RequireLevel) {
        ip->RequireLevel = (WORD)(ip->RequireLevel + 20);
    }

    if (wType >= 387 && wType <= 390) {
        if (Attribute2 & 1)  push_special(80, (BYTE)(5 * (itemLevel + 10)));
        if (Attribute2 & 2)  push_special(81, (BYTE)(5 * (itemLevel + 10)));
        if (Attribute2 & 4)  push_special(82, 3);
        if (Attribute2 & 8)  push_special(83, 50);
        if (Attribute2 & 0x10) push_special(77, 5);
    }

    if (Attribute1 & 0x80) {
        if (p->RequireClass[1]) {
            if (ip->Type >= 196 && ip->Type < 224) push_special(18);
            if (wType == 4 || wType == 7 || wType == 8) push_special(21);
            if (wType == 3 || wType == 6 || wType == 9 || wType == 11 || (wType >= 97 && wType <= 100)) push_special(20);
            if (wType == 5 || wType == 10 || wType == 13 || wType == 14 || wType == 16 || wType == 96 || (wType >= 103 && wType <= 105)) push_special(22);
            if (wType == 12 || (wType >= 34 && wType < 64) || wType == 65 || wType == 67 || wType == 68) push_special(19);
            if (wType == 15 || wType == 69 || wType == 70 || wType == 17) push_special(23);
            if (wType == 19 || wType == 106) push_special(22);
        }
        if (p->RequireClass[2]) {
            if (wType >= 128 && wType < 160 && wType != 135 && wType != 143) push_special(24, 6);
        }
        if (p->RequireClass[3]) {
            if (wType == 18) push_special(23);
            else if (wType == 31) push_special(56);
        }
        if (wType == 419) push_special(49);
    }

    if (Attribute1 & 4) {
        if ((wType >= 0 && wType < 384 && wType != 135 && wType != 143) || (wType >= 384 && wType <= 390)) {
            push_special(64);
        }
    }

    int option3 = (Attribute1 & 3) + 4 * ((Attribute2 >> 6) & 1);
    if (option3) {
        if (wType == 419) {
            if (option3 & 1) push_special(84, 5);
            if (option3 & 2) push_special(83, 50);
            if (option3 & 4) push_special(77, 5);
        } else {
            if (wType >= 0 && wType < 160 && wType != 135 && wType != 143) {
                push_special(60, (BYTE)(4 * option3));
                ip->RequireStrength = (WORD)(ip->RequireStrength + 5 * option3);
            }
            if (wType >= 160 && wType < 192) {
                push_special(61, (BYTE)(4 * option3));
                ip->RequireStrength = (WORD)(ip->RequireStrength + 5 * option3);
            }
            if (wType >= 192 && wType < 224) {
                push_special(62, (BYTE)(5 * option3));
                ip->RequireStrength = (WORD)(ip->RequireStrength + 5 * option3);
            }
            if (wType >= 224 && wType < 384) {
                push_special(63, (BYTE)(4 * option3));
                ip->RequireStrength = (WORD)(ip->RequireStrength + 5 * option3);
            }
            if (wType >= 424 && wType < 448) {
                push_special(65, (BYTE)option3);
            }
            switch (wType) {
            case 384: push_special(65, (BYTE)option3); break;
            case 385: push_special(61, (BYTE)(4 * option3)); break;
            case 386: push_special(60, (BYTE)(4 * option3)); break;
            case 387:
                if (itemExcel & 0x20) push_special(65, (BYTE)option3);
                else push_special(60, (BYTE)(4 * option3));
                break;
            case 388:
                if (itemExcel & 0x20) push_special(61, (BYTE)(4 * option3));
                else push_special(65, (BYTE)option3);
                break;
            case 389:
                if (itemExcel & 0x20) push_special(60, (BYTE)(4 * option3));
                else push_special(65, (BYTE)option3);
                break;
            case 390:
                if (itemExcel & 0x20) push_special(60, (BYTE)(4 * option3));
                else push_special(61, (BYTE)(4 * option3));
                break;
            default: break;
            }
        }
    }

    if ((wType >= 192 && wType < 384) || (wType >= 424 && wType <= 425)) {
        if (Attribute2 & 0x20) push_special(66);
        if (Attribute2 & 0x10) push_special(67);
        if (Attribute2 & 8)    push_special(68);
        if (Attribute2 & 4)    push_special(69);
        if (Attribute2 & 2)    push_special(70);
        if (Attribute2 & 1)    push_special(71);
    }

    if ((wType >= 0 && wType < 192) || (wType >= 428 && wType <= 429)) {
        if (Attribute2 & 0x20) push_special(72);
        if ((wType >= 160 && wType < 192) || wType == 428) {
            if (Attribute2 & 0x10) push_special(75, (BYTE)(*(unsigned short*)((BYTE*)CharacterAttribute + 14) / 20));
            if (Attribute2 & 8)    push_special(76);
        } else {
            if (Attribute2 & 0x10) push_special(73, (BYTE)(*(unsigned short*)((BYTE*)CharacterAttribute + 14) / 20));
            if (Attribute2 & 8)    push_special(74);
        }
        if (Attribute2 & 4) push_special(77, 7);
        if (Attribute2 & 2) push_special(78);
        if (Attribute2 & 1) push_special(79);
    }

    if ((wType >= 128 && wType < 136) || wType == 145) {
        ip->Part = 1;
        ip->Color = bExtOption ? 4 : (itemExcel > 0 ? 3 : 0);
        return;
    }
    if (wType >= 0) {
        if (wType < 192) { ip->Part = 0; ip->Color = bExtOption ? 4 : (itemExcel > 0 ? 3 : 0); return; }
        if (wType < 224) { ip->Part = 1; ip->Color = bExtOption ? 4 : (itemExcel > 0 ? 3 : 0); return; }
        if (wType < 256) { ip->Part = 2; ip->Color = bExtOption ? 4 : (itemExcel > 0 ? 3 : 0); return; }
        if (wType < 288) { ip->Part = 3; ip->Color = bExtOption ? 4 : (itemExcel > 0 ? 3 : 0); return; }
        if (wType < 320) { ip->Part = 4; ip->Color = bExtOption ? 4 : (itemExcel > 0 ? 3 : 0); return; }
        if (wType < 352) { ip->Part = 5; ip->Color = bExtOption ? 4 : (itemExcel > 0 ? 3 : 0); return; }
        if (wType < 384) { ip->Part = 6; ip->Color = bExtOption ? 4 : (itemExcel > 0 ? 3 : 0); return; }
        if (wType < 391) { ip->Part = 7; ip->Color = bExtOption ? 4 : (itemExcel > 0 ? 3 : 0); return; }
    }
    if (wType >= 416 && wType < 424) { ip->Part = 8; ip->Color = bExtOption ? 4 : (itemExcel > 0 ? 3 : 0); return; }
    if (wType >= 424 && wType < 428) { ip->Part = 10; ip->Color = bExtOption ? 4 : (itemExcel > 0 ? 3 : 0); return; }
    if (wType >= 428 && wType < 448) { ip->Part = 9; ip->Color = bExtOption ? 4 : (itemExcel > 0 ? 3 : 0); return; }
    ip->Part = (BYTE)-1;
    ip->Color = bExtOption ? 4 : (itemExcel > 0 ? 3 : 0);
}

// 2026-05-08: ItemValue is the IDA-name alias for FUN_0047c690. Previously
// returned 0 unconditionally → all sell-price calculations in Item_Click
// Handler / RenderItemInfo / shop UI yielded zero gold. Delegate to the real
// impl in stubs_helpers.cpp.
extern int __cdecl FUN_0047c690(void* item_v, int sellMode);
int __cdecl ItemValue(ITEM* ip, unsigned int goldType) {
    return FUN_0047c690((void*)ip, (int)goldType);
}
