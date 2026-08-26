// Render_LegacyTrails.cpp
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


// Entity_TeleportAnim @ 0x004792C0 — allocate a teleport-anim slot in DAT_07c80110 pool.
// Pool stride 0x70; slot layout: [0]=active, [4]=id, [0xc]=param4, [0x10..0x18]=src_pos+height,
// [0x1c..0x24]=dst_pos, [0x38..0x3b]=zeros, [0x48..0x4b]="\0\0 A".
// This overload takes dst as individual floats (a,b,c); FUN_0046c3e0 renders the trail.
void __cdecl Entity_TeleportAnim(float *pos, float a, float b, float c)
{
    return;  // AUTO-SKIP: dead duplicate (kept for reference). Active impl is the
             // 4-arg overload in stubs.cpp:3324 (Entity_TeleportAnim with float* dst_pos).
    char *slot = &DAT_07c80110[0];
    while ((int)slot < 0x7c82cd0) {
        if (*slot == '\0') {
            *slot = '\x01';
            *(float *)(slot + 0x10) = pos[0];
            *(float *)(slot + 0x14) = pos[1];
            *(float *)(slot + 0x18) = pos[2] + _DAT_00552958;
            *(float *)(slot + 0x1c) = a;
            *(float *)(slot + 0x20) = b;
            *(float *)(slot + 0x24) = c;
            slot[0x38] = '\0'; slot[0x39] = '\0'; slot[0x3a] = '\0'; slot[0x3b] = '\0';
            slot[0x48] = '\0'; slot[0x49] = '\0'; slot[0x4a] = ' ';  slot[0x4b] = 'A';
            return;
        }
        slot += 0x70;
    }
}

// Particle_Update @ 0x0046C3E0 — Trail_RenderAll: render weapon/beam trails in pool.
// Pool: g_RenderPool_07c608a8 (= shared joint/trail pool, 100 slots × 0x2f0).
// 2026-05-03: AUTO-SKIP removed. Pool now properly sized; iteration count
// is 100 (matching IDA bound `< 0x7c72e74` = base + 100*0x2f0).
void __cdecl Particle_Update(void)
{
    int *slot = (int *)&DAT_07c608b4;
    for (int slotIdx = 0; slotIdx < 100; ++slotIdx, slot = (int*)((int)slot + 0x2f0)) {
        if ((char)slot[-3] != '\0') {
            int mode = slot[-2];
            if (*(short *)(*slot + 0x1be) == 0 && mode < 3)
                FUN_00511710();
            else
                FUN_00511790();
            if (mode > 2) mode -= 3;
            int seg = slot[1];
            if (seg > 1) {
                FUN_00511480(mode + 0x48d);
                int *vp = slot + 0x5f;
                for (int i = 0; i < seg - 1; i++, vp += 3) {
                    glBegin(6);
                    float alpha = _DAT_0055256c;
                    if (*(short *)(*slot + 0x1be) == 0)
                        alpha = (float)(seg - i) / (float)seg;
                    glColor3f(alpha*(float)slot[2], alpha*(float)slot[3], alpha*(float)slot[4]);
                    glTexCoord2f((float)i / (float)seg, 1.0f);
                    glVertex3fv((float *)(vp - 0x5a));
                    glTexCoord2f((float)i / (float)seg, 0.0f);
                    glVertex3fv((float *)vp);
                    float alpha2 = _DAT_0055256c;
                    if (*(short *)(*slot + 0x1be) == 0)
                        alpha2 = (float)(seg - i - 1) / (float)seg;
                    glColor3f(alpha2*(float)slot[2], alpha2*(float)slot[3], alpha2*(float)slot[4]);
                    glTexCoord2f((float)(i+1) / (float)seg, 0.0f);
                    glVertex3fv((float *)(vp + 3));
                    glTexCoord2f((float)(i+1) / (float)seg, 1.0f);
                    glVertex3fv((float *)(vp - 0x57));
                    glEnd();
                }
            }
        }
    }  // end of explicit count loop (advance happens in for-statement)
}
