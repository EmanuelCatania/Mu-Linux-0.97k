// Extracted from stubs_linker.cpp during the linker-stub domain refactor.
// Original IDA/address comments are retained with each implementation.
#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "structs.h"

extern "C" DWORD DAT_07eaa128;
extern void __cdecl FUN_0054158c(void* ptr);
extern void FUN_004fa5a0(void);
#ifndef qmemcpy
#define qmemcpy(dst,src,sz) memcpy((dst),(src),(size_t)(sz))
#endif
#ifndef delete__
#define delete__(p) FUN_0054158c((unsigned char*)(p))
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
#ifndef IDA_PORT_004FDC00   // desactivado: el port FULL vive en stubs_IDA_ports.cpp
// 0x004FDC00 — MoveObjects: tick per-frame de cada objeto visible del mundo.
//
// 2026-08-11 — UNIFICACIÓN. Existían DOS ports de esta función:
//   · éste, mínimo, que sólo hacía Alpha + el banner MUGAME del login, y
//   · `MoveObject_PerWorld_stub` (stubs_game.cpp), con el toggle por HeroTile,
//     PlayAnimation y el switch por World COMPLETO.
// El que se llamaba desde el loop de MoveObjects era éste, así que el switch
// por World nunca corría: **ningún objeto del mundo generaba sus efectos**.
// Entre otras cosas, los tipos 130/131/132 de Lorencia (Light01/02/03) quedaban
// visibles como cajas de 8 vértices con la textura dummy `ston03` (2x2 negra)
// en vez de ocultarse (`HiddenMesh = -2`) y emitir el humo de las chimeneas y
// de la forja del herrero. Mismo patrón que `OpenSMDFile` / `RenderText` /
// `SetPlayerStop`: un símbolo con dos implementaciones donde gana la incompleta.
//
// Ahora hay una sola implementación, en el orden del binario:
//   World 9 → World 0/2 toggles → Alpha → early-return → PlayAnimation →
//   bloque de login (160/161/162) → switch por World.
void __cdecl FUN_004fdc00(float pObj) {
    if (LODWORD(pObj) == 0) return;
    MoveObject_PerWorld_stub(pObj);
}
#endif  // IDA_PORT_004FDC00 (minimal disabled)
