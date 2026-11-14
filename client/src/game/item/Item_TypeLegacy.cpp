#include "stdafx.h"
#include "game/item/ItemDefines.h"
#include "globals.h"
#include "functions.h"
#include "net/Net.h"

extern void __cdecl operator_delete(void* ptr);

// ConvertItemType @ 0x0047B110 — ConvertItemType(BYTE* Item)  (21 bytes)
// Port FIEL desde IDA: returns Item[0] + 2*(Item[3] & 0x80). The high bit of
// Item[3] is the "Type Hi" flag used to distinguish item categories beyond
// 256 entries (e.g. shields >= 0x100).
//
// DESVIACION (0.97.20): el item viaja en 7 bytes y el nibble alto del byte 5
// lleva los bits 9-12 del índice (agregados >= 512).  Para un vanilla vale 0
// y el resultado es el del binario.
extern "C" int __cdecl ConvertItemType(BYTE* Item)
{
    return ItemWire_GetType(Item);
}
