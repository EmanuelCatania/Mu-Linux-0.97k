// Font_LegacyFactory.cpp
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


// Font_CreateTextDib (FUN_0050f5f0) — implemented in src/Render/Font_Layout.cpp
// FUN_0040f570 @ 0x0040F570 — Font_CreateRenderer(this, type, dc)
// Creates a font rendering object: if type==1, allocates a 0x2C4-byte widget (FUN_0040f730),
// otherwise allocates a 4-byte simple widget (FUN_00410a90). Stores at this+4,
// sets this+8 = type, then calls the vtable's first virtual method with dc.
void __cdecl Font_CreateRenderer(int self, int param_1, int param_2)
{
    void* pObj = NULL;
    if (param_1 == 1) {
        void* mem = operator_new(0x2C4);
        if (mem) {
            pObj = (void*)FUN_0040f730(mem);
        }
    } else {
        int* mem = (int*)operator_new(4);
        if (mem) {
            FUN_00410a90(mem);
            pObj = (void*)mem;
        }
    }
    *(void**)(self + 4) = pObj;
    *(int*)(self + 8) = param_1;
    // Call vtable method 0: (*(code**)*pObj)(param_2)
    if (pObj) {
        typedef void (__cdecl *VtblFn)(int);
        VtblFn fn = *(VtblFn*)(*(DWORD*)pObj);
        if (fn) fn(param_2);
    }
}

// Model data loaders — implemented in src/Model/
