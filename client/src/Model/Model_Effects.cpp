// Model_LoadEffects @ 0x00508d10
// Binds texture paths to item/skill model slots using FUN_00505c80
// (Model_LoadTextures).  Covers all equipment slots (0x1a1-0x3bd range)
// and skill slots (0x333).  Slot 500 is initialised as the texture-slot base.
// Note: the function name is misleading — it assigns texture prefix paths
// to already-loaded mesh slots, not particle/effect geometry.
#include "stdafx.h"
#include "globals.h"
#include "functions.h"

void __cdecl FUN_00508d10(void)
{
    // Initialise texture base slot 500
    FUN_00505bd0(500);

    // Equipment slots mapped to "Item\\" texture prefix (various class/tier ranges)
    // Slots 0x340-0x342 and 0x3be (indices 0-3)
    for (int i = 0; i < 4; i++) {
        int slot = (i < 3) ? (i + 0x340) : (i + 0x3bb);
        FUN_00505c80(slot, "Item\\", 0x2600, '\x01');
    }

    // Slots 0x313-0x316 (item 0x313+i)
    for (int i = 0; i < 4; i++)
        FUN_00505c80(i + 0x313, "Item\\", 0x2600, '\x01');

    // Slots 0x334-0x335
    for (int i = 0; i < 2; i++)
        FUN_00505c80(i + 0x334, "Item\\", 0x2600, '\x01');

    FUN_00505c80(0x366, "Item\\", 0x2600, '\x01');

    // Slots 0x33e-0x33f
    for (int i = 0; i < 2; i++)
        FUN_00505c80(i + 0x33e, "Item\\", 0x2600, '\x01');

    DAT_0055a7c4 = 1;

    // Individual item texture slot bindings
    FUN_00505c80(0x1a1, "Item\\", 0x2600, '\x01');
    FUN_00505c80(0x1a2, "Item\\", 0x2600, '\x01');
    FUN_00505c80(0x239, "Item\\", 0x2600, '\x01');
    FUN_00505c80(0x221, "Item\\", 0x2600, '\x01');
    FUN_00505c80(0x333, "Skill\\", 0x2600, '\x01');  // Skill helper texture

    // 0x367-0x36a (quest items)
    for (int i = 0; i < 4; i++)
        FUN_00505c80(i + 0x367, "Item\\", 0x2600, '\x01');

    FUN_00505c80(0x1a3, "Item\\", 0x2600, '\x01');
    FUN_00505c80(0x23a, "Item\\", 0x2600, '\x01');
    FUN_00505c80(0x222, "Item\\", 0x2600, '\x01');

    DAT_0055a7c4 = 1;

    // More individual bindings
    FUN_00505c80(0x1af, "Item\\", 0x2600, '\x01');
    FUN_00505c80(0x25f, "Item\\", 0x2600, '\x01');
    FUN_00505c80(0x260, "Item\\", 0x2600, '\x01');
    FUN_00505c80(0x1fa, "Item\\", 0x2600, '\x01');
    FUN_00505c80(0x1d7, "Item\\", 0x2600, '\x01');

    // Large block: 0x190-0x310 range (17 types x 11 offsets)
    for (int i = 0x1b0; i-0x1af < 0x11; i++) {
        FUN_00505c80(i - 0x20, "Item\\", 0x2600, '\x01');
        FUN_00505c80(i,        "Item\\", 0x2600, '\x01');
        FUN_00505c80(i + 0x20, "Item\\", 0x2600, '\x01');
        FUN_00505c80(i + 0x40, "Item\\", 0x2600, '\x01');
        FUN_00505c80(i + 0x80, "Item\\", 0x2600, '\x01');
        FUN_00505c80(i + 0xa0, "Item\\", 0x2600, '\x01');
        FUN_00505c80(i + 0x60, "Item\\", 0x2600, '\x01');
        FUN_00505c80(i + 0x180,"Item\\", 0x2600, '\x01');
        FUN_00505c80(i + 0x160,"Item\\", 0x2600, '\x01');
        FUN_00505c80(i + 0x1a0,"Item\\", 0x2600, '\x01');
        FUN_00505c80(i + 0x1c0,"Item\\", 0x2600, '\x01');
    }

    // 0x321-0x323
    for (int i = 0x11; i < 0x14; i++)
        FUN_00505c80(i + 0x310, "Item\\", 0x2600, '\x01');

    FUN_00505c80(0xd8, "Item\\Bow\\", 0x2600, '\x01');

    // 0x3b7-0x3bd
    FUN_00505c80(0x3b7, "Item\\", 0x2600, '\x01');
    FUN_00505c80(0x3b8, "Item\\", 0x2600, '\x01');
    FUN_00505c80(0x3b9, "Item\\", 0x2600, '\x01');
    FUN_00505c80(0x3ba, "Item\\", 0x2600, '\x01');
    FUN_00505c80(0x3bb, "Item\\", 0x2600, '\x01');
    FUN_00505c80(0x3bc, "Item\\", 0x2600, '\x01');
    FUN_00505c80(0x3bd, "Item\\", 0x2600, '\x01');

    // 0x361-0x363
    for (int i = 0; i < 3; i++)
        FUN_00505c80(i + 0x361, "Item\\", 0x2600, '\x01');

    // 0x364-0x365
    for (int i = 0; i < 2; i++)
        FUN_00505c80(i + 0x364, "Item\\", 0x2600, '\x01');

    // 0x3b3-0x3b4
    for (int i = 0; i < 2; i++)
        FUN_00505c80(i + 0x3b3, "Item\\", 0x2600, '\x01');
}
