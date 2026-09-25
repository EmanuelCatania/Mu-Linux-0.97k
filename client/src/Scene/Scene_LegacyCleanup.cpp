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
// DeleteObjects / DeleteNpcs / DeleteMonsters / ClearItems — delegan en los
// ports reales de las mismas direcciones (los que usa OpenWorld al cambiar de
// mapa).
//
// 2026-09-16: los cuatro eran cuerpos VACIOS ("Simplified — handled at
// shutdown").  Su unico caller es ReleaseMainData (0x5110A0), que llama
// ReceiveLogOut al volver al char-select o al login: el mundo quedaba cargado
// entero (modelos, texturas, objetos, sonidos, items del suelo) y el
// char-select andaba lento.  Mismo patron de simbolo duplicado de siempre.
void __cdecl DeleteObjects(void)  { FUN_004ffd50(); }   // IDA: DeleteObjects  (0x004FFD50)
void __cdecl DeleteNpcs(void)     { FUN_00509190(); }   // IDA: DeleteNpcs     (0x00509190)
void __cdecl DeleteMonsters(void) { FUN_00509880(); }   // IDA: DeleteMonsters (0x00509880)
void __cdecl ClearItems(void)     { FUN_00502b80(); }   // IDA: ClearItems     (0x00502B80)

// ClearCharacters @ 0x0045ABB0 — DUPLICADO de FUN_0045abb0 (misma direccion).
// 2026-07-24: antes esta version leia el Key del offset EQUIVOCADO (+4 en vez
// de +476).  La impl VIVA (la que llama OpenWorld) es FUN_0045abb0 en
// Render/SMD_Parser.cpp, que ya lee +0x1dc correcto.  Se delega para que no
// haya dos comportamientos distintos para el mismo 0x45ABB0.
void __cdecl ClearCharacters(int Key) { FUN_0045abb0(Key); }

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
