#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "structs.h"

extern "C" DWORD GoldenArcherOpenType;
extern void __cdecl operator_delete(void* ptr);
extern void ClearActionObject(void);
#ifndef qmemcpy
#define qmemcpy(dst,src,sz) memcpy((dst),(src),(size_t)(sz))
#endif
#ifndef delete__
#define delete__(p) operator_delete((unsigned char*)(p))
#endif
#ifndef __OFSUB__
#define __OFSUB__(x,y) (0)
#endif
#ifndef LODWORD
#define LODWORD(x) (*((DWORD*)&(x)))
#define HIDWORD(x) (*(((DWORD*)&(x))+1))
#define SLOBYTE(x) (*((char*)&(x)))
#define SLOWORD(x) (*((short*)&(x)))
#define SLODWORD(x) (*((int*)&(x)))
#endif
#ifndef LOBYTE
#define LOBYTE(x) (*((unsigned char*)&(x)))
#define HIBYTE(x) (*(((unsigned char*)&(x))+1))
#define LOWORD(x) (*((unsigned short*)&(x)))
#define HIWORD(x) (*(((unsigned short*)&(x))+1))
#endif
#define ITEM_SPECIAL_SKILL_OPTION 0
#define ITEM_SPECIAL_LUCK_OPTION 1
#define ITEM_OPTION_ADD_PHYSI_DAMAGE_CODE 60
#define ITEM_OPTION_ADD_MAGIC_DAMAGE_CODE 61
#define ITEM_OPTION_ADD_DEFENSE_RATE_CODE 62
#define ITEM_OPTION_ADD_DEFENSE_CODE 63
#define ITEM_OPTION_ADD_EXCELLENT_DAMAGE_CODE 72
#ifndef IDA_PORT_004FDC00   // desactivado: el port FULL vive en Scene_ObjectUpdate.cpp
// 0x004FDC00 — MoveObjects: tick per-frame de cada objeto visible del mundo.
//
// Versión mínima DESACTIVADA (IDA_PORT_004FDC00 está definida en globals.h):
// la copia viva es el port completo de FUN_004fdc00 en Scene_ObjectUpdate.cpp,
// en el orden del binario:
//   World 9 → World 0/2 toggles → Alpha → early-return → PlayAnimation →
//   bloque de login (160/161/162) → switch por World.
void __cdecl FUN_004fdc00(float pObj) {
    if (LODWORD(pObj) == 0) return;
    MoveObject_PerWorld(pObj);
}
#endif  // IDA_PORT_004FDC00 (minimal disabled)
