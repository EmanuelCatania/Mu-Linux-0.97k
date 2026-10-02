// Runtime_Externs.cpp

#include "stdafx.h"
void __fastcall FUN_0045aaa0_impl(void *_this, char flags);
void __cdecl    FUN_00408680(void *_this, char flags);
#include "globals.h"
#include "functions.h"

// -- Declaraciones de funciones definidas en otros modulos -------------------
// Cloth_Integrate vive en Scene/Scene_CharSelect_Nav.cpp y Cloth_Solve en
// Net/Crypto.cpp.
void __fastcall Cloth_Integrate(int*, float);
int  __cdecl    Cloth_Solve(DWORD *a1);

#include "Net/Net.h"

extern "C" BYTE OffsetInventoryItems[];
extern void __cdecl operator_delete(void* ptr);
extern void MapFileDecrypt(BYTE* buf, int size);

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


// IDA: STRUCT_DECRYPT (0x00423040)
// STUB: uses unaff_retaddr phantom param — cannot implement safely.
void __cdecl STRUCT_DECRYPT(void *ctx, void *chardata) {
    // STUB: HashTable insert with obfuscation — cannot implement safely (unaff_retaddr)
    (void)ctx; (void)chardata;
}
// IDA: FUN_00422DF0 (0x00422DF0)
// STUB: uses unaff_retaddr phantom param — cannot implement safely.
void __cdecl PACKET_DECRYPT(void *ctx, void *counter) {
    // STUB: HashTable insert (ptr) with obfuscation — cannot implement safely
    (void)ctx; (void)counter;
}
// ChatListBox_ScrollByN @ 0x0040E330 — NO es "Timer_Advance": es el ciclador del TAMAÑO
// del historial del ChatListBox (tecla F4 y botón 2 del popup del chat).
// Cicla this[35] (visible row count, +0x8C): 3 → 6 → 30 → 6 …, alternando
// g_bUseChatListBox, y después re-scrollea.
//
// Las 4 ramas llaman a vtable+0x30 (entrada 12, sub_40CC50 / scrollByN) como
// __thiscall(this, 0) (disasm 0x40E35D, 0x40E375, 0x40E39C, 0x40E3BE). Hex-Rays
// tipa una de ellas como __stdcall sin this: NO invocarla como __cdecl, porque
// el this no viaja en ECX y la llamada salta a una dirección arbitraria.
static void ChatLB_ScrollBy0(int* self)
{
    // vtable+48 = entrada 12 = scrollByN(this, n).  __fastcall en nuestro build.
    typedef int (__fastcall *FnScrollByN)(int* /*ecx=this*/, int /*edx*/, int /*n*/);
    void** vt = *(void***)self;
    ((FnScrollByN)vt[12])(self, 0, 0);
}

void __cdecl ChatListBox_ScrollByN(unsigned long val) {
    int *param_1 = (int*)(uintptr_t)val;
    if (!param_1 || !*(int*)param_1) return;   // objeto sin construir / vtable nula
    switch (param_1[0x23]) {
    case 3:
        param_1[0x23] = 6;
        ChatLB_ScrollBy0(param_1);
        return;
    default:
        if (param_1[0x23] >= 0x1f) {
            g_bUseChatListBox = 1;
            param_1[0x23] = 6;
        }
        ChatLB_ScrollBy0(param_1);
        return;
    case 6: case 9: case 0xc: case 0xf: case 0x12: case 0x15: case 0x18: case 0x1b:
        if (g_bUseChatListBox == 1) {
            param_1[0x23] = 0x1e;
        } else {
            g_bUseChatListBox = 1;
            param_1[0x23] = 3;
        }
        ChatLB_ScrollBy0(param_1);
        return;
    case 0x1e:
        param_1[0x23] = 6;
        g_bUseChatListBox = 0;
        ChatLB_ScrollBy0(param_1);
        return;
    }
}

// Scene / map helpers
