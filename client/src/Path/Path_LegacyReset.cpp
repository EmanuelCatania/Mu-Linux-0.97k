// Path_LegacyReset.cpp
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


// IDA: FUN_0043f2d0 @ 0x0043F2D0 — Pathfinder_Reset: frees + re-initialises A* context at DAT_05826df4.
// Despite the "Font_Reset" comment in functions.h, this is clearly the A* grid init.
void __cdecl PathFinder_ResetContext(void) {
    DWORD *puVar4 = (DWORD*)DAT_05826df4;
    if ((void*)puVar4[0xff]  != nullptr) { operator_delete((void*)puVar4[0xff]);  puVar4[0xff]  = 0; }
    if ((void*)puVar4[0x102] != nullptr) { operator_delete((void*)puVar4[0x102]); puVar4[0x102] = 0; }
    if ((void*)puVar4[0x103] != nullptr) { operator_delete((void*)puVar4[0x103]); puVar4[0x103] = 0; }
    if ((void*)puVar4[0x104] != nullptr) { operator_delete((void*)puVar4[0x104]); puVar4[0x104] = 0; }
    puVar4[0x100] = 1950000000;
    puVar4[0x101] = 0xffffffff;
    puVar4[0]     = 0x100;
    puVar4[1]     = 0x100;
    puVar4[3]     = (DWORD)&DAT_0838bc70;
    puVar4[2]     = 0x10000;
    puVar4[0xff]  = (DWORD)operator_new(0x10000);
    puVar4[0x102] = (DWORD)operator_new(puVar4[2] << 2);
    puVar4[0x103] = (DWORD)operator_new(puVar4[2] << 2);
    void *pvVar1  = operator_new(puVar4[2] << 2);
    DWORD uVar3   = puVar4[2];
    puVar4[0x104] = (DWORD)pvVar1;
    DWORD *p      = (DWORD*)puVar4[0xff];
    for (DWORD i = uVar3 >> 2; i != 0; i--) { *p = 0; p++; }
    for (DWORD r = uVar3 &  3; r != 0; r--) { *(BYTE*)p = 0; p = (DWORD*)((BYTE*)p + 1); }
}
