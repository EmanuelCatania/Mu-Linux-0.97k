// Item_SpecialOption.cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// Item_GetDefenseWithSpecial @ 0x0047CFB0 (~17 lines) — get item special option value
// If item type==-1: return 0. Otherwise PlusSpecial(0x3f, item).
// IDA: sub_47CFB0 (0x0047CFB0)
// IDA sub_47CFB0: defensa del item (Value[9]) + PlusSpecial(63) (opcion de
// defensa adicional).  2026-09-12: faltaba el PlusSpecial, asi que el DefRate
// (FUN_0047e160) salia sin ese bonus.  El primer parametro sigue sin usarse.
extern "C" void __cdecl PlusSpecial(unsigned short *Value, int Special, DWORD Item);
unsigned int __cdecl Item_GetDefenseWithSpecial(DWORD ecx, short* param_1) {
    (void)ecx;
    if (!param_1 || *param_1 == -1) return 0;
    unsigned short value = (unsigned short)param_1[9];
    PlusSpecial(&value, 63, (DWORD)(uintptr_t)param_1);
    return (unsigned int)(short)value;   // IDA devuelve __int16
}
