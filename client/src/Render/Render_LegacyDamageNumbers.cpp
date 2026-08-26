// Extracted from stubs_misc2.cpp; IDA provenance comments are retained.
//
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 2578-4345 (1768 lines).
//
// Mixed sections:
//   "FUN_ stubs (non-void returning)" — non-void function stubs
//   "Screen coordinate converters"    — Screen_ToGLx / Screen_ToGLy
//   "AttackEffect / UseSkillWarrior"  — combat helpers
//   "Entity action stubs"             — Skills.cpp / Combat.cpp externs
//   "Missing stubs added for linker fix" — GL helpers, screen converters
//   "Item data helper stubs"
//   "OpenTexture (Model_LoadTextures)"

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "Net/Net.h"

extern "C" void DbgLogPublic(const char* msg);
extern void __cdecl FUN_0054158c(void* ptr);
extern void Net_SendSmallPacket(const BYTE* pkt, int totalLen);

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


// FUN_00479330 @ 0x00479330 — RenderPoints (damage popup renderer)
// 2026-05-06: ported from IDA mu97k-src-IDA/raw/00479330_RenderPoints.c.
//
// Itera el pool DAT_07c80110[100 × 0x70] de damage popups (poblado por
// CreatePoint en Net_Process case 0x15 / ReceiveAttackDamage). Para cada
// slot activo proyecta su world position a screen via gluProject y
// renderiza el número con RenderNumber2D usando el color del slot.
//
// Slot layout (per IDA CreatePoint):
//   +0x00 byte  active (1 if displayed)
//   +0x04 int   Value (damage to display; -1 = MISS)
//   +0x0c float scale (text size — typically 15 normal, 50 special)
//   +0x10 float pos.x (world)
//   +0x14 float pos.y (world)
//   +0x18 float pos.z (world, +140 elevation pre-applied)
//   +0x1c float color.r
//   +0x20 float color.g
//   +0x24 float color.b
//   +0x38 float frame counter (init 0, MovePoints increments)
//   +0x48 float lifetime (init 10.0, MovePoints decrements 0.3/tick)
extern "C" double __cdecl RenderNumber2D(float x, float y, int Num,
                                          float Width, float Height);

extern void __cdecl FUN_005113f0(float*, int*, int*);   // World_ToScreen
extern "C" // ─────────────────────────────────────────────────────────────────────────────
// RenderNumber  @ 0x005120C0  — dibuja un número de daño en el MUNDO.
// RenderPoints  @ 0x00479330  — recorre el pool y llama al anterior.
//
// 2026-08-15 — REESCRITO FIEL A IDA. La versión anterior era una invención en
// dos fases (project con `gluProject` + draw con `RenderNumber2D` en ortho 2D).
// Tres síntomas venían de ahí:
//   · nada se veía — se proyectaba con la matriz MODELVIEW leída de GL, que en
//     ese punto del frame es la IDENTIDAD (el call site corre después de
//     `FUN_00511cf0`/BeginSprite);
//   · los dígitos salían invertidos — `RenderNumber2D` usa V de 0.0→0.5 y el
//     original usa 0.5→0.0;
//   · los MISS salían como barras blancas — el original tiene un sprite propio
//     para `Num == -1`, no dibuja dígitos.
//
// El binario no proyecta a mano: `RenderNumber` emite el número como sprites 3D
// con `RenderSpriteUV` (0x511FB0), que ya transforma por la CameraMatrix. Por
// eso el call site está dentro del bloque 3D, entre BeginSprite y glPopMatrix.
// ─────────────────────────────────────────────────────────────────────────────
// (RenderSpriteUV_stub y FUN_005114f0 ya están declarados en functions.h)

extern "C" void __cdecl RenderNumber(float Position[3], int Num,
                                     float Color[3], float Alpha, float Scale);
void __cdecl RenderNumber(float Position[3], int Num, float Color[3],
                          float Alpha, float Scale)
{
    float p[3]   = { Position[0], Position[1], Position[2] };
    float UV[4][2];
    float Light[3][4];

    // Patrón cíclico de los 3 componentes, tal cual IDA L26-42.
    Light[0][0] = Color[0]; Light[0][1] = Color[1];
    Light[0][2] = Color[2]; Light[0][3] = Color[0];
    Light[1][0] = Color[1]; Light[1][1] = Color[2];
    Light[1][2] = Color[0]; Light[1][3] = Color[1];
    Light[2][0] = Color[2]; Light[2][1] = Color[0];
    Light[2][2] = Color[1]; Light[2][3] = Color[2];

    if (Num == -1) {
        // MISS: sprite propio, región (0..0.125, 0.53125..1.0), tamaño 45x20.
        // Constantes leídas de los QWORD del decompile.
        UV[0][0] = 0.0f;   UV[0][1] = 1.0f;
        UV[1][0] = 0.125f; UV[1][1] = 1.0f;
        UV[2][0] = 0.125f; UV[2][1] = 0.53125f;
        UV[3][0] = 0.0f;   UV[3][1] = 0.53125f;
        RenderSpriteUV_stub(1, p, 45.0f, 20.0f, UV, Light, Alpha);
        return;
    }

    char Text[32];
    _itoa(Num, Text, 10);
    const int len = (int)strlen(Text);
    if (len <= 0) return;

    const float v14 = (float)len * Scale * 0.125f;
    p[0] = p[0] - (float)len * 5.0f - v14;
    p[1] = p[1] - v14;

    const float step = Scale * 0.5f;
    for (int i = 0; i < len; ++i) {
        const float u = (float)(Text[i] - '0') * 0.0625f;
        // V va de 0.5 (abajo) a 0.0 (arriba) — invertida respecto de la
        // intuición; así lo hace el binario y es lo que deja los dígitos
        // derechos.
        UV[0][0] = u;            UV[0][1] = 0.5f;
        UV[1][0] = u + 0.0625f;  UV[1][1] = 0.5f;
        UV[2][0] = u + 0.0625f;  UV[2][1] = 0.0f;
        UV[3][0] = u;            UV[3][1] = 0.0f;
        RenderSpriteUV_stub(1, p, Scale, Scale, UV, Light, Alpha);
        p[0] += step;
        p[1] += step;
    }
}

// RenderPoints — port FIEL de IDA 0x479330. Los 4 args son un artefacto del
// call site anterior; se ignoran.
void __cdecl FUN_00479330(int, int, int, int)
{
    EnableAlphaTest(true);
    FUN_005114f0();                    // DisableDepthTest

    char* base = (char*)&DAT_07c80110[0];
    for (int i = 0; i < 100; ++i) {
        char* slot = base + i * 0x70;
        if (slot[0] == 0) continue;

        // IDA: alpha = lifetime(+0x48) * 0.4
        const float alpha = *(float*)(slot + 0x48) * 0.40000001f;
        RenderNumber((float*)(slot + 0x10),      // world pos
                     *(int*)(slot + 0x04),       // value (-1 = MISS)
                     (float*)(slot + 0x1c),      // color
                     alpha,
                     *(float*)(slot + 0x0c));    // scale
    }
}


// ── Missing stubs added for linker fix ───────────────────────────────────────

// GL helpers — cached OpenGL state wrappers
