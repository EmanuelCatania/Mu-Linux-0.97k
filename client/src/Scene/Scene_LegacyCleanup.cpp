// Extracted from stubs_linker.cpp during the linker-stub domain refactor.
// Original IDA/address comments are retained with each implementation.
#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "structs.h"

extern "C" DWORD GoldenArcherOpenType;   // Golden Archer panel flag (globals.cpp)
extern void __cdecl operator_delete(void* ptr);
extern void FUN_004fa5a0(void);

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
#define ITEM_SPECIAL_SKILL_OPTION             0
#define ITEM_SPECIAL_LUCK_OPTION              1
#define ITEM_OPTION_ADD_PHYSI_DAMAGE_CODE     60
#define ITEM_OPTION_ADD_MAGIC_DAMAGE_CODE     61
#define ITEM_OPTION_ADD_DEFENSE_RATE_CODE     62
#define ITEM_OPTION_ADD_DEFENSE_CODE          63
#define ITEM_OPTION_ADD_EXCELLENT_DAMAGE_CODE 72
// 2026-09-25: los cinco puentes que habia aca (DeleteObjects, DeleteNpcs,
// DeleteMonsters, ClearItems y ClearCharacters) se eliminaron al renombrar:
// solo redirigian al FUN_ de la misma direccion, y con los dos lados ya con
// el mismo nombre quedaban llamandose a si mismos.  Las implementaciones
// reales viven en Render/SMD_Parser.cpp y Terrain/Terrain_LegacyLoad.cpp.


// CSQuest__CheckQuestState @ 0x00401730
// 2026-08-21: acá había un resumen inventado ("State machine dispatch
// (simplified)") que sólo escribía el byte de estado y descartaba el resto,
// mientras el port fiel de la misma dirección vive en Scene_CharSelect_Nav.cpp
// como CSQuest_CheckQuestState (IDA: FUN_00401730; despacha por estado 1/2/3 a CheckActCondition /
// FindQuestContext / CheckRequestCondition).  Ahora delega.
void __fastcall CSQuest_CheckQuestState(void *pThis, char param_1); // IDA: FUN_00401730
void __fastcall CSQuest__CheckQuestState(void *This, int state) {
    if (!This) return;
    CSQuest_CheckQuestState(This, (char)state);
}

// CSQuest__ShowDialogText @ 0x004017E0
// 2026-08-21: acá había una SEGUNDA implementación inventada (armaba el cuadro
// con una sola respuesta fija y no tocaba la tabla de diálogos), mientras el
// port fiel de la misma dirección vivía en Scene_CharSelect_Nav.cpp como
// CSQuest_ShowDialogText (IDA: FUN_004017E0). Dos implementaciones del mismo address escribiendo globals
// distintos — el patrón de siempre.  Ahora delega.
// El 2do parámetro no existe en IDA (`CSQuest::ShowDialogText(This, iDialogIndex)`
// es thiscall; el índice es el único dato que se usa).
void __fastcall CSQuest_ShowDialogText(int param_1); // IDA: FUN_004017E0
void __cdecl CSQuest__ShowDialogText(int param_1, int param_2) {
    (void)param_2;
    CSQuest_ShowDialogText(param_1);
}
