// Item_SpecialOption.cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// FUN_0047cfb0 @ 0x0047CFB0 (~17 lines) — get item special option value
// If item type==-1: return 0. Otherwise PlusSpecial(0x3f, item).
unsigned int __cdecl FUN_0047cfb0(DWORD ecx, short* param_1) {
    (void)ecx;
    if (!param_1 || *param_1 == -1) return 0;
    // PlusSpecial(&result, 0x3f, (ITEM*)param_1);
    return (unsigned int)param_1[9];
}
