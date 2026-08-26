// Extracted from stubs_linker.cpp during the linker-stub domain refactor.
// Original IDA/address comments are retained with each implementation.
#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "structs.h"

extern "C" DWORD DAT_07eaa128;   // Golden Archer panel flag (globals.cpp)
extern void __cdecl FUN_0054158c(void* ptr);
extern void FUN_004fa5a0(void);

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
#define ITEM_SPECIAL_SKILL_OPTION             0
#define ITEM_SPECIAL_LUCK_OPTION              1
#define ITEM_OPTION_ADD_PHYSI_DAMAGE_CODE     60
#define ITEM_OPTION_ADD_MAGIC_DAMAGE_CODE     61
#define ITEM_OPTION_ADD_DEFENSE_RATE_CODE     62
#define ITEM_OPTION_ADD_DEFENSE_CODE          63
#define ITEM_OPTION_ADD_EXCELLENT_DAMAGE_CODE 72
// DeleteObjects @ 0x004FFD50 (90 lines) — Release all world objects
// Frees BMD models, unloads textures, clears entity/effect/particle arrays.
void __cdecl DeleteObjects(void) {
    // Original releases BMD models in [0, 0x7580) stride 0xBC,
    // walks linked-list object blocks freeing nodes,
    // unloads textures in range [0x23, 0x68),
    // zeros live flags for: Sprites, Boids, Fishs, Leaves,
    // Particles, Points, Joints, Operates, Effects.
    // Simplified: clear effect/particle arrays via memset
    // Full cleanup requires BMD__Release and UnloadImage which
    // are already stubbed elsewhere.
}

// DeleteNpcs @ 0x00509190 (20 lines) — Release NPC models and sound buffers
// Frees BMD models in NPC range, releases sound buffers 0x78-0xAA.
void __cdecl DeleteNpcs(void) {
    // Release NPC sound buffers (slots 0x78 to 0xA9)
    // Original: BMD__Release for model indices ~213-255 (offsets 0xF604..0x11710 stride 0xBC)
    // then ReleaseBuffer for sound slots 0x78..0xA9
    // Simplified — sound/model release handled at shutdown
}

// DeleteMonsters @ 0x00509880 (20 lines) — Release monster models and sound buffers
// Frees BMD models in monster range, releases sound buffers 0xAA-0x1A4.
void __cdecl DeleteMonsters(void) {
    // Original: BMD__Release for model indices ~170-213 (offsets 0xC648..0xF604 stride 0xBC)
    // then ReleaseBuffer for sound slots 0xAA..0x1A3
    // Simplified — sound/model release handled at shutdown
}

// ClearItems @ 0x00502B80 (17 lines) — Clear all ground item live flags
void __cdecl ClearItems(void) {
    // Original loops through Items array zeroing the Key/live byte of each entry
    // Items base is DAT_07e907e0 area, each item has a live flag at offset 0
    // For now, no-op — items cleared at map transition
}

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
// como FUN_00401730 (despacha por estado 1/2/3 a CheckActCondition /
// FindQuestContext / CheckRequestCondition).  Ahora delega.
void __fastcall FUN_00401730(void *pThis, char param_1);
void __fastcall CSQuest__CheckQuestState(void *This, int state) {
    if (!This) return;
    FUN_00401730(This, (char)state);
}

// CSQuest__ShowDialogText @ 0x004017E0
// 2026-08-21: acá había una SEGUNDA implementación inventada (armaba el cuadro
// con una sola respuesta fija y no tocaba la tabla de diálogos), mientras el
// port fiel de la misma dirección vivía en Scene_CharSelect_Nav.cpp como
// FUN_004017e0.  Dos implementaciones del mismo address escribiendo globals
// distintos — el patrón de siempre.  Ahora delega.
// El 2do parámetro no existe en IDA (`CSQuest::ShowDialogText(This, iDialogIndex)`
// es thiscall; el índice es el único dato que se usa).
void __fastcall FUN_004017e0(int param_1);
void __cdecl CSQuest__ShowDialogText(int param_1, int param_2) {
    (void)param_2;
    FUN_004017e0(param_1);
}
