// stubs_mouse_hover.cpp
//
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 1180-2577 (1398 lines).
//
// "Mouse hover helpers" section + neutralized stubs:
//   FUN_004f8480 — Terrain_TilePick
//   FUN_004afa40 — ItemOnGround_HoverTest (NEUTRALIZED, returns -1)
//   FUN_004b0240 — SpecialObject_HoverTest (NEUTRALIZED)
//   FUN_004afdc0 — Entity_SelectNearest
//   ... and additional misc/legacy stubs through line 2577.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

extern "C" void DbgLogPublic(const char* msg);
extern void __cdecl FUN_0054158c(void* ptr);

// ── [DIAG FORGE] TEMPORAL — identificar quién dibuja el cubo cyan (Lorencia) ──
// Loguea draw/spawn calls cuyo centro (pos mundial) esté dentro de ~6 tiles del
// héroe. Rate-limited a 250 líneas/segundo. NO modifica lógica del cliente.
// REMOVER cuando se identifique el renderable.
//   fn    = punto del pipeline (RenderObject/RenderMesh/RenderSprite/...)
//   type  = entity type / particle-bitmap type (-1 si N/A)
//   model = model index (-1 si N/A)
//   bmp   = bitmap id solicitado (-1 si N/A)
//   glTex = GL texture handle (-1 si N/A)
//   mesh  = mesh index (-1 si N/A)
//   blend = flags/Components de blend (-1 si N/A)
// [DIAG FORGE] — trazado selectivo del pipeline de habilidades.  No modifica
// pools ni estado GL: sólo registra los tipos visuales de combate, limitado
// por segundo, para poder contrastar spawn → bind → draw contra IDA.
extern "C" void DbgForge(const char* fn, int type, int, int, int,
                         int, int, float wx, float wy, float wz,
                         float, float, float, float)
{
    // Las habilidades 9/10/14 crean primero efectos lógicos 190..240 y éstos
    // luego emiten partículas 1180..1300.  Mantener ambos rangos permite
    // comprobar la cadena completa sin registrar el mundo entero.
    if (!fn || !((type >= 190 && type <= 240) ||
                 (type >= 1180 && type <= 1300))) return;

    static DWORD lastWindow = 0;
    static unsigned count = 0;
    DWORD now = GetTickCount();
    if (now - lastWindow >= 1000) {
        lastWindow = now;
        count = 0;
    }
    if (count++ >= 80) return;

    char line[192];
    _snprintf_s(line, sizeof(line), _TRUNCATE,
                "VIS %s type=%d pos=(%.0f,%.0f,%.0f)",
                fn, type, wx, wy, wz);
    DbgLogPublic(line);
}

#ifndef qmemcpy
#define qmemcpy(dst,src,sz) memcpy((dst),(src),(size_t)(sz))
#endif
#ifndef delete__
#define delete__(p) FUN_0054158c((unsigned char*)(p))
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
    FUN_00511680('\x01');  // EnableAlphaTest(1)
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
        FUN_005125a0(2, cx, cy, 24.0f, 24.0f, 0.0f, 0.0f, 1.0f, 1.0f, '\x01', '\x01');
    };

    // Error message o diálogo bloqueante → arrow
    if (DAT_07eaa14c != 0 || DAT_083a7c24 != 0) { draw_arrow(); return; }

    // SelectedItem (hover sobre ítem en el piso)
    if (SelectedItem != -1) {
        FUN_005125a0(5, cx, cy, 24.0f, 24.0f, 0.0f, 0.0f, 1.0f, 1.0f, '\x01', '\x01');
        return;
    }
    // SelectedNpc (hover sobre NPC) — UV animado 3×2
    if (SelectedNpc != -1) {
        FUN_005125a0(6, cx, cy, 24.0f, 24.0f,
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
        if (match) FUN_005125a0(8, cx, cy, 24.0f, 24.0f, 0.0f, 0.0f, 1.0f, 1.0f, '\x01', '\x01');
        else       FUN_005125a0(9, cx, cy, 24.0f, 24.0f, 0.0f, 0.0f, 1.0f, 1.0f, '\x01', '\x01');
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
            FUN_005125a0(4, cx, cy, 24.0f, 24.0f, 0.0f, 0.0f, 1.0f, 1.0f, '\x01', '\x01');
        } else {
            draw_arrow();
        }
        return;
    }
    // Repair active (modo taladro del smith)
    if (DAT_07eaa134 == 1) {
        FUN_005125a0(7, cx, cy, 24.0f, 24.0f, 0.0f, 0.0f, 1.0f, 1.0f,
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
        FUN_005125a0(10, cx, cy, 24.0f, 24.0f, 0.0f, 0.0f, 1.0f, 1.0f, '\x01', '\x01');
    else
        FUN_005125a0(3,  cx, cy, 24.0f, 24.0f, 0.0f, 0.0f, 1.0f, 1.0f, '\x01', '\x01');
}
// FUN_004c04a0 — implemented in src/Input/Input.cpp
// FUN_004C14E0 @ 0x004C14E0 — FPS_TickReset.
// Selects the UI font into the DC, resets text color to white (0xFFFFFFFF)
// and background to black (0xFF000000). On each 1-second interval resets
// the FPS counter (DAT_07E11DCC = 0) and updates the timestamp.
void __cdecl FUN_004c14e0(void) {
    SelectObject(DAT_055c9fec, (HGDIOBJ)(uintptr_t)DAT_055ca00c);
    DAT_00559c78 = 0xffffffff;
    DAT_00559c80 = 0xff000000;
    DWORD DVar1 = timeGetTime();
    if (999 < DVar1 - (DWORD)_DAT_07e016f0) {
        _DAT_07e016f0 = timeGetTime();
        DAT_07e11dcc = 0;
    }
}
// FUN_004c3530 — implemented in src/UI/CharMenu_Build.cpp (char menu builder, 379 lines)
// FUN_004c2420 — implemented in src/UI/CharMenu_Build.cpp (CharMenu_RenderTextList)
// FUN_004c2880 — implemented in src/UI/CharMenu_Build.cpp (CharMenu_AppendSkillReq)
// FUN_004c2c10 — implemented in src/UI/CharMenu_Build.cpp (CharMenu_AppendStatRows)
// FUN_004c2d50 — implemented in src/UI/CharMenu_Build.cpp (CharMenu_AppendSkillDesc)
// FUN_004c2e20 — implemented in src/UI/CharMenu_Build.cpp (CharMenu_BuildStatRequirements)
// FUN_004cbdf0 — implemented in src/Net/Net_PacketSession.cpp
// FUN_004ecb00 — implemented in src/Net/Net_PacketSession.cpp
// FUN_004f64d0 — implemented in src/Scene/Scene_MapTick.cpp
// FUN_004f6ef0 — implemented in src/Terrain/Terrain_Tile.cpp
// FUN_004f6f10 — implemented in src/Terrain/Terrain_Tile.cpp
// FUN_004f6f30 — implemented in src/Terrain/Terrain_Tile.cpp
// FUN_004f7960 — implemented in src/Terrain/Terrain_Light.cpp
// FUN_004f8eb0 — implemented in src/Render/Camera.cpp
// FUN_004f95e0 — implemented in src/Terrain/Terrain_Light.cpp
// FUN_004f9db0 — implemented in src/Math/Math_3D.cpp
// FUN_004fa0b0 — implemented in src/Math/Math_3D.cpp
// FUN_004fa110 — implemented in src/Math/Math_3D.cpp
// FUN_004fa4d0 — implemented in src/Math/Math_3D.cpp
// FUN_004fa5c0 — implemented in src/Render/Camera.cpp
// FUN_004fa930 — implemented in src/Terrain/Terrain_Light.cpp
// FUN_004fae00 — implemented in src/Render/Entity_DrawByType.cpp (Entity_DrawByType, 807 lines)
// FUN_004fc030 — implemented in src/Render/Entity_Render.cpp
// FUN_004fc070 — implemented in src/Render/Entity_Render_3D.cpp (Entity_SpawnEffects)
// FUN_004fd800 — defined in Terrain_Render.cpp, no stub needed

// FUN_004f6cb0 @ 0x004F6CB0 — ExitProgram
// Shows the fatal error message at lpText_07d2aa08 then destroys the main window.
void __cdecl FUN_004f6cb0(void)
{
    MessageBoxA(g_hWnd, lpText_07d2aa08, NULL, 0);
    SendMessageA(g_hWnd, 2, 0, 0);  // WM_DESTROY
}

// FUN_004f6eb0 @ 0x004F6EB0 — BuxConvert(BYTE* Buffer, int Size)
// XOR-obfuscates Buffer in place using a 3-byte rotating key at DAT_0055a770.
// Used for terrain block (.bux) encryption/decryption.
void __cdecl FUN_004f6eb0(int data, int size)
{
    BYTE* Buffer = (BYTE*)data;
    BYTE* key = &DAT_0055a770;   // 3 consecutive bytes at 0x0055a770..2
    for (int i = 0; i < size; i++)
        Buffer[i] ^= key[i % 3];
}

// MapFileDecrypt — decrypt EncTerrain%d.{map,obj} with MU map-file algorithm.
// Reference: público en source leaks de Mu Online Season clients (válido para 0.97.x).
// Algorithm: each byte is XOR'd with a 16-byte rolling key, minus a per-byte
// running counter (wKey) that is updated based on the original ciphertext byte.
//   plain[i] = (cipher[i] ^ MapFileKey[i & 0x0F]) - wKey
//   wKey = cipher[i] + 0x3D    (note: cipher byte, not plain)
// Initial wKey = 0x5E.
//
// BUG-FIX 2026-05-01: BuxConvert (3-byte XOR) NO sirve para .map/.obj — esos
// archivos usan un algorithm distinto (de ahí el prefix "Enc" mientras que .att
// usa BuxConvert simple).
void MapFileDecrypt(BYTE* buf, int size)
{
    static const BYTE MapFileKey[16] = {
        0xD1, 0x73, 0x52, 0xF6, 0xD2, 0x9A, 0xCB, 0x27,
        0x3E, 0xAF, 0x59, 0x31, 0x37, 0xB3, 0xE7, 0xA2
    };
    BYTE wKey = 0x5E;
    for (int i = 0; i < size; i++) {
        BYTE enc = buf[i];
        buf[i]   = (BYTE)((enc ^ MapFileKey[i & 0x0F]) - wKey);
        wKey     = (BYTE)(enc + 0x3D);
    }
}

// FUN_0050f700 @ 0x0050F700 — Map_Load(filename)
// Opens map file and reads blocks into the buffer at DAT_07e0ffc8 via the
// format descriptor at DAT_00560694 (10 iterations of 0x100 bytes each).
//
// 2026-05-03: AUTO-SKIP removed. DAT_07e0ffc8 is properly sized
// `char[10 * 0x100]` (2560 bytes) in globals.cpp; only the bound was a
// literal source-binary address. Iteration count is exactly 10.
void __cdecl FUN_0050f700(const char* map_name)
{
    FILE* pFVar1 = (FILE*)fopen(map_name, (const char*)DAT_00559b74);
    if (!pFVar1) return;
    char* puVar2 = (char*)&DAT_07e0ffc8[0];
    for (int i = 0; i < 10; ++i, puVar2 += 0x100) {
        FUN_00543274((int*)pFVar1, (void*)&DAT_00560694);
    }
    fclose(pFVar1);
    (void)puVar2;
}

// FUN_004ff5a0 @ 0x004ff5a0 — Entity_New (scene entity allocator)
// Allocates 0x1BC-byte entity node, zeroes it, links into doubly-linked list
// at DAT_083A021C[cell]/DAT_083A0220[cell] (16x16 grid, 4-DWORD stride).
//
// Cálculo de celda (verbatim contra asm @ 0x4ff5a7):
//   FLD [pos.x]  FMUL [DAT_00552d20]  __ftol  → grid_x
//   FLD [pos.y]  FMUL [DAT_00552d20]  __ftol  → grid_y
//   DAT_00552d20 = 6.25e-4f = 1/1600  (cell = 1600 unidades de mundo)
// __ftol usa el modo de redondeo actual (por defecto nearest-even), no
// truncate-toward-zero. Para los mocks del login todos caen en (0,0)/(0,1).
// Rechaza si grid_x<0 || grid_y<0 || grid_x>15 || grid_y>15.
void* __cdecl FUN_004ff5a0(int param_1, float* param_2, float* param_3, float param_4) {
    // BUG-FIX 2026-04-26 (audit #12): __ftol implementa truncate-toward-zero
    // (semántica de cast C de float→int), no nearest-even. lrintf redondeaba al
    // más cercano y divergía en negativos (lrintf(-1.5)=-2 vs __ftol(-1.5)=-1).
    int igx = (int)(param_2[0] * _DAT_00552d20);
    int igy = (int)(param_2[1] * _DAT_00552d20);
    if (igx < 0 || igy < 0 || igx > 15 || igy > 15) return nullptr;
    BYTE  bVar4 = (BYTE)((char)igx * 0x10 + (char)igy);
    UINT  uVar2 = (UINT)bVar4;

    UINT* puVar3 = (UINT*)operator_new(0x1bc);
    UINT* p = puVar3;
    for (int i = 0x6f; i != 0; i--) *p++ = 0;

    // link into per-cell doubly-linked list (head=DAT_083a021c, tail=DAT_083a0220)
    UINT** headArr = (UINT**)&DAT_083a021c;
    UINT** tailArr = (UINT**)((char*)&DAT_083a021c + 4);
    if (headArr[uVar2 * 4] == nullptr) {
        puVar3[0x6d] = 0; puVar3[0x6e] = 0;
        headArr[uVar2 * 4] = puVar3;
    } else {
        UINT* prevTail = tailArr[uVar2 * 4];
        *(UINT**)((char*)prevTail + 0x1b8) = puVar3;
        puVar3[0x6d] = (UINT)prevTail;
        puVar3[0x6e] = 0;
    }
    tailArr[uVar2 * 4] = puVar3;

    // common initialisation
    *(BYTE*)puVar3                  = 1;           // active flag
    *(BYTE*)(puVar3 + 0x58)         = 0;           // +0x160
    *(BYTE*)((char*)puVar3 + 0x161) = 0;
    *(BYTE*)(puVar3 + 0x37)         = 1;           // +0x0dc
    *(BYTE*)(puVar3 + 0x39)         = 0;           // +0x0e4
    *(BYTE*)(puVar3 + 0x44)         = 0;           // +0x110
    *(short*)((char*)puVar3 + 2)    = (short)param_1;
    puVar3[3]    = *(UINT*)&param_4;               // extra (+0x0c)
    puVar3[0x5a] = 0x3f800000;                     // scale_x = 1.0f  (+0x168)
    puVar3[0x59] = 0x3f800000;                     // scale_y = 1.0f  (+0x164)
    puVar3[0x33] = 0x3e23d70a;                     // speed 0.16f     (+0x0cc)
    puVar3[0x35] = 0x42480000;                     // 50.0f           (+0x0d4)
    puVar3[0x16] = 0xffffffff;                     // RGBA white      (+0x058)
    *(BYTE*)((char*)puVar3 + 0x105) = 0;           // anim_state
    *(BYTE*)((char*)puVar3 + 0x106) = 0;           // anim_state_prev
    puVar3[0x42] = 0; puVar3[0x43] = 0;
    *(BYTE*)(puVar3 + 0x6c) = bVar4;               // grid cell       (+0x1b0)
    puVar3[0x19] = 0xffffffff;                     // flags           (+0x064)
    puVar3[0x1a] = 0x3f800000;                     // entity scale 1.0f (+0x068)
    puVar3[0x1b] = 0; puVar3[0x1c] = 0; puVar3[0x1e] = 0;
    puVar3[0x34] = 0xc1f00000;                     // -30.0f          (+0x0d0)
    puVar3[0x20] = 0;
    puVar3[4] = *(UINT*)&param_2[0];               // world_x         (+0x010)
    puVar3[5] = *(UINT*)&param_2[1];               // world_y         (+0x014)
    puVar3[6] = *(UINT*)&param_2[2];               // world_z         (+0x018)
    puVar3[7] = *(UINT*)&param_3[0];               // target_x        (+0x01c)
    puVar3[8] = *(UINT*)&param_3[1];               // target_y        (+0x020)
    puVar3[9] = *(UINT*)&param_3[2];               // target_z        (+0x024)
    puVar3[0x46] = 0xc2200000;                     // bbox_min_x -40f (+0x118)
    puVar3[0x47] = 0xc2200000;                     // bbox_min_y -40f (+0x11c)
    puVar3[0x3a] = 0; puVar3[0x3b] = 0; puVar3[0x3c] = 0;
    puVar3[0xa]  = 0; puVar3[0xb]  = 0; puVar3[0xc]  = 0;
    puVar3[0x30] = 0; puVar3[0x31] = 0; puVar3[0x32] = 0;
    puVar3[0x48] = 0;
    puVar3[0x49] = 0x42200000;                     // bbox_max_x 40f  (+0x124)
    puVar3[0x4a] = 0x42200000;                     // bbox_max_y 40f  (+0x128)
    puVar3[0x4b] = 0x42a00000;                     // bbox_max_z 80f  (+0x12c)

    // login/char-select scene type overrides (g_GameState 2 or 4)
    if (DAT_005615c0 == 2 || DAT_005615c0 == 4) {
        switch (param_1) {
        case 0x3c:
            puVar3[3]=0x3f4ccccd; puVar3[0x3a]=puVar3[0x3b]=puVar3[0x3c]=0x3e4ccccd;
            *(BYTE*)(puVar3+0x37)=1; break;
        case 0xa0:
            puVar3[3]=0x3d343958; puVar3[0x19]=1; puVar3[0x1a]=0x3f800000;
            puVar3[0x3a]=puVar3[0x3b]=puVar3[0x3c]=0x3f800000;
            *(BYTE*)(puVar3+0x37)=0; break;
        case 0xa1:
            puVar3[3]=0x3f4ccccd; puVar3[0x19]=0; puVar3[0x1a]=0x3f800000;
            puVar3[0x3a]=puVar3[0x3b]=puVar3[0x3c]=0x3f800000; break;
        case 0xa2:
            // IDA-faithful MODEL_MUGAME (case 3 en byte_4FFAA4[Type-60]):
            //   scale=0.6, [0x64]=1, [0xDC]=0. No escribe offsets 58/59/60
            //   (bodyLight). memset(0) previo deja bodyLight=0, pero lightEnable=0
            //   → nunca se lee. Revertido el "PORT FIX" previo porque el tint
            //   (1,1,1) no tenía efecto visual (lightEnable==0) y divergía de IDA.
            puVar3[3]=0x3f19999a; puVar3[0x19]=1;
            *(BYTE*)(puVar3+0x37)=0; break;
        case 0xa3:
            puVar3[3]=0x40400000; puVar3[0x19]=0;
            puVar3[0x3a]=puVar3[0x3b]=puVar3[0x3c]=0x3f000000; break;
        case 0xa4:
            puVar3[0x19]=10;
            puVar3[0x3a]=puVar3[0x3b]=puVar3[0x3c]=0x3f800000; break;
        }
    }

    // game-substate overrides; some cases return directly (skipping FUN_004ff580),
    // others call it and return; fall-through also calls it.
    switch (DAT_0055a7ac) {
    case 0:
        switch (param_1) {
        default: goto lbl_skip_init;
        case 6: case 0x91: case 0x92: break;
        case 0x34: puVar3[0x19]=1; return puVar3;
        case 0x3b: puVar3[0x33]=0; return puVar3;
        case 0x50: puVar3[0x34]=0xc2480000; return puVar3;
        case 0x5a: case 0x96: puVar3[0x19]=1; [[fallthrough]];
        case 0x60: case 0x61: puVar3[0x33]=0x3e99999a; return puVar3;
        case 0x62: case 0x77: puVar3[0x19]=2; return puVar3;
        case 0x69: puVar3[0x19]=3; return puVar3;
        case 0x75: case 0x7a: puVar3[0x19]=4; return puVar3;
        case 0x76: puVar3[0x19]=8; return puVar3;
        case 0x85:
            FUN_004ff580(puVar3);
            puVar3[0x49]=0x42200000; puVar3[0x4a]=0x42200000; puVar3[0x4b]=0x43200000;
            puVar3[0x16]=0xfffffffe; return puVar3;
        }
        break;
    case 1:
        if (param_1==0x3b) break;
        if (param_1==0x3c) {
            FUN_004ff580(puVar3);
            puVar3[0x49]=0x42200000; puVar3[0x4a]=0x42200000; puVar3[0x4b]=0x43200000;
            puVar3[0x16]=0xfffffffe;
        }
        return puVar3;
    case 2:
        switch (param_1) {
        default: goto lbl_skip_init;
        case 0x13: case 0x5c: case 0x5d: puVar3[0x19]=0; return puVar3;
        case 0x14: case 0x41: case 0x56: case 0x58:
            // copy current pos to previous, compute facing angle mod 360
            puVar3[0xb]=puVar3[8]; puVar3[0xf]=puVar3[6]; puVar3[0xd]=puVar3[4];
            puVar3[0xa]=puVar3[7]; puVar3[0xe]=puVar3[5];
            { UINT ua = puVar3[9];
              float fa = (float)((int)((long long)(unsigned long long)ua % 0x168));
              puVar3[9]=*(UINT*)&fa; puVar3[0xc]=*(UINT*)&fa; }
            return puVar3;
        case 0x16: case 0x19: case 0x28: case 0x2d: case 0x37: case 0x49: break;
        case 0x36: case 0x38: puVar3[0x19]=1; return puVar3;
        case 0x4e: puVar3[0x19]=3; return puVar3;
        case 0x5b:
            FUN_004ff580(puVar3);
            puVar3[0x49]=0x42200000; puVar3[0x4a]=0x42200000; puVar3[0x4b]=0x43200000;
            puVar3[0x16]=0xfffffffe; return puVar3;
        case 100: puVar3[0x16]=0xfffffffe; return puVar3;
        }
        [[fallthrough]];
    case 3:
        switch (param_1) {
        default: goto lbl_skip_init;
        case 1: puVar3[0x19]=1; return puVar3;
        case 8: break;
        case 9: puVar3[0x19]=3; return puVar3;
        case 0x11: case 0x13: case 0x25: puVar3[0x19]=0; return puVar3;
        case 0x12: puVar3[0x19]=2; return puVar3;
        case 0x26: FUN_004ff580(puVar3); puVar3[0x16]=0xfffffffe; return puVar3;
        }
        [[fallthrough]];
    default:
        goto lbl_skip_init;
    case 7:
        if (param_1==0x27) { FUN_004ff580(puVar3); puVar3[0x16]=0xfffffffe; return puVar3; }
        return puVar3;
    case 8:
        if (param_1==0x4e) { FUN_004ff580(puVar3); goto lbl_skip_init; }
        return puVar3;
    }
    FUN_004ff580(puVar3);
lbl_skip_init:
    return puVar3;
}
// FUN_004FFFA0 @ 0x004FFFA0 — DeleteBug(Owner).
// Walk butterfly slot array (10 entries × 0x1BC stride at DAT_083a1218..DAT_083a1218+0x1158),
// clear active flag (slot[0]=0) on every slot whose owner field (slot+0xFC = DWORD index 63)
// matches `Owner`. Each slot occupies 111 DWORDs (= 0x1BC bytes); we step in DWORD units.
// Ported verbatim from IDA reference 0045C8C0_ChangeCharacterExt's helper.
void __cdecl FUN_004fffa0(DWORD Owner) {
    DWORD* Butterflies = (DWORD*)DAT_083a1218;
    DWORD* End         = (DWORD*)(DAT_083a1218 + 0x1158);
    do {
        if (*(BYTE*)Butterflies) {
            if (Butterflies[63] == Owner) {
                *(BYTE*)Butterflies = 0;
            }
        }
        Butterflies += 111;
    } while ((int)(uintptr_t)Butterflies < (int)(uintptr_t)End);
}

// FUN_004FFFD0 @ 0x004FFFD0 — CreateBug(Type, Position[3], Owner, SubType[, LinkBone]).
// Allocates a free slot in butterfly/effect array at DAT_083A1218 (stride 0x1BC, 10 entries up
// to DAT_083A1218+0x1158). Initialises slot from owner entity (param_3) and world_pos (param_2).
// Per IDA: only spawns when owner class==390 OR Type==816.
// Special cases: type 0x330(816)/0x331(817) → randomise spawn XY around owner pos.
//                type 0xC3(195)/0x10B(267)  → set bug-color tint to 0.9 (0x3F666666 = "fff?").
// Note: original IDA signature has 5th `LinkBone` param but it's never read; our 4-arg form
// is functionally identical. The Ghidra decompile labelled this Entity_Spawn — that was wrong.
void __cdecl FUN_004fffd0(int param_1, void *param_2_v, void *param_3_v, int param_4) {
    DWORD *param_2 = (DWORD*)param_2_v;
    int param_3 = (int)(uintptr_t)param_3_v;
    if ((*(short*)(param_3 + 2) == 0x186) || (param_1 == 0x330)) {
        char *pcVar5 = DAT_083a1218;
        while (*pcVar5 != '\0') {
            pcVar5 += 0x1bc;
            if (0x83a236f < (int)pcVar5) return;
        }
        *(DWORD*)(pcVar5 + 4)   = param_4;
        pcVar5[0x58] = pcVar5[0x59] = pcVar5[0x5a] = pcVar5[0x5b] = -1;
        pcVar5[100]  = pcVar5[0x65] = pcVar5[0x66] = pcVar5[0x67] = -1;
        *(short*)(pcVar5 + 2)   = (short)param_1;
        pcVar5[0x68] = pcVar5[0x69] = '\0'; pcVar5[0x6a] = (char)0x80; pcVar5[0x6b] = '?';
        pcVar5[0x164]= pcVar5[0x165]= '\0'; pcVar5[0x166]= (char)0x80; pcVar5[0x167]= '?';
        *pcVar5 = '\x01';
        pcVar5[0x160] = '\0';
        pcVar5[0xdc]  = '\x01';
        pcVar5[0xe4]  = '\0';
        pcVar5[0x161] = '\0';
        pcVar5[0x110] = '\0';
        *(int*)(pcVar5 + 0xfc)  = param_3;
        pcVar5[0x0c]='3'; pcVar5[0x0d]='3'; pcVar5[0x0e]='3'; pcVar5[0x0f]='?';
        pcVar5[0x60]='\x1e'; pcVar5[0x61]=pcVar5[0x62]=pcVar5[63]='\0';
        pcVar5[0x168]=pcVar5[0x169]=pcVar5[0x16a]=pcVar5[0x16b]='\0';
        *(DWORD*)(pcVar5+0x10)=param_2[0]; *(DWORD*)(pcVar5+0x14)=param_2[1]; *(DWORD*)(pcVar5+0x18)=param_2[2];
        *(DWORD*)(pcVar5+0x1c)=*(DWORD*)(param_3+0x1c);
        *(DWORD*)(pcVar5+0x20)=*(DWORD*)(param_3+0x20);
        *(DWORD*)(pcVar5+0x24)=*(DWORD*)(param_3+0x24);
        pcVar5[0x10c]=pcVar5[0x10d]=pcVar5[0x10e]=pcVar5[0x10f]='\0';
        pcVar5[0x108]=pcVar5[0x109]=pcVar5[0x10a]=pcVar5[0x10b]='\0';
        pcVar5[0xe8]=pcVar5[0xe9]='\0'; pcVar5[0xea]=pcVar5[0xeb]='@';
        pcVar5[0xec]=pcVar5[0xed]='\0'; pcVar5[0xee]=pcVar5[0xef]='@';
        pcVar5[0xf0]=pcVar5[0xf1]='\0'; pcVar5[0xf2]=pcVar5[0xf3]='@';
        pcVar5[0xcc]=pcVar5[0xcd]=pcVar5[0xce]='\0'; pcVar5[0xcf]='?';
        short sVar4 = (short)param_1;
        if (sVar4 == 0x330) {
            UINT u; int v;
            u=_rand()&0x800001ff; if((int)u<0)u=(u-1|0xfffffe00)+1; *(float*)(pcVar5+0x10)=(float)(int)(u-0x100)+*(float*)(param_3+0x10);
            u=_rand()&0x800001ff; if((int)u<0)u=(u-1|0xfffffe00)+1; *(float*)(pcVar5+0x14)=(float)(int)(u-0x100)+*(float*)(param_3+0x14);
            u=_rand()&0x8000007f; if((int)u<0)u=(u-1|0xffffff80)+1; *(float*)(pcVar5+0x18)=(float)(int)(u+0x80)+*(float*)(param_3+0x18);
        } else if (sVar4 == 0x331) {
            UINT u;
            u=_rand()&0x8000007f; if((int)u<0)u=(u-1|0xffffff80)+1; *(float*)(pcVar5+0x10)=(float)(int)(u-0x40)+*(float*)(param_3+0x10);
            u=_rand()&0x8000007f; if((int)u<0)u=(u-1|0xffffff80)+1; *(float*)(pcVar5+0x14)=(float)(int)(u-0x40)+*(float*)(param_3+0x14);
            *(DWORD*)(pcVar5+0x18)=*(DWORD*)(param_3+0x18);
            *(float*)(pcVar5+0x18)=(float)(_rand()%100)+FUN_004f7500(*(float*)(pcVar5+0x10), *(float*)(pcVar5+0x14));
        } else if ((sVar4==0xc3)||(sVar4==0x10b)) {
            strncpy(pcVar5+0x0c,"fff?",4);
        }
    }
}
// FUN_00500970 — implemented in src/Render/Entity_Render.cpp
// FUN_00500e80 — implemented in src/Render/Weather.cpp (Weather_Update)
// FUN_00502320 — implemented in src/Render/Ambient_Particles.cpp (Ambient_ParticleUpdate)
// FUN_00503760 — implemented in src/Util/Misc.cpp
// FUN_00503830 — implemented in src/Render/Entity_Render.cpp
// FUN_00504b50 — implemented in src/Render/Entity_DrawSetup.cpp (Entity_SetColorAndRender)
// FUN_00505970 — implemented in src/Render/Entity_Render.cpp
// FUN_00505a10 — implemented in src/Render/Entity_Render.cpp
// FUN_0050e5a0 @ 0x0050e5a0 — OpenWorld(int Map)
// Per IDA decompile (raw/0050E5A0_OpenWorld.c, 1500 bytes).
// Loads all terrain and tile textures for the current world map.
//
// World name = "World<N>" where N = DAT_0055a7ac+1 (capped at 12 for dungeons 11-16).
// Loads Terrain.map, Terrain<N>.att, terrain.obj (or terrain<N>.obj for maps 2/3),
// TerrainHeight.bmp, TerrainLight.jpg, then 14 tile JPGs (slots 0x23-0x30) +
// 3 alpha-overlay TGAs (slots 0x32-0x34) + leaf01/02 + rain01/02 (always from
// World1) + rain03 (always from World10).
//
// BUG-FIX 2026-04-27: fixed path strings to match IDA exactly:
//   - "World_%d"           → "World%d"            (no underscore)
//   - "Data/%s/Terrain/%d" → "Data/%s/Terrain%d"  (no extra slash)
//   - "Data/%s/terrain/%d" → "Data/%s/terrain%d"  (no extra slash)
//   - rain01/02 use "World1" hardcoded; rain03 uses "World10" hardcoded.
//   - Pass FileName to OpenTerrainAttribute (was called with no args → no-op).
void __cdecl FUN_0050e5a0(void) {
    BYTE  uVar1;
    CHAR  world_name[32];
    CHAR  local_40[64];

    FUN_004ffd50();             // DeleteObjects
    FUN_00509190();             // DeleteNpcs
    FUN_00509880();             // DeleteMonsters
    FUN_00502b80();             // ClearItems
    FUN_0045abb0(DAT_05826cac); // ClearCharacters(HeroKey)

    // BUG-FIX 2026-04-28: limpiar TODOS los pools de char-select que
    // sobreviven al world load. Sin esto los tick-functions iteran slots
    // con punteros garbage → AV.
    memset(DAT_07abf5f0, 0, sizeof(DAT_07abf5f0));   // particle pool (3000×0x70)
    memset(DAT_07c5ab3c, 0, sizeof(DAT_07c5ab3c));   // skill effect pool (200×0x70)
    memset(DAT_07b11670, 0, sizeof(DAT_07b11670));   // effect pool (124×0x1bc)
    memset(DAT_07b27150, 0, sizeof(DAT_07b27150));   // joint pool (200×0x9d8)
    memset(DAT_07c74ec8, 0, sizeof(DAT_07c74ec8));   // fade-effect pool (40×0x1bc)
    memset(DAT_07c80128, 0, sizeof(DAT_07c80128));   // spark pool (100×0x70)
    memset(DAT_07c82cdc, 0, sizeof(DAT_07c82cdc));   // flare pool (63×0x70)
    memset(DAT_083a2e90, 0, sizeof(DAT_083a2e90));   // boids pool (10×0x1bc)
    memset(DAT_083a2f78, 0, sizeof(DAT_083a2f78));   // ambient particle pool (10×0x1bc)
    memset(DAT_07e016f8, 0, sizeof(DAT_07e016f8));   // tooltip pool (26×0x254)

    FUN_0050c4d0();             // OpenWorldModels

    int iVar2 = DAT_0055a7ac + 1;
    if (DAT_0055a7ac >= 11 && DAT_0055a7ac <= 16) iVar2 = 12;

    crt_sprintf(world_name, "World%d", iVar2);

    // BUG-FIX 2026-05-01: los archivos reales en bin/Client/Data/World%d/
    // son EncTerrain%d.{map,att,obj} (versiones encrypted). El loader de
    // texturas tiene auto-fallback OZ*↔jpg/tga, pero los loaders de map/
    // att/obj NO. Sin esto los modelos cargan (Object*.bmd OK) pero las
    // INSTANCIAS (qué objeto va dónde) jamás se leen → mapa renderiza
    // solo terreno + hero, sin casas/NPCs estáticos/props.
    crt_sprintf(local_40, "Data/%s/EncTerrain%d.map", world_name, iVar2);
    FUN_004f6f90(local_40);     // OpenTerrainMapping

    // 2026-05-04: el archivo `EncTerrain%d.att` mide 131076 bytes (formato
    // encriptado custom) pero `OpenTerrainAttribute` solo acepta 65539 bytes
    // (formato vanilla 0.97k). Sin .att cargado → DAT_0838bc70 queda en 0
    // → todas las tiles son walkable → atravesamos casas y NPCs.
    // Intentamos el archivo unencrypted `Terrain%d.att` primero (mismo formato
    // que IDA espera). Fallback a EncTerrain*.att si no existe.
    crt_sprintf(local_40, "Data/%s/Terrain%d.att", world_name, iVar2);
    if (FUN_004f6ce0(local_40) == 0) {
        crt_sprintf(local_40, "Data/%s/EncTerrain%d.att", world_name, iVar2);
        FUN_004f6ce0(local_40);     // OpenTerrainAttribute(FileName)
    }

    crt_sprintf(local_40, "Data/%s/EncTerrain%d.obj", world_name, iVar2);
    FUN_004ffe70(local_40);     // OpenObjectsEnc

    uVar1 = DAT_0055a7c4;
    if (DAT_083a410c != '\0') DAT_0055a7c4 = 0;

    // BUG-FIX 2026-05-01: archivos reales en filesystem son OZ* (encrypted),
    // no .bmp/.jpg/.tga. La función FUN_00529740 no hace ext-swap automático
    // a menos que DAT_0055a7c4 != 0 — y en in-game está en 0. Usamos extensiones
    // reales directamente para que fopen abra el archivo correcto.
    crt_sprintf(local_40, "%s/TerrainHeight.OZB", world_name); FUN_004f7270(local_40);
    crt_sprintf(local_40, "%s/TerrainLight.OZJ",  world_name); FUN_004f7250(local_40);

    // Tile textures (OZJ: slots 0x23-0x30; OZT alpha overlays: slots 0x32-0x34)
    crt_sprintf(local_40, "%s/TileGrass01.OZJ",  world_name); FUN_00529740(local_40, 0x23, 0x2600, 0x2901, 0, '\x01');
    crt_sprintf(local_40, "%s/TileGrass01.OZT",  world_name); FUN_00529bd0(local_40, 0x32, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileGrass02.OZT",  world_name); FUN_00529bd0(local_40, 0x33, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileGrass03.OZT",  world_name); FUN_00529bd0(local_40, 0x34, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileGrass02.OZJ",  world_name); FUN_00529740(local_40, 0x24, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileGround01.OZJ", world_name); FUN_00529740(local_40, 0x25, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileGround02.OZJ", world_name); FUN_00529740(local_40, 0x26, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileGround03.OZJ", world_name); FUN_00529740(local_40, 0x27, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileWater01.OZJ",  world_name); FUN_00529740(local_40, 0x28, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileWood01.OZJ",   world_name); FUN_00529740(local_40, 0x29, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileRock01.OZJ",   world_name); FUN_00529740(local_40, 0x2a, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileRock02.OZJ",   world_name); FUN_00529740(local_40, 0x2b, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileRock03.OZJ",   world_name); FUN_00529740(local_40, 0x2c, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileRock04.OZJ",   world_name); FUN_00529740(local_40, 0x2d, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileRock05.OZJ",   world_name); FUN_00529740(local_40, 0x2e, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileRock06.OZJ",   world_name); FUN_00529740(local_40, 0x2f, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/TileRock07.OZJ",   world_name); FUN_00529740(local_40, 0x30, 0x2600, 0x2901, 0, '\0');
    crt_sprintf(local_40, "%s/leaf01.OZT",  world_name); FUN_00529bd0(local_40, 100,  0x2600, 0x2900, 0, '\0');
    crt_sprintf(local_40, "%s/leaf01.OZJ",  world_name); FUN_00529740(local_40, 100,  0x2600, 0x2900, 0, '\0');
    crt_sprintf(local_40, "%s/leaf02.OZJ",  world_name); FUN_00529740(local_40, 0x65, 0x2600, 0x2900, 0, '\0');
    // rain01/02 always loaded from World1; rain03 always from World10 (per IDA).
    crt_sprintf(local_40, "World1/rain01.OZT"); FUN_00529bd0(local_40, 0x66, 0x2600, 0x2900, 0, '\0');
    crt_sprintf(local_40, "World1/rain02.OZT"); FUN_00529bd0(local_40, 0x67, 0x2600, 0x2900, 0, '\0');
    crt_sprintf(local_40, "World10/rain03.OZT"); FUN_00529bd0(local_40, 0x68, 0x2600, 0x2900, 0, '\0');

    if (DAT_083a410c != '\0') DAT_0055a7c4 = uVar1;
}

// FUN_0050f690 @ 0x0050f690 — Font_Init
// Resets font state, loads FontInput.tga (slot 0) and FontTest.tga (slot 1) as TGA,
// then builds font layout (FUN_0050f5f0) and character-to-texture map (FUN_0040f570).
void __cdecl FUN_0050f690(void) {
    FUN_0043f2d0();
    FUN_00529bd0("Interface/FontInput.tga", 0, 0x2600, 0x2900, 0, '\x01');
    FUN_00529bd0("Interface/FontTest.tga",  1, 0x2600, 0x2900, 0, '\x01');
    FUN_0050f5f0(DAT_055ca004);
    FUN_0040f570(DAT_055c9ff8, (int)lpData_055ca044, DAT_055ca004);
}
// FUN_0050fcf0 — implemented in src/Scene/Scene_Resources.cpp
// FUN_0050ff10 — implemented in src/Scene/Scene_Resources.cpp
// FUN_0050ff50 — implemented in src/Scene/Scene_Resources.cpp
// FUN_005102c0 — implemented in src/Scene/Scene_Resources.cpp
// FUN_00510320 @ 0x00510320 — UI_LoadTextures (complete game asset loader)
// Loads: cursor TGAs (slots 2-10), interface JPGs (0xE6-0x101, 0x500-0x507),
// effect JPGs/TGAs (0x47E-0x567), then invokes all model, data and sound preload routines.
void __cdecl FUN_00510320(void) {
    CHAR local_64[100];
    // Cursor sprites
    FUN_00529bd0("Interface/Cursor.tga",          2,  0x2601, 0x2900, 0, '\x01');
    FUN_00529bd0("Interface/CursorPush.tga",       3,  0x2601, 0x2900, 0, '\x01');
    FUN_00529bd0("Interface/CursorAttack.tga",     4,  0x2601, 0x2900, 0, '\x01');
    FUN_00529bd0("Interface/CursorGet.tga",        5,  0x2601, 0x2900, 0, '\x01');
    FUN_00529bd0("Interface/CursorTalk.tga",       6,  0x2601, 0x2900, 0, '\x01');
    FUN_00529bd0("Interface/CursorRepair.tga",     7,  0x2601, 0x2900, 0, '\x01');
    FUN_00529bd0("Interface/CursorLeanAgainst.tga",8,  0x2601, 0x2900, 0, '\x01');
    FUN_00529bd0("Interface/CursorSitDown.tga",    9,  0x2601, 0x2900, 0, '\x01');
    FUN_00529bd0("Interface/CursorDontMove.tga",  10,  0x2601, 0x2900, 0, '\x01');
    // Interface / HUD JPGs
    FUN_00529740("Interface/Menu01_new.jpg",  0xe6, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/Menu02.jpg",      0xe7, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/Menu03_new.jpg",  0xe8, 0x2600, 0x2900, 0, '\x01');
    FUN_00529bd0("Interface/Menu04.tga",      0xe9, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/Menu_Red.jpg",    0xea, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/Menu_Blue.jpg",   0xeb, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/Menu_Party.jpg",  0xec, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/Menu_Character.jpg",  0xed, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/Menu_Inventory.jpg",  0xee, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/dialogue2_1.jpg", 0xef, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/Message_box.jpg", 0xf0, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/ok.jpg",          0xf1, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/ok2.jpg",         0xf2, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/cancel.jpg",      0xf3, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/cancel2.jpg",     0xf4, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/Level_box.jpg",   0xf5, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/level01.jpg",     0xf6, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/Menu_Green.jpg",  0xf7, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/dialogue1.jpg",   0xf8, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/dialogue2.jpg",   0xf9, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/Guild.jpg",       0xfa, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/Message_box1.jpg",0xfb, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/Message_box2.jpg",0xfc, 0x2600, 0x2900, 0, '\x01');
    // NIS (new interface system) frames
    FUN_00529bd0("Interface/nis_rsframe.tga", 0x500, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/nis_vframe.jpg",  0x501, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/nis_bar.jpg",     0x502, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/nis_back.jpg",    0x503, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/nis_btnarrow.jpg",0x504, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/nis_btnchat.jpg", 0x505, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/nis_btnsize.jpg", 0x506, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/nis_btnalpha.jpg",0x507, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/bar.jpg",         0xfd, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/back1.jpg",       0xfe, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/back2.jpg",       0xff, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/back3.jpg",      0x100, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/Menu03_new_AG.jpg",0x101,0x2600, 0x2900, 0, '\x01');
    // Effect textures
    FUN_00529740("Effect/Fire01.jpg",     0x4ab, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/Fire02.jpg",     0x4ac, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/Fire03.jpg",     0x4ad, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/PoundingBall.jpg",0x567,0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/fi01.jpg",       0x565, 0x2601, 0x2900, 0, '\x01');
    FUN_00529bd0("Effect/fi02.tga",       0x566, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/Flame01.jpg",    0x4b0, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/flare01.jpg",    0x47e, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/Magic_Ground2.jpg",0x4f0,0x2601,0x2900, 0, '\x01');
    FUN_00529740("Effect/Magic_Circle1.jpg",0x4f1,0x2601,0x2900, 0, '\x01');
    FUN_00529740("Effect/Spark02.jpg",    0x497, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/Spark03.jpg",    0x498, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/smoke01.jpg",    0x4c4, 0x2601, 0x2900, 0, '\x01');
    FUN_00529bd0("Effect/smoke02.tga",    0x4c5, 0x2601, 0x2900, 0, '\x01');
    FUN_00529bd0("Effect/blood01.tga",    0x4b5, 0x2601, 0x2900, 0, '\x01');
    FUN_00529bd0("Effect/blood.tga",      0x4b6, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/Explotion01.jpg",0x4bf, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/DinoE.jpg",      0x4c0, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/Shiny01.jpg",    0x4ce, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/Shiny02.jpg",    0x4cf, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/Shiny03.jpg",    0x4d0, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/eye01.jpg",      0x4d1, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/ring.jpg",       0x4d2, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/Chrome01.jpg",   0x492, 0x2601, 0x2901, 0, '\x01');
    FUN_00529740("Effect/blur01.jpg",     0x48d, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Effect/bab2.jpg",       0x493, 0x2601, 0x2901, 0, '\x01');
    FUN_00529740("Effect/motion_blur.jpg",0x48e, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Effect/motion_blur_r.jpg",0x48f,0x2600,0x2900, 0, '\x01');
    FUN_00529740("Effect/lightning2.jpg", 0x4a7, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/Thunder01.jpg",  0x49c, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/Spark01.jpg",    0x4e9, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/JointThunder01.jpg",0x4e6,0x2601,0x2900,0,'\x01');
    FUN_00529740("Effect/JointSpirit01.jpg", 0x4e5,0x2601,0x2900,0,'\x01');
    FUN_00529740("Effect/JointLaser01.jpg",  0x4ea,0x2601,0x2900,0,'\x01');
    FUN_00529740("Effect/JointEnergy01.jpg", 0x4eb,0x2601,0x2900,0,'\x01');
    FUN_00529740("Effect/JointLaser02.jpg",  0x4ed,0x2601,0x2901,0,'\x01');
    FUN_00529740("Monster/iui03.jpg",     0x4f6, 0x2601, 0x2901, 0, '\x01');
    FUN_00529bd0("Monster/magic_H.tga",   0x4fb, 0x2601, 0x2901, 0, '\x01');
    FUN_00529bd0("Item/lower_14m.tga",    0x4fc, 0x2601, 0x2901, 0, '\x01');
    FUN_00529740("Skill/Skull.jpg",       0x4fa, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Effect/motion_blur_r2.jpg",0x4ee,0x2601,0x2901,0,'\x01');
    FUN_00529740("Effect/Fire04.jpg",     0x4df, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/Flare.jpg",      0x4e1, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/Chrome02.jpg",   0x4f2, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Effect/flareBlue.jpg",  0x4fd, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/NSkill.jpg",     0x4fe, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/Flare02.jpg",    0x4e2, 0x2601, 0x2901, 0, '\x01');
    // Model loaders
    FUN_00506170(); FUN_00507610(); FUN_005079d0();
    FUN_00508d10(); FUN_0050b710(); FUN_0050eb80(); FUN_0050f030();
    // Data/BMD loaders (locale-sensitive)
    if (DAT_0055a7c4 == '\0') {
        crt_sprintf(local_64, "Data2/Local/item_%s.txt", (char*)DAT_0055a7a4);
        FUN_0047b130(local_64);
        FUN_0047b650("Data/Local/Item.bmd");
        crt_sprintf(local_64, "Data2/Local/skill_%s.txt", (char*)DAT_0055a7a4);
        FUN_0047a5b0(local_64);
        FUN_0047a970("Data/Local/Skill.bmd");
        FUN_0047a010("Data2/Gate.txt");
        FUN_0047a170("Data/Gate.bmd");
        crt_sprintf(local_64, "Data2/Local/Filter_%s.txt", (char*)DAT_0055a7a4);
        FUN_004799d0(local_64);
        FUN_00479a50("Data/Local/Filter.bmd");
        crt_sprintf(local_64, "Data2/Local/FilterName_%s.txt", (char*)DAT_0055a7a4);
        FUN_00479cf0(local_64);
        FUN_00479d70("Data/Local/FilterName.bmd");
        crt_sprintf(local_64, "Data2/Local/Dialog_%s.txt", (char*)DAT_0055a7a4);
        FUN_00479cf0(local_64);
        FUN_00479d70("Data/Local/FilterName.bmd");
    } else {
        FUN_0047b740("Data/Local/Item.bmd");
        FUN_0047ac50("Data/Local/Skill.bmd");
        FUN_0047a4d0("Data/Gate.bmd");
        FUN_00479b30("Data/Local/Filter.bmd");
        FUN_00479e50("Data/Local/FilterName.bmd");
        FUN_0047b020("Data/Local/Dialog_Spn.bmd");   // cliente traducido: solo Dialog_{Eng,Por,Spn}.bmd
        FUN_00401040((int)(uintptr_t)DAT_00583d8c, "Data/Local/Quest.bmd");
    }
    crt_sprintf(local_64, "Data/Local/NPCName.txt");   // cliente: archivo sin sufijo de locale
    FUN_0047d120(local_64);
    // Sound preloads (tail of OpenBasicData @ 0x00510F65-0x00510FC5).
    // IDs y filenames verificados contra IDA Hex-Rays 00510320_OpenBasicData.c
    // y los binary patches del companion project (Mu-linux-97K/Source/Client/
    // Main/Sound.cpp:162-166). Ghidra había inventado los nombres
    // iPickUp/iGet/iDrop/iEquip — no existen como archivos .wav en Data/Sound.
    FUN_00404a10(4,    "Data\\Sound\\iTitle.wav",        1, '\0');
    FUN_00404a10(0x1b, "Data\\Sound\\iButtonMove.wav",   2, '\0');  // 27
    FUN_00404a10(0x19, "Data\\Sound\\iButtonClick.wav",  1, '\0');  // 25
    FUN_00404a10(0x1a, "Data\\Sound\\iButtonError.wav",  1, '\0');  // 26
    FUN_00404a10(0x1c, "Data\\Sound\\iCreateWindow.wav", 1, '\0');  // 28
    FUN_00404a10(0x25, "Data\\Sound\\iRepair.wav",       1, '\0');  // 37
    FUN_00404a10(0x26, "Data\\Sound\\iWhisper.wav",      1, '\0');  // 38
}
// FUN_00511060 — implemented in src/Monster/Monster_Data.cpp
// FUN_00511140 — implemented in src/Render/GL_State.cpp
// FUN_005111d0 — implemented in src/Render/GL_State.cpp
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

// FUN_00513c10 @ 0x00513C10 — CharSelect_SendSelectPacket
// Builds a C1 XOR-encrypted packet: [C1][len][F3][01][charID...][padding][pin10bytes]
// charID = entity +0x1C1 from CharactersClient[DAT_005615e0], stride 0x394.
// InputText[0] is at DAT_07db8710 (10 bytes of PIN data).
// XOR key: {0xe7,0x6d,0x3a,0x89,0xbc,0xb2,0x9f,0x73,0x23,0xa8,0xfe,0xb6,
//           0x49,0x5d,0x39,0x5d,0x8a,0xcb,0x63,0x8d,0xea,0x7d,0x2b,0x5f,
//           0xc3,0xb1,0xe9,0x83,0x29,0x51,0xe8,0x56}
// After send: login sub-state → 0x18, secondary → 0x15, PlayBuffer(0x1b), ClearInput(1).
void __cdecl FUN_00513c10(void)
{
    static const unsigned char xorKey[32] = {
        0xe7,0x6d,0x3a,0x89,0xbc,0xb2,0x9f,0x73,
        0x23,0xa8,0xfe,0xb6,0x49,0x5d,0x39,0x5d,
        0x8a,0xcb,0x63,0x8d,0xea,0x7d,0x2b,0x5f,
        0xc3,0xb1,0xe9,0x83,0x29,0x51,0xe8,0x56
    };

    DAT_005615e0 = DAT_005616ac; // SelectedHero
    DAT_005616ac = (DWORD)-1;
    DAT_083a7c14 = 0x38;  // CurrentProtocolState

    // Build packet: [C1][len][F3][01] + payload
    unsigned char pkt[0x400];
    memset(pkt, 0, sizeof(pkt));
    pkt[0] = 0xC1;        // packet type
    pkt[1] = 0x00;        // length (filled later)
    pkt[2] = 0xF3;        // opcode
    pkt[3] = 0x01;        // sub-opcode: char select

    unsigned short pos = 4;

    // XOR first segment (bytes 3..3, i.e. just 1 byte — the loop runs for uVar4=3 to 3)
    for (unsigned int i = 3; i < 4; i++) {
        unsigned int ki = i & 0x1F;
        pkt[i] ^= xorKey[ki] ^ pkt[i];
        // Note: the decompile XORs buf[i] with key[ki] XOR buf[i], which zeroes the byte.
        // This is the anti-tamper XOR pattern; actual payload starts next.
    }

    // Append charID string from CharactersClient[DAT_005615e0].ID
    // Entity stride = 0x394, ID at entity + 0x1C1
    char* entityBase = (char*)(uintptr_t)DAT_07abf5d0;
    char* charID = entityBase + (int)DAT_005615e0 * 0x394 + 0x1C1;
    unsigned int idLen = (unsigned int)strlen(charID);

    if (pos + idLen < 0x401) {
        memcpy(pkt + pos, charID, idLen);
        // XOR-encrypt the charID portion
        for (unsigned int i = pos; i < pos + idLen; i++) {
            unsigned int ki = i & 0x1F;
            pkt[i] ^= xorKey[ki] ^ pkt[i]; // double-XOR (anti-tamper pattern)
        }
        pos += (unsigned short)idLen;
    }

    // Append zero padding: (10 - idLen) bytes
    unsigned int padLen = 10 - idLen;
    if (padLen > 0 && pos + padLen < 0x401) {
        memset(pkt + pos, 0, padLen);
        for (unsigned int i = pos; i < pos + padLen; i++) {
            unsigned int ki = i & 0x1F;
            pkt[i] ^= xorKey[ki] ^ pkt[i];
        }
        pos += (unsigned short)padLen;
    }

    // Append 10 bytes of InputText[0] (PIN/second password from DAT_07db8710)
    if (pos + 10 < 0x401) {
        memcpy(pkt + pos, DAT_07db8710, 10);
        for (unsigned int i = pos; i < pos + 10; i++) {
            unsigned int ki = i & 0x1F;
            pkt[i] ^= xorKey[ki] ^ pkt[i];
        }
        pos += 10;
    }

    // Set packet length
    pkt[1] = (unsigned char)pos;

    // Send via socket
    SOCKET sock = DAT_055ca168;
    if (sock != INVALID_SOCKET) {
        int totalSent = 0;
        int remaining = (int)pos;
        while (remaining > 0) {
            int sent = send(sock, (const char*)(pkt + totalSent), remaining, 0);
            if (sent == SOCKET_ERROR) {
                int err = WSAGetLastError();
                if (err == WSAEWOULDBLOCK) {
                    if ((int)(DAT_055cc16c + pos) < 0x2001) {
                        memcpy((char*)DAT_055ca16c + DAT_055cc16c, pkt, pos);
                        DAT_055cc16c += pos;
                    } else {
                        FUN_0043dc90(((int)(uintptr_t)DAT_055ca160));
                    }
                } else {
                    FUN_0043dc90(((int)(uintptr_t)DAT_055ca160));
                }
                break;
            }
            if (sent == 0) break;
            if (DAT_055ce174 != 0) {
                FUN_0043de60();
            }
            remaining -= sent;
            totalSent += sent;
        }
    }

    // Post-send state transitions
    DAT_083a7c14 = 0x18;
    DAT_083a7c18 = 0x15;
    PlayBuffer(0x1b, 0, 0);
    FUN_0047ec60(1);              // ClearInput(1)
    DAT_00559c84 = 0;             // InputEnable = false
}
// FUN_0051af50 — implemented in src/UI/UI_StatsPanel.cpp (UI_StatsPanel_Render)
// FUN_0051e0c0 — implemented in src/Render/Scene_CharPreview.cpp
// FUN_0051e7e0 — implemented in src/Scene/Scene_ServerSelect_Input.cpp (server select hit-test, 337 lines)
// FUN_005239a0 — implemented in src/Render/Scene_CharPreview.cpp
// FUN_0052a050 — implemented in src/Render/Texture.cpp
// FUN_0053d5c0 — implemented in src/Render/Texture.cpp
// FUN_0053ed00 @ 0x0053ed00 — Pipe_SendQuery
// If manager's first byte is non-zero (pipe open), sends opcode 0x616.
uint __cdecl FUN_0053ed00(void* mgr, char* name)
{
    if (mgr == NULL || *(char *)mgr == '\0') return 0;
    FUN_005404a0((LPCVOID)0x8b1, 0x616, name);
    return 1;
}
// FUN_0053de60 @ 0x0053DE60 — jump stub → FUN_0053d890 __thiscall (GameGuard init2, 773 lines).
// This is a trampoline to FUN_0053d890 which handles GameGuard process injection via
// CreateProcess/LoadLibrary. Too complex to implement without SEH and platform env.
// NOP — GameGuard not needed for development builds.
void __cdecl FUN_0053de60(void)                             {} // NOP — GameGuard trampoline

