// Model_LoadSkillEffects @ 0x0050eb80
// Loads UI interface textures (inventory slots, buttons, skill icons, money
// icons, trading UI, guild badge, etc.) into texture slots 0x22 and 0x104-0x12a.
// Despite the name, this loads inventory/UI interface textures, not skill
// particle effects (those are in Model_Misc.cpp).
#include "stdafx.h"
#include "globals.h"
#include "functions.h"

void __cdecl FUN_0050eb80(void)
{
    FUN_00529740("Interface\\Skill.jpg",          0x12a, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface\\Item_Back01.jpg",    0x104, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface\\Item_Back02.jpg",    0x105, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface\\Item_Cap.jpg",       0x107, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface\\Item_Upper.jpg",     0x108, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface\\Item_Lower.jpg",     0x109, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface\\Item_Weapon01.jpg",  0x10a, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface\\Item_Gloves.jpg",    0x10b, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface\\Item_Boots.jpg",     0x10c, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface\\Item_NeckLace.jpg",  0x10d, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface\\Item_Ring.jpg",      0x10e, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface\\Item_Money.jpg",     0x10f, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface\\Money_01.jpg",       0x110, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface\\Money_02.jpg",       0x111, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface\\Item_Wing.jpg",      0x112, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface\\Item_Fairy.jpg",     0x113, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface\\Item_Weapon02.jpg",  0x114, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface\\Item_box.jpg",       0x115, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface\\InventoryBox2.jpg",  0x116, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface\\Trading_line.jpg",   0x117, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface\\exit_01.jpg",        0x118, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface\\exit_02.jpg",        0x119, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface\\buy_01.jpg",         0x11a, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface\\buy_02.jpg",         0x11b, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface\\sell_01.jpg",        0x11c, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface\\sell_02.jpg",        0x11d, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface\\repair_01.jpg",      0x11e, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface\\repair_02.jpg",      0x11f, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface\\level_01.jpg",       0x120, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface\\level_02.jpg",       0x121, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface\\level_02.jpg",       0x121, 0x2600, 0x2900, 0, '\x01'); // original has duplicate
    FUN_00529740("Interface\\accept_box01.jpg",   0x122, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface\\accept_box02.jpg",   0x123, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface\\mix_button1.jpg",    0x124, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface\\mix_button2.jpg",    0x125, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface\\lock_01.jpg",        0x126, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface\\lock_02.jpg",        0x127, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface\\lock_03.jpg",        0x128, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface\\lock_04.jpg",        0x129, 0x2600, 0x2900, 0, '\x01');
    FUN_00529bd0("Interface\\guild.tga",          0x22,  0x2600, 0x2900, 0, '\x01');
}
