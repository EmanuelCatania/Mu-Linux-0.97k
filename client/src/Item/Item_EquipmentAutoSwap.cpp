// Item_EquipmentAutoSwap.cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

extern "C" void __cdecl UI_Main(int slot_idx, short* inv_base, unsigned int gridW);

// IDA: sub_48B680 @ 0x0048B680 (1006 bytes) — sin nombre en el binario;
// `FUN_0048b680` es el alias del port.  Auto-swap de arma de Elf.
// 1. Calls FUN_004824c0() to get equipped weapon type index.
// 2. If weapon == -1, returns immediately (no weapon equipped).
// 3. anti-tamper hash table — skipped (encrypt CharacterMachine)
// 4. If class == elf (2) and DAT_07e91388 == 0:
//    reads weapon item types from CharacterMachine equipment slots.
//    - If elf-bow (type 0x80..0x86 or 0x91) equipped and crossbow slot empty:
//      copies item to pPickedItem, sends equipment swap request to slot 0.
//    - If crossbow (type 0x88..0x8E or 0x90..0x9F) equipped and bow slot empty:
//      copies item to pPickedItem, sends equipment swap request to slot 1.
// 5. anti-tamper hash table — skipped (decrypt CharacterMachine)
//
// The input parameter is an artefact of the caller's stack in the original
// declaration.  0048B685 always obtains the actual source slot itself through
// FUN_004824C0, then uses that slot for the local drag state and the 0x24 item
// move request.  The encrypted CharacterMachine bookkeeping surrounding this
// logic is intentionally not reproduced here.
void __cdecl FUN_0048b680(int /*unused*/) {
    const int inventorySlot = Item_FindElfWeaponInventorySlot();
    if (inventorySlot < 0 || inventorySlot >= 64)
        return;

    BYTE* const characterMachine = (BYTE*)(uintptr_t)DAT_07cf1ffc;
    BYTE* const characterAttribute = (BYTE*)(uintptr_t)DAT_07cf1ff4;
    if (!characterMachine || !characterAttribute)
        return;

    // 0048B762..0048B78A: only an unblocked Elf enables the two-hand swap.
    if ((characterAttribute[11] & 7) != 2 || DAT_07e91388 != 0)
        return;

    // IDA sub_48B680 L120-146.  Los slots estaban invertidos:
    //
    //   if ( (v18 >= 128 && v18 < 135 || v18 == 145) && *v25 == -1 )
    //   { ... dword_7E11E78 = 0; SendRequestEquipmentItem(0, v19, 0, 0); }   // slot 0
    //   else
    //   {
    //       v20 = *v25;
    //       if ( v20 >= 136 && v20 < 143 || v20 >= 144 && v20 < 160 && v18 == -1 )
    //       { ... dword_7E11E78 = 1; SendRequestEquipmentItem(0, v21, 0, 1); } // slot 1
    //   }
    //
    // `v18` es el test de ARCO y `v20` el de BALLESTA.  Segun sub_4824C0 L76-88
    // y CreateArrow (0x474370 L67), el arco vive en CharacterMachine + 604 y la
    // ballesta en + 536; la municion va al slot que queda libre.  El port leia
    // el arco en +536 y la ballesta en +604, o sea al reves: con arco equipado
    // ninguna rama daba, y al quedarse sin flechas (+536 == -1) caia en la rama
    // de ballesta y pedia equipar en el slot 1 — el del propio arco.
    const short bowSlotType      = *(const short*)(characterMachine + 604);  // IDA: v18
    const short crossbowSlotType = *(const short*)(characterMachine + 536);  // IDA: v20 / *v25

    int targetEquipmentSlot = -1;
    const char* chatId = nullptr;
    if (((bowSlotType >= 128 && bowSlotType < 135) || bowSlotType == 145)
        && crossbowSlotType == -1)
    {
        targetEquipmentSlot = 0;                       // arco -> flechas al slot 0
        chatId = (const char*)&DAT_07e11dec;
    }
    else if ((crossbowSlotType >= 136 && crossbowSlotType < 143)
          || (crossbowSlotType >= 144 && crossbowSlotType < 160 && bowSlotType == -1))
    {
        targetEquipmentSlot = 1;                       // ballesta -> bolts al slot 1
        chatId = (const char*)&DAT_07e11df0;
    }
    else
    {
        return;
    }

    ITEM* const source = (ITEM*)(OffsetInventoryItems + (size_t)inventorySlot * sizeof(ITEM));
    memcpy(DAT_07e91350, source, sizeof(ITEM));

    const int encodedSourceSlot = inventorySlot + 12;
    UI_Main(encodedSourceSlot, (short*)OffsetInventoryItems, 8);
    DAT_07e11e78 = (DWORD)targetEquipmentSlot;
    SendRequestEquipmentItem_stub(0, encodedSourceSlot, (ITEM*)DAT_07e91350,
                                  0, targetEquipmentSlot);
    UIChatLogWindow_AddText(chatId, GlobalText[250], 1);
}
