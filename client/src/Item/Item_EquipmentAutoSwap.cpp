// Item_EquipmentAutoSwap.cpp
// Extracted from stubs_game.cpp. IDA provenance remains in function comments.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// FUN_0048b680 @ 0x0048B680 (~247 lines) — elf weapon auto-swap
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
// NOTE: ~70% of Ghidra output is anti-tamper encrypt/decrypt.
// Phantom stack params (unaff_EDI, psStack_18, psStack_1c) carry weapon slot data
// pushed by the caller, making full reimplementation impossible without caller context.
// The weapon swap logic below is the core behavior.
void __stdcall FUN_0048b680(void) {
    int weaponType = Item_FindElfWeaponInventorySlot();
    if (weaponType == -1) return;

    // anti-tamper hash table — skipped (encrypt CharacterMachine)

    char* cm = (char*)(uintptr_t)DAT_07cf1ffc;  // CharacterMachine base

    // Check if class is elf (2) and DAT_07e91388 == 0
    char* charAttr = (char*)DAT_07cf1ff4;
    int charClass = *(short*)(charAttr) & 7;  // CharacterAttribute->Class & 7

    if (charClass == 2 && DAT_07e91388 == 0) {
        // Elf class: read weapon slot item types
        // Slot 0 (right hand) = CharacterMachine + 0x10c * 2 = offset in equipment
        // Slot 1 (left hand) = CharacterMachine + 0x12e * 2 = offset in equipment
        // These are within the 0x44-byte stride equipment array at CharacterMachine + 0x218
        short rightType = *(short*)(cm + 0x218);  // equipment slot 0 (right hand)
        short leftType  = *(short*)(cm + 0x218 + 0x44);  // equipment slot 1 (left hand)

        // Elf bow items: type 0x80..0x86 or 0x91
        // Crossbow items: type 0x88..0x8E or 0x90..0x9F (excluding 0x91)
        bool rightIsBow = (rightType > 0x7F && rightType < 0x87) || rightType == 0x91;
        bool leftIsEmpty = (leftType == -1);
        bool leftIsCrossbow = (leftType > 0x87 && leftType < 0x8F) ||
                              (leftType > 0x8F && leftType < 0xA0 && rightType == -1);

        // Weapon swap logic handled by caller via phantom params
        // (psStack_18, psStack_1c carry actual slot indices from caller context)
        // Without caller context, we note the logic but cannot execute the swap.
        (void)rightIsBow; (void)leftIsEmpty; (void)leftIsCrossbow;
    }

    // anti-tamper hash table — skipped (decrypt CharacterMachine)
}
