// Entity_ActionLegacy.cpp
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



// FUN_0043e820 @ 0x0043E820 — SetAction(object_ptr, action)
// Sets animation action on an OBJECT. Checks action < Models[type].numActions,
// with exceptions for actions 77 and 76. If action changes, saves prior state.
//
// Models base = DAT_05828d58 (already a pointer, no extra indirection).
// Per disasm @ 0x0043e830-0x0043e836:
//   ECX = [0x05828d58]                       ; table base
//   slot offset = type * 47 * 4 = type*0xBC
//   numLimit = *(short*)(ECX + slot_off + 0x26)
// El offset 0x26 en el slot BMD es nBones (no nActions); el original valida
// el action_id contra ese campo igualmente — replicamos sin reinterpretar.
//
// BUG previo: agregábamos un deref *(DWORD*)slot que leía los primeros 4 bytes
// del modelName ("Play"=0x79616C50) como puntero y crasheaba en *(short*)(0x79616C50+38).
void* __cdecl FUN_0043e820(int entity_ptr, int anim_id)
{
    if (entity_ptr == 0) return NULL;
    if (DAT_05828d58 == 0) return NULL;           // tabla aún no inicializada
    short objType = *(short*)(entity_ptr + 0x02); // OBJECT.Type
    short numActions = *(short*)(DAT_05828d58 + (int)objType * 0xBC + 0x26);

    // Allow action if < numActions, or if it's 77 (0x4D) or 76 (0x4C)
    if (anim_id >= numActions && anim_id != 77 && anim_id != 76)
        return NULL;

    BYTE curAction = *(BYTE*)(entity_ptr + 0x105); // OBJECT.CurrentAction
    if ((int)(unsigned int)curAction != anim_id) {
        *(BYTE*)(entity_ptr + 0x106) = curAction;             // PriorAction
        *(float*)(entity_ptr + 0x10C) = *(float*)(entity_ptr + 0x108); // PriorAnimationFrame = AnimationFrame
        *(BYTE*)(entity_ptr + 0x105) = (BYTE)anim_id;         // CurrentAction
        *(float*)(entity_ptr + 0x108) = 0.0f;                 // AnimationFrame = 0
    }
    return (void*)entity_ptr;
}
