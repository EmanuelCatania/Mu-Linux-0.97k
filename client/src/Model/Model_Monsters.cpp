// IDA: OpenPlayerTextures @ 0x00507610 — Model_LoadPlayerEquipmentTextures
// Loads player skin textures (slots 0x12d-0x133) and binds texture paths to
// all class/tier equipment model slots using OpenTexture (Model_LoadTextures).
// Also loads robe textures at the end.
// Note: despite the name, this loads textures for player gear, not monster models.
// Monster models are handled by Monster_Data.cpp (Monster_LoadStartupData, IDA: Monster_Data_Load).
#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// IDA: OpenPlayerTextures
void __cdecl Model_LoadPlayerEquipmentTextures(void)
{
    // Initialise texture slot 0x12d (Barbarian skin)
    SetMaxTextures(0x12d);
    OpenJPG("Player\\skin_barbarian_01.jpg", 0x12d, 0x2600, 0x2900, 0, '\x01');
    TextureCurrent++;
    OpenJPG("Player\\level_man022.jpg",      0x12e, 0x2600, 0x2900, 0, '\x01');
    TextureCurrent++;
    OpenJPG("Player\\skin_wizard_01.jpg",    0x12f, 0x2600, 0x2900, 0, '\x01');
    TextureCurrent++;
    OpenJPG("Player\\level_man01.jpg",       0x130, 0x2600, 0x2900, 0, '\x01');
    TextureCurrent++;
    OpenJPG("Player\\skin_archer_01.jpg",    0x131, 0x2600, 0x2900, 0, '\x01');
    TextureCurrent++;
    OpenJPG("Player\\level_man033.jpg",      0x132, 0x2600, 0x2900, 0, '\x01');
    TextureCurrent++;
    OpenJPG("Player\\skin_special_01.jpg",   0x133, 0x2600, 0x2900, 0, '\x01');
    TextureCurrent++;

    // Slot 0x136: Load textures for all class equipment slots (class 1-4, 5 piece types)
    SetMaxTextures(0x136);

    // Class equipment texture binding (slots 0x390-0x3af range, class 1-4)
    // IDA 0x00507610 L38: `while (v0 - 919 < 4)` con v0=919 → 4 iteraciones (919..922).
    for (int i = 0x397; i-0x397 < 4; i++) {
        OpenTexture(i - 7,  "Player\\", 0x2600, '\x01');
        OpenTexture(i,      "Player\\", 0x2600, '\x01');
        OpenTexture(i + 7,  "Player\\", 0x2600, '\x01');
        OpenTexture(i + 0xe, "Player\\", 0x2600, '\x01');
        OpenTexture(i + 0x15,"Player\\", 0x2600, '\x01');
    }

    // Male equipment texture binding (slots 0x270-0x30f range, tiers 1-10 × 5 types)
    // IDA: `v1-656 < 17` → 17 iteraciones (656..672).
    for (int i = 0x290; i-0x290 < 0x11; i++) {
        OpenTexture(i - 0x20, "Player\\", 0x2600, '\x01');
        OpenTexture(i,        "Player\\", 0x2600, '\x01');
        OpenTexture(i + 0x20, "Player\\", 0x2600, '\x01');
        OpenTexture(i + 0x40, "Player\\", 0x2600, '\x01');
        OpenTexture(i + 0x60, "Player\\", 0x2600, '\x01');
    }

    // Class2 equipment texture binding (slots 0x394-0x3b3, class2 tiers 1-3 × 5 types)
    // IDA: `v2-919 < 7` con v2=923 → 3 iteraciones (923..925).
    for (int i = 0x39b; i-0x39b < 3; i++) {
        OpenTexture(i - 7,  "Player\\", 0x2600, '\x01');
        OpenTexture(i,      "Player\\", 0x2600, '\x01');
        OpenTexture(i + 7,  "Player\\", 0x2600, '\x01');
        OpenTexture(i + 0xe, "Player\\", 0x2600, '\x01');
        OpenTexture(i + 0x15,"Player\\", 0x2600, '\x01');
    }

    // Elf equipment texture binding (slots 0x280-0x30f, tiers × 5)
    // IDA: `v3-673 < 4` con v3=673 → 4 iteraciones (673..676).
    for (int i = 0x2a1; i-0x2a1 < 4; i++) {
        OpenTexture(i - 0x20, "Player\\", 0x2600, '\x01');
        OpenTexture(i,        "Player\\", 0x2600, '\x01');
        OpenTexture(i + 0x20, "Player\\", 0x2600, '\x01');
        OpenTexture(i + 0x40, "Player\\", 0x2600, '\x01');
        OpenTexture(i + 0x60, "Player\\", 0x2600, '\x01');
    }

    DAT_0055a7c4 = 1;

    // Shadow texture
    OpenTexture(0x187, "Player\\", 0x2600, '\x01');

    // Robe textures (worn-item rendering)
    OpenJPG("Player\\Robe01.jpg",  0x1ea, 0x2600, 0x2900, 0, '\x01');
    OpenJPG("Player\\Robe02.jpg",  0x1eb, 0x2600, 0x2900, 0, '\x01');
    OpenTGA("Player\\Robe03.tga", 0x1ec, 0x2600, 0x2900, 0, '\x01');
}
