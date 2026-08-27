// Entity_LegacyEffects.cpp
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


// FUN_005030c0 @ 0x005030C0 — Entity_GravityInit(entity_ptr)
// Sets initial gravity velocity components at +0x1c/+0x20/+0x24 and scale +0x0c
// based on entity type (short at +2). Each entity type has hardcoded float offsets.
void __cdecl FUN_005030c0(int param_1) {
    short sVar1 = *(short*)(param_1 + 2);
    *(unsigned int*)(param_1 + 0x1c) = 0;
    *(unsigned int*)(param_1 + 0x20) = 0;
    *(unsigned int*)(param_1 + 0x24) = 0xc2340000; // -45.0f
    if ((399 < sVar1) && (sVar1 < 0x1d0)) {
        *(unsigned int*)(param_1 + 0x1c) = 0x42700000; // 60.0f
        if (sVar1 == 0x1a3)
            *(unsigned int*)(param_1 + 0xc) = 0x3f333333; // 0.7f
        return;
    }
    if (((0x217 < sVar1) && (sVar1 < 0x221)) || sVar1 == 0x222) {
        *(unsigned int*)(param_1 + 0x1c) = 0x42b40000; // 90.0f
        *(unsigned int*)(param_1 + 0x20) = 0;
        return;
    }
    if (sVar1 < 0x1d0) {
        if (!(0x24f < sVar1)) { *(unsigned int*)(param_1 + 0x1c) = 0; return; }
    } else {
        if (sVar1 < 0x250) {
            *(unsigned int*)(param_1 + 0x1c) = 0;
            *(unsigned int*)(param_1 + 0x20) = 0x43870000; // 270.0f
            return;
        }
        if (sVar1 < 0x270) {
            *(unsigned int*)(param_1 + 0x20) = 0x43870000;
            *(unsigned int*)(param_1 + 0x24) = 0x43610000; // 225.0f
            return;
        }
    }
    if ((0x28f < sVar1) && (sVar1 < 0x2f0)) { *(unsigned int*)(param_1 + 0x1c) = 0x43870000; return; }
    // [0x310, 0x32f]: x=270 + z=45
    if (!((sVar1 < 0x310) || (0x32f < sVar1))) { *(unsigned int*)(param_1 + 0x1c) = 0x43870000; goto lbl_z45; }
    if (sVar1 == 0x3b7) { *(unsigned int*)(param_1 + 0x1c) = 0x42b40000; return; }
    if ((sVar1 == 0x3bb) || (sVar1 == 0x3bc)) { *(unsigned int*)(param_1 + 0x1c) = 0x43870000; goto lbl_z45; }
    if (sVar1 == 0x3bd) { *(unsigned int*)(param_1 + 0xc) = 0x3e4ccccd; return; }
    if (sVar1 == 0x3b8) {
        *(unsigned int*)(param_1 + 0x1c) = 0;
        *(unsigned int*)(param_1 + 0x24) = 0x42340000; // 45.0f
        *(unsigned int*)(param_1 + 0xc)  = 0x3f8ccccd;
        return;
    }
    if (sVar1 == 0x367) {
        *(unsigned int*)(param_1 + 0x20) = 0x42340000;
        *(unsigned int*)(param_1 + 0x24) = 0x42340000;
        return;
    }
    if ((sVar1 == 0x368)||(sVar1 == 0x369)||(sVar1 == 0x36a)||(sVar1 == 0x33e)) goto lbl_z45;
    if (sVar1 == 0x361) { *(unsigned int*)(param_1 + 0x1c) = 0x42b40000; return; }
    if (sVar1 == 0x362) { *(unsigned int*)(param_1 + 0x24) = 0x43870000; *(unsigned int*)(param_1 + 0x1c) = 0x43870000; return; }
    if (sVar1 == 0x363) { *(unsigned int*)(param_1 + 0x1c) = 0x43870000; *(unsigned int*)(param_1 + 0x24) = 0x42b40000; return; }
    if (sVar1 == 0x3be) {
        *(unsigned int*)(param_1 + 0x1c) = 0x42e60000; *(unsigned int*)(param_1 + 0x20) = 0x42960000;
        *(unsigned int*)(param_1 + 0x24) = 0x41000000; *(unsigned int*)(param_1 + 0xc)  = 0x3ecccccd;
        return;
    }
    if (sVar1 == 0x365) { *(unsigned int*)(param_1 + 0x1c) = 0x43870000; *(unsigned int*)(param_1 + 0x24) = 0x42b40000; return; }
    if ((sVar1 == 0x3ba) || (sVar1 == 0x364)) goto lbl_z45;
    *(unsigned int*)(param_1 + 0x1c) = 0;
    return;
lbl_z45:
    *(unsigned int*)(param_1 + 0x24) = 0x42340000; // 45.0f
}

// FUN_00503650 @ 0x00503650 — Entity_UpdateSparkleEffect(entity_ptr)
// Every 0x30 ticks spawns two Shiny01 particles (type 0x4ce) at random direction offset
// from entity's facing matrix. Stack layout: {local_48[3], 0, local_34} = random XZ offsets.
void __cdecl Entity_UpdateSparkleEffect(int param_1)
{
    int iVar2 = *(int *)(param_1 + 4);
    *(int *)(param_1 + 4) = iVar2 + 1;
    if (iVar2 % 0x30 == 0) {
        float *pfVar1 = (float *)(param_1 + 0x1c);
        float local_30[12];
        FUN_004f9db0(pfVar1, local_30);

        unsigned int uVar4 = (unsigned int)_rand() & 0x8000001f;
        if ((int)uVar4 < 0) uVar4 = (uVar4 - 1 | 0xffffffe0) + 1;
        float angX = (float)(int)(uVar4 + 0x10);

        uVar4 = (unsigned int)_rand() & 0x8000001f;
        if ((int)uVar4 < 0) uVar4 = (uVar4 - 1 | 0xffffffe0) + 1;
        float angZ = (float)(int)(uVar4 + 0x10);

        // Original stack layout: {angX, 0.0f, angZ} passed as float[3] to Matrix_TransformPoint
        float inVec[3] = { angX, 0.0f, angZ };
        float outPos[3];
        FUN_004fa0b0(inVec, local_30, outPos);
        outPos[0] += *(float *)(param_1 + 0x10);
        outPos[1] += *(float *)(param_1 + 0x14);
        outPos[2] += *(float *)(param_1 + 0x18);

        float size[4] = { 1.0f, 1.0f, 1.0f, angX };
        FUN_00475220(0x4ce, outPos, pfVar1, size, 0, 1.0f, 0);
        FUN_00475220(0x4ce, outPos, pfVar1, size, 1, 1.0f, 0);
    }
}


// FUN_00408940 @ 0x00408940 — Sound_UpdateChannel3D_Tick(channel)
// Updates 3D sound position sin/cos from entity facing angle at param_1[1]+0x24,
// scaled by random key _DAT_00590af0. Calls constraint update and validity check.
// IDA `sub_408940` devuelve `sub_408E30(this) != 0` — el "¿convergió?" que usa
// el bucle de `sub_408900`. El port lo descartaba (void).
int __cdecl FUN_00408940(int *param_1, float dt) {
    double angle = (*(float*)(param_1[1] + 0x24) + *(float*)&_DAT_005524ec) * *(float*)&_DAT_0055253c;
    DAT_00590af4 = (float)(sin(angle)  * *(float*)&_DAT_00590af0);
    DAT_00590af8 = (float)(-cos(angle) * *(float*)&_DAT_00590af0);
    FUN_00408cb0(param_1, dt);
    return FUN_00408e30((DWORD *)param_1) != 0;
}
