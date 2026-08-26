// Runtime_Externs.cpp
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


// FUN_00423040 @ 0x00423040 — HashTable_Insert_Obfuscated (__thiscall this, param_1)
// STUB: uses unaff_retaddr phantom param — cannot implement safely.
void __cdecl FUN_00423040(void *ctx, void *chardata) {
    // STUB: HashTable insert with obfuscation — cannot implement safely (unaff_retaddr)
    (void)ctx; (void)chardata;
}
// FUN_00422df0 @ 0x00422DF0 — HashTable_Insert_Ptr (__thiscall this, param_1)
// STUB: uses unaff_retaddr phantom param — cannot implement safely.
void __cdecl FUN_00422df0(void *ctx, void *counter) {
    // STUB: HashTable insert (ptr) with obfuscation — cannot implement safely
    (void)ctx; (void)counter;
}
// FUN_0040e330 @ 0x0040E330 — NO es "Timer_Advance": es el ciclador del TAMAÑO
// del historial del ChatListBox (tecla F4 y botón 2 del popup del chat).
// Cicla this[35] (visible row count, +0x8C): 3 → 6 → 30 → 6 …, alternando
// g_bUseChatListBox (DAT_005590ac), y después re-scrollea.
//
// FIX 2026-07-20 — CRASH 0xC0000005 con param0=8 (violación de EJECUCIÓN):
// las 4 ramas hacían `(**(void(__cdecl**)(int))(*(int*)param_1 + 0x30))(0)`.
// `*param_1 + 0x30` es vtable+48 = entrada 12 (sub_40CC50 / scrollByN), que es
// __thiscall.  Al invocarla como __cdecl con un solo argumento, el `this` no
// viajaba en ECX: la callee tomaba como `this` la basura que hubiera quedado en
// ECX, deferenciaba su "vtable" y saltaba a una dirección arbitraria.
// El disasm (0x40E35D, 0x40E375, 0x40E39C, 0x40E3BE) muestra las 4 ramas como
// `mov eax,[ecx] / push 0 / call [eax+30h]` con ECX intacto = __thiscall(this, 0).
// Hex-Rays tipó UNA de las ramas como __stdcall sin this (perdió el tracking de
// ECX al hoistear `v2 = *this`); las otras tres sí salen como __thiscall.
static void ChatLB_ScrollBy0(int* self)
{
    // vtable+48 = entrada 12 = scrollByN(this, n).  __fastcall en nuestro build.
    typedef int (__fastcall *FnScrollByN)(int* /*ecx=this*/, int /*edx*/, int /*n*/);
    void** vt = *(void***)self;
    ((FnScrollByN)vt[12])(self, 0, 0);
}

void __cdecl FUN_0040e330(unsigned long val) {
    int *param_1 = (int*)(uintptr_t)val;
    if (!param_1 || !*(int*)param_1) return;   // objeto sin construir / vtable nula
    switch (param_1[0x23]) {
    case 3:
        param_1[0x23] = 6;
        ChatLB_ScrollBy0(param_1);
        return;
    default:
        if (param_1[0x23] >= 0x1f) {
            DAT_005590ac = 1;
            param_1[0x23] = 6;
        }
        ChatLB_ScrollBy0(param_1);
        return;
    case 6: case 9: case 0xc: case 0xf: case 0x12: case 0x15: case 0x18: case 0x1b:
        if (DAT_005590ac == 1) {
            param_1[0x23] = 0x1e;
        } else {
            DAT_005590ac = 1;
            param_1[0x23] = 3;
        }
        ChatLB_ScrollBy0(param_1);
        return;
    case 0x1e:
        param_1[0x23] = 6;
        DAT_005590ac = 0;
        ChatLB_ScrollBy0(param_1);
        return;
    }
}

// Scene / map helpers
