// Font_LegacyFactory.cpp
// Creación del renderer de fuentes (Font_CreateRenderer).

#include "stdafx.h"
void __fastcall FUN_0045aaa0_impl(void *_this, char flags);
void __cdecl    ClothNode_ScalarDtor(void *_this, char flags);
#include "globals.h"
#include "functions.h"

// -- Declaraciones de funciones definidas en otros módulos --------------------
// Cloth_Integrate vive en Physics/Cloth_Simulation.cpp y Cloth_Solve en
// Net/Crypto.cpp.
void __fastcall Cloth_Integrate(int*, float);
int  __cdecl    Cloth_Solve(DWORD *a1);

#include "net/Net.h"

extern "C" BYTE OffsetInventoryItems[];
extern void __cdecl operator_delete(void* ptr);
extern void MapFileDecrypt(BYTE* buf, int size);

// Font_CreateTextDib (FUN_0050f5f0) — implemented in src/render/Font_Layout.cpp
// FUN_0040f570 @ 0x0040F570 — Font_CreateRenderer(this, type, dc)
// Creates a font rendering object: if type==1, allocates a 0x2C4-byte widget (FUN_0040f730),
// otherwise allocates a 4-byte simple widget (FUN_00410a90). Stores at this+4,
// sets this+8 = type, then calls the vtable's first virtual method with dc.
void __cdecl Font_CreateRenderer(intptr_t self, int param_1, int param_2)
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
    *(void**)(self + 0x10) = pObj;
    *(int*)(self + 8) = param_1;
    // Call vtable method 0: (*(code**)*pObj)(param_2)
    if (pObj && sizeof(void*) == 4) {
        typedef void (__cdecl *VtblFn)(int);
        VtblFn fn = *(VtblFn*)(uintptr_t)*(DWORD*)pObj;  // el descriptor del binario queda vacio
        if (fn) fn(param_2);
    }
}

// Model data loaders — implemented in src/assets/model/
