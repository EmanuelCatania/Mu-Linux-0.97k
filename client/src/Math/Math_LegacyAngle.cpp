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


// Particle / angle helpers

// FUN_0043e430 @ 0x0043E430 — Angle_From2Points(x1,y1,x2,y2)
// Computes clockwise angle 0..359 from point (x1,y1) toward (x2,y2).
// Uses x87 fpatan(dy/dx,1) then ftol; adds 180 if x2<x1; wraps negative.
int __cdecl FUN_0043e430(float param_1, float param_2, float param_3, float param_4) {
    double fVar2;
    if ((double)param_3 - (double)param_1 == 0.0) {
        fVar2 = 0.0;
    } else {
        fVar2 = ((double)param_2 - (double)param_4) / ((double)param_3 - (double)param_1);
    }
    int iVar1 = (int)atan(fVar2);  // fpatan → ftol (result in radians-as-int units)
    if (param_3 < param_1) iVar1 += 0xb4;   // 180
    if (iVar1 < 0)         iVar1 += 0x168;  // 360
    return (0x168 - iVar1) % 0x168;
}


// Monster/Scene data loaders
