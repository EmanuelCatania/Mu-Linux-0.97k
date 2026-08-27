// Render_LegacyRotatedRect.cpp
// Extracted from stubs_externs.cpp; IDA function comments are retained.

#include "stdafx.h"
void __fastcall FUN_0045aaa0_impl(void *_this, char flags);
void __cdecl    FUN_00408680(void *_this, char flags);
#include "globals.h"
#include "functions.h"

// -- Declaraciones de funciones movidas a otros modulos (refactor B3) -------
// FUN_00408cb0 vive ahora en Scene/Scene_CharSelect_Nav.cpp y FUN_00408e30 en
// Net/Crypto.cpp; antes se definian en este archivo.
void __fastcall FUN_00408cb0(int*, float);
int  __cdecl    FUN_00408e30(DWORD *a1);

#include "Net/Net.h"

extern "C" void DbgLogPublic(const char* msg);
extern "C" BYTE OffsetInventoryItems[];
extern void __cdecl FUN_0054158c(void* ptr);
extern void MapFileDecrypt(BYTE* buf, int size);

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


// GL / 2D render
// FUN_005126e0 @ 0x005126E0 — GL_DrawRotatedRect
// Draws a textured, rotated 2D quad using GL_TRIANGLE_FAN.
// x,y = center angles (converted via sin/cos), w,h = half-extents,
// color = z-depth as float bits.
void __cdecl FUN_005126e0(int id, float x, float y, float w, float h, unsigned int color)
{
    float fSinX = (float)Screen_ToGLX(x);
    float fCosX = (float)Screen_ToGLY(y);
    float fSinW = (float)Screen_ToGLX(w);
    float fCosW = (float)Screen_ToGLY(h);
    GL_BindTextureSlot(id);
    float sz = (float)DAT_00561570;
    float depth = *(float*)&color;

    // Build rotation matrix from direction vector pointing at (depth)
    float bvec[3] = { 0.0f, 0.0f, depth };
    float mat[12];
    FUN_004f9db0(bvec, mat);

    // 4 corner UV + positions
    static float uvs[8] = { 0.0f, 1.0f,  1.0f, 1.0f,  1.0f, 0.0f,  0.0f, 0.0f };
    float corners[4][3] = {
        { fSinW * _DAT_00552a14,  fCosW * _DAT_00552504, 0.0f },
        { fSinW * _DAT_00552a14, -fCosW * _DAT_00552504, 0.0f },
        {-fSinW * _DAT_00552a14,  fCosW * _DAT_00552504, 0.0f },
        {-fSinW * _DAT_00552a14, -fCosW * _DAT_00552504, 0.0f }
    };

    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i < 4; i++) {
        float out[3];
        FUN_004fa0b0(corners[i], mat, out);
        glTexCoord2f(uvs[i*2], uvs[i*2+1]);
        glVertex2f(fSinX + out[0], (sz - fCosX) + out[1]);
    }
    glEnd();
}
