// Entity_DrawByType.cpp — Draw_RenderObject @ 0x004fae00
// Dispatches entity render based on entity type (*(short*)(param_1+2)).
//
// param_1 = entity data pointer (stride 0x394, entity array at DAT_07abf5d0)
// param_2 = second entity pointer (unused in most cases)
// param_3 = extra param (used in type 0x14a HashTable block)
// param_4 = flag byte (player-focused override for some types)
//
// Render model context:
//   this = (void*)(DAT_05828d58 + entity_type * 0xbc)
//   this+0x48/4c/50 = tint RGB floats
//   this+0x88       = render layer mask
//
// Entity fields used:
//   param_1+0x02  = entity_type (short)
//   param_1+0x04  = unk_04 (int)
//   param_1+0x58  = facing/anim_id (float/int) — -2 = invisible guard
//   param_1+0x68  = world_x
//   param_1+0x6c  = world_y
//   param_1+0x70  = world_z
//   param_1+0x74  = render_mode byte
//   param_1+0x78  = entity_flags (uint) — bit0=player flag, bit1=PvP
//   param_1+0x7c  = unk_7c byte
//   param_1+0x84  = alive state
//   param_1+0x8b  = flash counter (10=full red, >0=fading)
//   param_1+0x105 = anim_state (byte) — 6=death trigger
//   param_1+0x168 = scale float
//   param_1+0x17c = base level (int)
//   param_1+100   = height_int
//
// ── Anti-tamper note ──────────────────────────────────────────────────────────
// Type 0x14a contains an ~300-line HashTable encode/decode block operating on
// DAT_083a7c00 (a local slot key). Per CLAUDE.md policy this is obfuscation,
// not game logic. The post-hash render logic IS implemented.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "structs.h"
#include <windows.h>
#include <GL/gl.h>
#include <math.h>

extern "C" void DbgLogPublic(const char*);   // [DIAG TEMP #4]
extern "C" void DbgForge(const char* fn, int type, int model, int bmp, int glTex,
                         int mesh, int blend, float wx, float wy, float wz,
                         float r, float g, float b, float a);   // [DIAG FORGE]

// fsin is an x87 FPU intrinsic declared in math.h — no separate extern needed

// ─────────────────────────────────────────────────────────────────────────────

// ─────────────────────────────────────────────────────────────────────────────
// Silueta de Molt — clase de vtable off_552588 (IDA sub_40A660).
//
// Vtable: [0] 0x40A6C0 dtor, [1] 0x40A6F0 armar, [2] 0x40A830 liberar.  Layout
// (0x20 bytes): +0 vtable, +4 short, +8 buffer, +12..+20 direccion de la luz,
// +24 cantidad de aristas, +28 aristas (10 bytes c/u: v0, v1, malla, uv0, uv1).
//
// Desviacion: el binario hace `operator_new(0x20)` en CADA frame y nunca libera
// el objeto (el metodo [2] solo libera sus buffers), y llama a los metodos por
// la vtable.  Aca el objeto vive en el stack y los metodos se llaman directo:
// mismo resultado, sin la fuga.  2026-09-18: antes el port llamaba por una
// vtable que nunca se instalaba (WidgetB_CtorFull la saltea) y sin `this`.
// ─────────────────────────────────────────────────────────────────────────────
struct MoltSilhouette {
    void          *vtable;
    short          f4;
    void          *buf8;
    float          dir[3];     // +12
    int            count;      // +24
    unsigned char *edges;      // +28
};

extern void __cdecl FUN_0040a8f0(void *obj, float *p1, float *p2);   // sub_40A8F0

// IDA sub_40A110 (0x40A110): agrega la arista `edge` del triangulo `triIdx` si
// es de borde (sin vecino, o el vecino no mira a la luz).
static void MoltSilhouette_AddEdge(MoltSilhouette *s, short a, short b, short mesh,
                                   int triIdx, int edge, unsigned char *tris)
{
    unsigned char *tri = tris + 36 * triIdx;
    const short adj = *(short *)(tri + 2 * edge + 26);
    if (adj != -1 && tris[36 * adj + 34] != 0) return;
    unsigned char *out = s->edges + 10 * s->count;
    *(short *)(out + 0) = a;
    *(short *)(out + 2) = b;
    *(short *)(out + 4) = mesh;
    *(short *)(out + 6) = *(short *)(tri + 2 * edge + 10);
    *(short *)(out + 8) = *(short *)(tri + 2 * ((edge + 1) % 3) + 10);
    s->count++;
}

// IDA sub_40A1C0 (0x40A1C0): marca cada triangulo segun mire o no a la luz
// (byte +34) y junta las aristas de silueta de los que miran.
static void MoltSilhouette_BuildMesh(MoltSilhouette *s, short mesh, unsigned char *verts,
                                     short numTri, unsigned char *tris)
{
    const int base = 15000 * mesh;
    for (int i = 0; i < numTri; ++i) {
        unsigned char *tri = tris + 36 * i;
        float normal[3] = { 0.0f, 0.0f, 0.0f };
        FaceNormalize((float *)(verts + 12 * (base + *(short *)(tri + 2))),
                      (float *)(verts + 12 * (base + *(short *)(tri + 4))),
                      (float *)(verts + 12 * (base + *(short *)(tri + 6))), normal);
        tri[34] = (normal[2] * s->dir[2] + normal[1] * s->dir[1] + normal[0] * s->dir[0] <= 0.0f);
    }
    for (int i = 0; i < numTri; ++i) {
        unsigned char *tri = tris + 36 * i;
        if (!tri[34]) continue;
        MoltSilhouette_AddEdge(s, *(short *)(tri + 2), *(short *)(tri + 4), mesh, i, 0, tris);
        MoltSilhouette_AddEdge(s, *(short *)(tri + 4), *(short *)(tri + 6), mesh, i, 1, tris);
        MoltSilhouette_AddEdge(s, *(short *)(tri + 6), *(short *)(tri + 2), mesh, i, 2, tris);
    }
}

// IDA sub_40A6F0 (0x40A6F0), con a5 = 1.  La "luz" es la direccion camara ->
// heroe; solo se procesa la malla 1.
static void MoltSilhouette_Build(MoltSilhouette *s, unsigned char *verts,
                                 unsigned char *model, unsigned char *o)
{
    unsigned char *hero = (unsigned char *)DAT_07abf5d8;
    s->dir[0] = *(float *)(hero + 16) - _DAT_083a42d4;   // CameraPosition
    s->dir[1] = *(float *)(hero + 20) - _DAT_083a42d8;
    s->dir[2] = *(float *)(hero + 24) - _DAT_083a42dc;
    Vec3_Normalize(s->dir);
    if (!(*(float *)(o + 360) >= 0.0099999998f)) return;
    const short hidden = *(short *)(o + 88);
    const short blend  = *(short *)(o + 100);
    if (hidden == -2 || blend == -2) return;

    unsigned char *meshes = *(unsigned char **)(model + 40);
    const short texIdx = *(short *)(*(unsigned char **)(model + 56) + 2);
    const bool  alphaTex = (Bitmaps[texIdx].Components == 4);
    int numTri = 0;
    if (hidden != 1 && blend != 1 && !alphaTex)
        numTri = *(short *)(meshes + 50);
    s->count = 0;
    s->edges = (unsigned char *)operator new(30 * numTri);
    if (hidden != 1 && blend != 1 && !alphaTex)
        MoltSilhouette_BuildMesh(s, 1, verts, *(short *)(meshes + 50),
                                 *(unsigned char **)(meshes + 68));
}

static void MoltSilhouette_Render(unsigned char *model, unsigned char *o)
{
    MoltSilhouette s = {};
    unsigned char *verts = (unsigned char *)&DAT_0584621c;   // v2 = 0x0584621C
    MoltSilhouette_Build(&s, verts, model, o);
    // IDA sub_40A860 (0x40A860): un quad por arista.  El 4o/5o argumento
    // (el buffer 0x060DB65C) no lo usa sub_40A8F0.
    for (int i = 0; i < s.count; ++i) {
        const short *e = (const short *)(s.edges + 10 * i);
        const int base = 15000 * e[2];
        FUN_0040a8f0(&s, (float *)(verts + 12 * (base + e[0])),
                         (float *)(verts + 12 * (base + e[1])));
    }
    // IDA sub_40A830 (0x40A830).
    operator delete(s.edges);
    if (s.buf8) operator delete(s.buf8);
}

void __cdecl Draw_RenderObject(void *param_1_v, int param_2, int param_3, char param_4)
{
    unsigned char *param_1 = (unsigned char *)param_1_v;
    // Model context for this entity type
    void *model = (void *)(DAT_05828d58 + *(short *)(param_1 + 2) * 0xbc);


    // Invisible guard — IDA Draw_RenderObject L82:
    //     if ( !EditFlag && *(_DWORD *)(o + 88) == -2 ) return;
    // `+0x58` (88) es HiddenMesh; -2 = objeto entero invisible (marcadores como
    // PoseBox01 / Light01-03). `DAT_07e11d30` es EditFlag y vale 0 fuera del
    // editor, así que este return SÍ dispara.
    if (DAT_07e11d30 == 0 && *(int *)(param_1 + 0x58) == -2)
        return;

    // ── Tint overrides based on game sub-state / type ──────────────────────────

    // Sub-state 10 (dense rain) + type 0x12d → blue-tinted tint
    if (World == 10 && *(short *)(param_1 + 2) == 0x12d) {
        *(float *)((int)model + 0x48) = 0.02f;          // 0x3ca3d70a R (el port tenia 0.15)
        *(float *)((int)model + 0x4c) = 0.05f;          // 0x3d4ccccd G
        *(float *)((int)model + 0x50) = 0.15f;          // 0x3e19999a B
        goto LAB_render_dispatch;
    }

    // Sub-state 9 (snow) + type 0x120 → icy tint
    if (World == 9 && *(short *)(param_1 + 2) == 0x120) {
        *(float *)((int)model + 0x48) = 0.0f;
        *(float *)((int)model + 0x4c) = 0.3f;           // 0x3e99999a G
        *(float *)((int)model + 0x50) = 1.0f;            // 0x3f800000 B
    }

LAB_render_dispatch:
    // param_4 + type 0x129 → pure-blue override
    if (param_4 != '\0' && *(short *)(param_1 + 2) == 0x129) {
        *(float *)((int)model + 0x48) = 0.0f;
        *(float *)((int)model + 0x4c) = 0.0f;
        *(float *)((int)model + 0x50) = 1.0f;
    }

    // Render mode 0x80: special full render + return
    if (param_1[0x74] == (unsigned char)0x80) {
        BMD__RenderBody(model, 0x82,
                     *(float *)(param_1 + 0x168), *(int *)(param_1 + 100),
                     *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                     *(float *)(param_1 + 0x70),  *(int *)(param_1 + 0x58),
                     0xffffffff);
        return;
    }

    short sType = *(short *)(param_1 + 2);

    // ── Type dispatch ──────────────────────────────────────────────────────────

    if (sType == 0x128) {
        // Double-pass: dim orange first, then normal
        *(float *)((int)model + 0x48) = 0.4f;    // 0x3ecccccd R
        *(float *)((int)model + 0x4c) = 0.6f;    // 0x3f19999a G (el port tenia 0.575)
        *(float *)((int)model + 0x50) = 1.0f;
        *(unsigned char *)((int)model + 0x88) = 0;
        BMD__RenderBody(model, 2,
                     *(float *)(param_1 + 0x168), *(int *)(param_1 + 100),
                     *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                     *(float *)(param_1 + 0x70),  *(int *)(param_1 + 0x58),
                     0x492);
        *(float *)((int)model + 0x48) = 1.0f;
        *(float *)((int)model + 0x4c) = 1.0f;
        *(float *)((int)model + 0x50) = 1.0f;
        *(unsigned char *)((int)model + 0x88) = 0xff;
        goto LAB_simple_render;
    }

    if (sType == 0x10b)
        goto LAB_simple_render;

    if (World == 0) {
        if (sType == 0x69) {
            // IDA 0x004FAE00: Waterspout01 in Lorencia renders 4 explicit mesh
            // passes with mesh indices 0..3 and object fields as the remaining
            // parameters. Our prior decompile treated small integer args as
            // denormal floats, collapsing mesh selection and blend args.
            BMD__BeginRender();
            BMD__RenderMesh(model, 0.0f, 2,
                         *(float *)(param_1 + 0x168), *(int *)(param_1 + 100),
                         *(float *)(param_1 + 104), 0.0f, 0.0f, 0xffffffff);
            BMD__RenderMesh(model, 1.0f, 2,
                         *(float *)(param_1 + 0x168), *(int *)(param_1 + 100),
                         *(float *)(param_1 + 104), 0.0f, 0.0f, 0xffffffff);
            BMD__RenderMesh(model, 2.0f, 2,
                         *(float *)(param_1 + 0x168), *(int *)(param_1 + 100),
                         *(float *)(param_1 + 104), 0.0f, 0.0f, 0xffffffff);
            BMD__RenderMesh(model, 3.0f, 2,
                         *(float *)(param_1 + 0x168), 3,
                         *(float *)(param_1 + 104), *(float *)(param_1 + 108),
                         *(float *)(param_1 + 0x70),  0xffffffff);
            glPopMatrix();
            return;
        }
    } else if (World == 4) {
        // Char-select scene: specific entity types get extra render passes
        if (sType == 0x17 || sType == 0x13 || sType == 0x14 || sType == 3 || sType == 4) {
            _rand();
            sType = *(short *)(param_1 + 2);
            if (sType == 0x17) {
                // Normal char-select render
                goto LAB_substate4_done;
            }
            if (sType == 0x13 || sType == 0x14) {
                // Save tint, render with warm overlay, restore
                float savedR = *(float *)((int)model + 0x48);
                float savedG = *(float *)((int)model + 0x4c);
                float savedB = *(float *)((int)model + 0x50);
                *(float *)((int)model + 0x48) = 1.0f;
                *(float *)((int)model + 0x4c) = 0.2f;   // 0x3e4ccccd
                *(float *)((int)model + 0x50) = 0.1f;   // 0x3dcccccd
                *(unsigned char *)((int)model + 0x88) = 2;
                BMD__RenderBody(model, 2,
                             *(float *)(param_1 + 0x168), *(int *)(param_1 + 100),
                             *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                             *(float *)(param_1 + 0x70),  *(int *)(param_1 + 0x58),
                             0x492);
                *(float *)((int)model + 0x48) = savedR;
                *(float *)((int)model + 0x4c) = savedG;
                *(float *)((int)model + 0x50) = savedB;
                *(unsigned char *)((int)model + 0x88) = 0xff;
            } else {
                if (sType != 3 && sType != 4) return;
                // Decoration objects: same warm overlay
                float savedR = *(float *)((int)model + 0x48);
                float savedG = *(float *)((int)model + 0x4c);
                float savedB = *(float *)((int)model + 0x50);
                *(float *)((int)model + 0x48) = 1.0f;
                *(float *)((int)model + 0x4c) = 0.2f;
                *(float *)((int)model + 0x50) = 0.1f;
                *(unsigned char *)((int)model + 0x88) = 1;
                BMD__RenderBody(model, 2,
                             *(float *)(param_1 + 0x168), *(int *)(param_1 + 100),
                             *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                             *(float *)(param_1 + 0x70),  *(int *)(param_1 + 0x58),
                             0x492);
                *(float *)((int)model + 0x48) = savedR;
                *(float *)((int)model + 0x4c) = savedG;
                *(float *)((int)model + 0x50) = savedB;
                *(unsigned char *)((int)model + 0x88) = 0xff;
            }
LAB_substate4_done:
            BMD__RenderBody(model, 2,
                         *(float *)(param_1 + 0x168), *(int *)(param_1 + 100),
                         *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                         *(float *)(param_1 + 0x70),  *(int *)(param_1 + 0x58),
                         0xffffffff);
            return;
        }
    } else if (World == 8) {
        if (sType == 0x51) {
            // Map transition portal
            BMD__BeginRender();
            BMD__RenderMesh(model, 0.0f,  2,
                         *(float *)(param_1 + 0x168), *(int *)(param_1 + 100),
                         *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                         *(float *)(param_1 + 0x70),  0xffffffff);
            BMD__RenderMesh(model, 0.0f,  0x44,
                         *(float *)(param_1 + 0x168), *(int *)(param_1 + 100),
                         *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                         *(float *)(param_1 + 0x70),  0x492);
            glPopMatrix();
            return;
        }
    } else if (World > 10 && World < 0x11) {
        if (sType == 0x1c || sType == 0x1d) {
            // Map-load transition: render + shadow drop
            BMD__BeginRender();
            BMD__RenderBody(model, 2,
                         *(float *)(param_1 + 0x168), *(int *)(param_1 + 100),
                         *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                         *(float *)(param_1 + 0x70),  *(int *)(param_1 + 0x58),
                         0xffffffff);
            glPopMatrix();
            GL_SetBlendSrcOver('\x01');
            glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
            float wx = *(float *)(param_1 + 0x10);
            float wy = *(float *)(param_1 + 0x14);
            *(float *)((int)model + 0x74) = RequestTerrainHeight(wx, wy);
            *(float *)((int)model + 0x6c) = wx;
            *(float *)((int)model + 0x70) = wy;
            *(int *)(param_1 + 0x58) = 2;
            BMD__RenderBodyShadow(model, *(int *)(param_1 + 100), 2);
            *(int *)(param_1 + 0x58) = -1;
            return;
        }
    }

    // ── Entity flag tint (PvP/party) ───────────────────────────────────────────
    {
        unsigned char flags = (unsigned char)*(unsigned int *)(param_1 + 0x78);
        if (flags & 1) {
            if (flags & 2) {
                // PvP player: yellow-green
                *(float *)((int)model + 0x4c) = 1.0f;
                *(float *)((int)model + 0x48) = 0.3f;   // 0x3e99999a
                *(float *)((int)model + 0x50) = 0.8f;   // 0x3f4ccccd (el port tenia 0.6)
            } else {
                // Party player: yellow
                *(float *)((int)model + 0x4c) = 1.0f;
                *(float *)((int)model + 0x48) = 0.3f;
                *(float *)((int)model + 0x50) = 0.5f;   // 0x3f000000
            }
        } else if (flags & 2) {
            // PvP non-party: blue-orange
            *(float *)((int)model + 0x48) = 0.3f;
            *(float *)((int)model + 0x4c) = 0.5f;
            *(float *)((int)model + 0x50) = 1.0f;
        }
    }

    // ── Flash / hit-flash counter (param_1+0x8b) ─────────────────────────────
    {
        unsigned char flashCtr = param_1[0x8b];
        if (flashCtr == 10) {
            // Full red flash
            *(float *)((int)model + 0x48) = 1.0f;
            *(float *)((int)model + 0x4c) = 0.3f;
            *(float *)((int)model + 0x50) = 0.3f;
            param_1[0x8b] = flashCtr - 1;
        } else if (flashCtr != 0) {
            // Fading flash
            *(float *)((int)model + 0x48) = 1.0f;
            float fade = (float)(10 - (int)flashCtr) * _DAT_00552a10 + _DAT_005528b8;
            *(float *)((int)model + 0x4c) = fade;
            *(float *)((int)model + 0x50) = fade;
            param_1[0x8b] = flashCtr - 1;
        }
    }

    // ── Type-specific render dispatch ─────────────────────────────────────────
    sType = *(short *)(param_1 + 2);

    // Scale fixup for types 0x131/0x11d/0x140/0x141 at specific scale+x
    if ((sType == 0x131 || sType == 0x11d || sType == 0x140 || sType == 0x141) &&
        *(int *)(param_1 + 0x168) == 0x3f800000 &&
        *(int *)(param_1 + 0x68)  == 0x3d4ccccd) {
        *(float *)(param_1 + 0x68) = 1.0f;
    }

    if (sType == 0x138 && *(int *)(param_1 + 4) == 1) {
        // Wave-bobbing object (water lily etc.)
        BMD__BeginRender();
        float wave = (float)(fsin((long double)DAT_05826e08 * (long double)_DAT_005528e0)
                     * (long double)_DAT_005524f8 + (long double)_DAT_0055256c);
        BMD__RenderMesh(model, 0.0f,        0x42,
                     *(float *)(param_1 + 0x168), 0, wave,
                     *(float *)(param_1 + 0x6c),  *(float *)(param_1 + 0x70), 0x56c);
        BMD__RenderMesh(model, 1.4013e-45f, 0x42,
                     *(float *)(param_1 + 0x168), 1, wave,
                     *(float *)(param_1 + 0x6c),  *(float *)(param_1 + 0x70), 0x56b);
        BMD__RenderMesh(model, 2.8026e-45f, 0x42,
                     *(float *)(param_1 + 0x168), 1, wave,
                     *(float *)(param_1 + 0x6c),  *(float *)(param_1 + 0x70), 0xffffffff);
        BMD__RenderMesh(model, 2.8026e-45f, 0x42,
                     *(float *)(param_1 + 0x168), 1, wave,
                     *(float *)(param_1 + 0x6c),  *(float *)(param_1 + 0x70), 0xffffffff);
        glPopMatrix();
        goto LAB_postprocess;
    }

    if (sType == 0x14c) {
        // Double render (main + overlay at scale 1.0)
        BMD__RenderBody(model, 2,
                     *(float *)(param_1 + 0x168), *(int *)(param_1 + 100),
                     *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                     *(float *)(param_1 + 0x70),  *(int *)(param_1 + 0x58),
                     0xffffffff);
        BMD__RenderMesh(model, 2.8026e-45f, 2,
                     *(float *)(param_1 + 0x168), 2, 1.0f,
                     *(float *)(param_1 + 0x6c),  *(float *)(param_1 + 0x70), 0xffffffff);
        goto LAB_postprocess;
    }

    if (sType == 0x13f) {
        // Molt (monstruo 68, modelo 319).  IDA Draw_RenderObject case 319:
        // dibuja la malla 0 y encima las aristas de silueta de la malla 1, con
        // la clase de vtable off_552588 (sub_40A660).  Ver MoltSilhouette_*.
        BMD__BeginRender();
        BMD__RenderMesh(model, 0.0f, 2,
                     *(float *)(param_1 + 0x168), *(int *)(param_1 + 100),
                     *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                     *(float *)(param_1 + 0x70),  0xffffffff);
        glPopMatrix();
        MoltSilhouette_Render((unsigned char *)model, param_1);
        goto LAB_postprocess;
    }

    if (sType == 0x144) {
        if (param_4 != '\0') {
            // Colored dual render
            *(float *)((int)model + 0x48) = 0.1f;   // 0x3dcccccd
            *(float *)((int)model + 0x4c) = 0.1f;
            *(float *)((int)model + 0x50) = 0.1f;
            BMD__RenderBody(model, 2,
                         *(float *)(param_1 + 0x168), *(int *)(param_1 + 100),
                         *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                         *(float *)(param_1 + 0x70),  1, 0xffffffff);
            *(float *)((int)model + 0x48) = 1.0f;
            *(float *)((int)model + 0x4c) = 0.1f;
            *(float *)((int)model + 0x50) = 0.1f;
            BMD__RenderMesh(model, 1.4013e-45f, 2,
                         *(float *)(param_1 + 0x168), *(int *)(param_1 + 100),
                         *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                         *(float *)(param_1 + 0x70),  0xffffffff);
            goto LAB_postprocess;
        }
        goto LAB_standard_render;
    }

    if (sType == 0x142) {
        // Double-pass with mid tint + optional PvP override
        *(unsigned char *)((int)model + 0x88) = 0;
        *(float *)((int)model + 0x48) = 0.4f;   // 0x3ecccccd
        *(float *)((int)model + 0x4c) = 0.3f;   // 0x3e99999a
        *(float *)((int)model + 0x50) = 0.5f;   // 0x3f000000
        BMD__RenderBody(model, 2,
                     *(float *)(param_1 + 0x168), *(int *)(param_1 + 100),
                     *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                     *(float *)(param_1 + 0x70),  *(int *)(param_1 + 0x58),
                     0x4f6);
        *(float *)((int)model + 0x48) = 0.5f;
        *(float *)((int)model + 0x4c) = 0.5f;
        *(float *)((int)model + 0x50) = 0.5f;
        *(unsigned char *)((int)model + 0x88) = 0xff;
        if (param_4 != '\0') {
            *(float *)((int)model + 0x48) = 0.1f;
            *(float *)((int)model + 0x4c) = 1.0f;
            *(float *)((int)model + 0x50) = 0.8f;  // 0x3f4ccccd (el port tenia 0.6)
        }
        goto LAB_standard_render;
    }

    if (sType == 0x143) {
        // Purple-tint render (chest/relic)
        *(float *)((int)model + 0x48) = 0.9f;   // 0x3f666666
        *(float *)((int)model + 0x4c) = 0.8f;   // 0x3f4ccccd
        *(float *)((int)model + 0x50) = 1.0f;
        BMD__RenderBody(model, 4,
                     *(float *)(param_1 + 0x168), *(int *)(param_1 + 100),
                     *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                     *(float *)(param_1 + 0x70),  *(int *)(param_1 + 0x58),
                     0x493);
        *(float *)((int)model + 0x48) = 1.0f;
        *(float *)((int)model + 0x4c) = 1.0f;
        *(float *)((int)model + 0x50) = 1.0f;
        goto LAB_standard_render;
    }

    if (sType == 0x141) {
        BMD__RenderBody(model, 2,
                     *(float *)(param_1 + 0x168), *(int *)(param_1 + 100),
                     *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                     *(float *)(param_1 + 0x70),  1, 0xffffffff);
        BMD__RenderMesh(model, 1.4013e-45f, 0x402,
                     0.5f, *(int *)(param_1 + 100),
                     *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                     *(float *)(param_1 + 0x70),  *(unsigned int *)(param_1 + 0x58));
        BMD__RenderMesh(model, 1.4013e-45f, 0x402,
                     *(float *)(param_1 + 0x168), *(int *)(param_1 + 100),
                     *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                     *(float *)(param_1 + 0x70),  *(unsigned int *)(param_1 + 0x58));
        goto LAB_postprocess;
    }

    if (sType == 0x146) {
        // White then gray dual render
        *(unsigned char *)((int)model + 0x88) = 0;
        *(float *)((int)model + 0x48) = 1.0f;
        *(float *)((int)model + 0x4c) = 1.0f;
        *(float *)((int)model + 0x50) = 1.0f;
        BMD__RenderBody(model, 0x42,
                     *(float *)(param_1 + 0x168), *(int *)(param_1 + 100),
                     *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                     *(float *)(param_1 + 0x70),  *(int *)(param_1 + 0x58),
                     0x492);
        *(unsigned char *)((int)model + 0x88) = 0xff;
        *(float *)((int)model + 0x48) = 0.6f;    // 0x3f19999a (el port tenia 0.575)
        *(float *)((int)model + 0x4c) = 0.6f;
        *(float *)((int)model + 0x50) = 0.6f;
        goto LAB_standard_render;
    }

    if (sType == 0x10c) {
        // Direct render at facing=0
        BMD__RenderBody(model, 0x402,
                     *(float *)(param_1 + 0x168), *(int *)(param_1 + 100),
                     *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                     *(float *)(param_1 + 0x70),  1, 0xffffffff);
        goto LAB_postprocess;
    }

    if (sType == 0x14b) {
        if (param_1[0x105] == '\x06') {
            // Death animation trigger
            *param_1 = 0;
            PlayBuffer(0x6a, 0, 0);
            BMD__RenderMeshEffect(model, 0, 0x106);
            goto LAB_postprocess;
        }
        goto LAB_standard_render;
    }

    // 2026-09-04: aca habia un segundo bloque para sType 0x14a gateado por
    // `SceneFlag == 2` (login).  Draw_RenderObject (0x4FAE00) NO consulta
    // SceneFlag en ninguna parte -- su switch tiene UN solo `case 330`.  Era
    // una copia del bloque de abajo sin el chequeo de frames, o sea el patron
    // [[bloque-duplicado-dentro-de-una-funcion]].  Removido.

    if (sType == 0x14a) {
        // ── Estatua de Blood Castle (entidad 330) ────────────────────────────
        // IDA Draw_RenderObject (0x4FAE00) case 330:
        //     if ( o->CurrentAction == 6 ) goto LABEL_137;   // MONSTER01_DIE
        //     <bloque de hash-table que descifra MoveSceneFrame>
        //     if ( MoveSceneFrame - o[380] >= 25 ) { <3 capas>; goto LABEL_146; }
        //     if ( o->CurrentAction == 6 ) { LABEL_137: *(BYTE *)o = 0; }
        //     PlayBuffer(106);
        //     sub_441BE0(o->model, 0, 260);
        //
        // O sea la rama de FRAGMENTOS (mesh 260 + sonido 106) se usa para dos
        // cosas: los primeros 25 frames desde el spawn (la estatua se
        // materializa rompiendose) y la MUERTE, que ademas desactiva la entidad
        // (`*(BYTE *)o = 0`) para que corra un unico frame.
        //
        // `o + 380` lo siembra CreateCharacterPointer con el MoveSceneFrame del
        // spawn.
        //
        // 2026-09-04: el port usaba `param_3` en vez de MoveSceneFrame -> la
        // resta daba siempre < 25 y la estatua quedaba rompiendose EN LOOP.
        // 2026-09-05: y con `CurrentAction == 6` caia a LAB_standard_render en
        // vez de a la rama de fragmentos, asi que al matarla no se veia la
        // animacion de romperse.
        const bool bDead = (param_1[0x105] == 6);
        bool bNormalRender = false;

        if (!bDead) {
            const int msf = (int)DAT_083a7c00;              // MoveSceneFrame
            bNormalRender = (msf - *(int *)(param_1 + 0x17c) >= 0x19);
        }

        if (bNormalRender) {
            BMD__RenderBody(model, 2,
                         *(float *)(param_1 + 0x168), *(int *)(param_1 + 100),
                         *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                         *(float *)(param_1 + 0x70),  *(int *)(param_1 + 0x58),
                         0xffffffff);
            *(float *)((int)model + 0x48) = 1.0f;
            *(float *)((int)model + 0x4c) = 1.0f;
            *(float *)((int)model + 0x50) = 1.0f;
            BMD__RenderBody(model, 0x44,
                         *(float *)(param_1 + 0x168), *(int *)(param_1 + 100),
                         *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                         *(float *)(param_1 + 0x70),  *(int *)(param_1 + 0x58),
                         0x492);
            *(float *)((int)model + 0x50) = 1.0f;
            *(float *)((int)model + 0x48) = 0.3f;
            *(float *)((int)model + 0x4c) = 0.3f;
            BMD__RenderBody(model, 0x48,
                         *(float *)(param_1 + 0x168), *(int *)(param_1 + 100),
                         *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                         *(float *)(param_1 + 0x70),  *(int *)(param_1 + 0x58),
                         0x492);
        } else {
            if (bDead) {
                *param_1 = 0;          // desactiva la entidad: corre un solo frame
            }
            PlayBuffer(0x6a, 0, 0);  // PlayBuffer(106)
            BMD__RenderMeshEffect(model, 0, 0x104);
        }
        goto LAB_postprocess;
    }

    if (sType == 0x104 || sType == 0x105) {
        // Multi-layer entity: base + armor + weapon
        BMD__RenderBody(model, 2,
                     *(float *)(param_1 + 0x168), *(int *)(param_1 + 100),
                     *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                     *(float *)(param_1 + 0x70),  *(int *)(param_1 + 0x58),
                     0xffffffff);
        *(float *)((int)model + 0x48) = 1.0f;
        *(float *)((int)model + 0x4c) = 1.0f;
        *(float *)((int)model + 0x50) = 1.0f;
        BMD__RenderBody(model, 0x44,
                     *(float *)(param_1 + 0x168), *(int *)(param_1 + 100),
                     *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                     *(float *)(param_1 + 0x70),  *(int *)(param_1 + 0x58),
                     0x492);
        *(float *)((int)model + 0x50) = 1.0f;
        *(float *)((int)model + 0x48) = 0.3f;
        *(float *)((int)model + 0x4c) = 0.3f;
        BMD__RenderBody(model, 0x48,
                     *(float *)(param_1 + 0x168), *(int *)(param_1 + 100),
                     *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                     *(float *)(param_1 + 0x70),  *(int *)(param_1 + 0x58),
                     0x492);
        goto LAB_postprocess;
    }

LAB_standard_render:
    BMD__RenderBody(model, 2,
                 *(float *)(param_1 + 0x168), *(int *)(param_1 + 100),
                 *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                 *(float *)(param_1 + 0x70),  *(int *)(param_1 + 0x58),
                 0xffffffff);
    goto LAB_postprocess;

LAB_simple_render:
    BMD__RenderBody(model, 2,
                 *(float *)(param_1 + 0x168), *(int *)(param_1 + 100),
                 *(float *)(param_1 + 0x68),  *(float *)(param_1 + 0x6c),
                 *(float *)(param_1 + 0x70),  *(int *)(param_1 + 0x58),
                 0xffffffff);

LAB_postprocess:
    // ── Post-render: PvP outline ───────────────────────────────────────────────
    if (((unsigned char)*(unsigned int *)(param_1 + 0x78) & 2) == 2) {
        BMD__RenderBody(model, 2,
                     *(float *)(param_1 + 0x168), -2, 1.0f,   // IDA L845: BlendMesh -2
                     *(float *)(param_1 + 0x6c),  *(float *)(param_1 + 0x70),
                     *(int *)(param_1 + 0x58),  0xffffffff);
    }

    // ── Type 0x133: wave-bobbing shadow ───────────────────────────────────────
    if (*(short *)(param_1 + 2) == 0x133) {
        BMD__BeginRender();
        float wave2 = (float)(fsin((long double)DAT_05826e08 * (long double)_DAT_005528e0)
                      * (long double)_DAT_005528b8 + (long double)_DAT_00552504);
        int texFrame = *(short *)(*(int *)((int)model + 0x38) + 0xc);   // IDA: (__int16) de la malla 0
        BMD__RenderMesh(model, 0.0f, 2,
                     *(float *)(param_1 + 0x168), 0, wave2,
                     *(float *)(param_1 + 0x6c), *(float *)(param_1 + 0x70),
                     texFrame);
        // Modulo-100 periodic shadow offset (based on frame counter)
        int framemod = (int)((long long)DAT_05826e08 % 100);
        float modFrac = -(float)(framemod * _DAT_005524f8);
        BMD__RenderMesh(model, 4.2039e-45f, 0x44,
                     *(float *)(param_1 + 0x168), 3, wave2,
                     *(float *)(param_1 + 0x6c), modFrac, 0xffffffff);
        glPopMatrix();
    }

    // ── Types 0x178/0x177: extra zero-position render ─────────────────────────
    if (*(short *)(param_1 + 2) == 0x178 || *(short *)(param_1 + 2) == 0x177) {
        BMD__RenderMesh(model, 0.0f, 0x42,
                     *(float *)(param_1 + 0x168), *(int *)(param_1 + 100),
                     *(float *)(param_1 + 0x68),  0.0f, 0.0f, 0xffffffff);
    }
}
