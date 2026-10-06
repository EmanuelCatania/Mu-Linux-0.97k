// Math_LegacyAngle.cpp — Math_GetAngleFromPoints.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "Net/Net.h"

extern void __cdecl operator_delete(void* ptr);

#ifndef qmemcpy
#define qmemcpy(dst,src,sz) memcpy((dst),(src),(size_t)(sz))
#endif
#ifndef delete__
#define delete__(p) operator_delete((unsigned char*)(p))
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


// Particle / angle helpers

// IDA: FUN_0043e430 @ 0x0043E430 — Math_GetAngleFromPoints(x1,y1,x2,y2)
// Computes clockwise angle 0..359 from point (x1,y1) toward (x2,y2).
// Uses x87 fpatan(dy/dx,1) then ftol; adds 180 if x2<x1; wraps negative.
int __cdecl Math_GetAngleFromPoints(float param_1, float param_2, float param_3, float param_4) {
    double fVar2;
    if ((double)param_3 - (double)param_1 == 0.0) {
        fVar2 = 0.0;
    } else {
        fVar2 = ((double)param_2 - (double)param_4) / ((double)param_3 - (double)param_1);
    }
    // IDA: (__int64)(atan2(v5, 1.0) * 57.295776) -- en GRADOS.  El port
    // truncaba los radianes (quedaba en -1..1), asi que el agrupamiento de los
    // pajaros (sub_43E680, unico caller) solo giraba hacia 0 o 180 grados y no
    // formaban los circulos del original.
    int iVar1 = (int)(long long)(atan2(fVar2, 1.0) * 57.295776);
    if (param_3 < param_1) iVar1 += 0xb4;   // 180
    if (iVar1 < 0)         iVar1 += 0x168;  // 360
    return (0x168 - iVar1) % 0x168;
}


// Monster/Scene data loaders
