// Scene_AssetLoad.cpp
// Extracted from stubs_mouse_hover.cpp; IDA provenance comments retained.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// FUN_00510320 @ 0x00510320 — UI_LoadTextures (complete game asset loader)
// Loads: cursor TGAs (slots 2-10), interface JPGs (0xE6-0x101, 0x500-0x507),
// effect JPGs/TGAs (0x47E-0x567), then invokes all model, data and sound preload routines.
void __cdecl FUN_00510320(void) {
    CHAR local_64[100];
    // Cursor sprites
    FUN_00529bd0("Interface/Cursor.tga",          2,  0x2601, 0x2900, 0, '\x01');
    FUN_00529bd0("Interface/CursorPush.tga",       3,  0x2601, 0x2900, 0, '\x01');
    FUN_00529bd0("Interface/CursorAttack.tga",     4,  0x2601, 0x2900, 0, '\x01');
    FUN_00529bd0("Interface/CursorGet.tga",        5,  0x2601, 0x2900, 0, '\x01');
    FUN_00529bd0("Interface/CursorTalk.tga",       6,  0x2601, 0x2900, 0, '\x01');
    FUN_00529bd0("Interface/CursorRepair.tga",     7,  0x2601, 0x2900, 0, '\x01');
    FUN_00529bd0("Interface/CursorLeanAgainst.tga",8,  0x2601, 0x2900, 0, '\x01');
    FUN_00529bd0("Interface/CursorSitDown.tga",    9,  0x2601, 0x2900, 0, '\x01');
    FUN_00529bd0("Interface/CursorDontMove.tga",  10,  0x2601, 0x2900, 0, '\x01');
    // Interface / HUD JPGs
    FUN_00529740("Interface/Menu01_new.jpg",  0xe6, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/Menu02.jpg",      0xe7, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/Menu03_new.jpg",  0xe8, 0x2600, 0x2900, 0, '\x01');
    FUN_00529bd0("Interface/Menu04.tga",      0xe9, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/Menu_Red.jpg",    0xea, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/Menu_Blue.jpg",   0xeb, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/Menu_Party.jpg",  0xec, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/Menu_Character.jpg",  0xed, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/Menu_Inventory.jpg",  0xee, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/dialogue2_1.jpg", 0xef, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/Message_box.jpg", 0xf0, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/ok.jpg",          0xf1, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/ok2.jpg",         0xf2, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/cancel.jpg",      0xf3, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/cancel2.jpg",     0xf4, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/Level_box.jpg",   0xf5, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/level01.jpg",     0xf6, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/Menu_Green.jpg",  0xf7, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/dialogue1.jpg",   0xf8, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/dialogue2.jpg",   0xf9, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/Guild.jpg",       0xfa, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/Message_box1.jpg",0xfb, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/Message_box2.jpg",0xfc, 0x2600, 0x2900, 0, '\x01');
    // NIS (new interface system) frames
    FUN_00529bd0("Interface/nis_rsframe.tga", 0x500, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/nis_vframe.jpg",  0x501, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/nis_bar.jpg",     0x502, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/nis_back.jpg",    0x503, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/nis_btnarrow.jpg",0x504, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/nis_btnchat.jpg", 0x505, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/nis_btnsize.jpg", 0x506, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/nis_btnalpha.jpg",0x507, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/bar.jpg",         0xfd, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/back1.jpg",       0xfe, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/back2.jpg",       0xff, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/back3.jpg",      0x100, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Interface/Menu03_new_AG.jpg",0x101,0x2600, 0x2900, 0, '\x01');
    // Effect textures
    FUN_00529740("Effect/Fire01.jpg",     0x4ab, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/Fire02.jpg",     0x4ac, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/Fire03.jpg",     0x4ad, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/PoundingBall.jpg",0x567,0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/fi01.jpg",       0x565, 0x2601, 0x2900, 0, '\x01');
    FUN_00529bd0("Effect/fi02.tga",       0x566, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/Flame01.jpg",    0x4b0, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/flare01.jpg",    0x47e, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/Magic_Ground2.jpg",0x4f0,0x2601,0x2900, 0, '\x01');
    FUN_00529740("Effect/Magic_Circle1.jpg",0x4f1,0x2601,0x2900, 0, '\x01');
    FUN_00529740("Effect/Spark02.jpg",    0x497, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/Spark03.jpg",    0x498, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/smoke01.jpg",    0x4c4, 0x2601, 0x2900, 0, '\x01');
    FUN_00529bd0("Effect/smoke02.tga",    0x4c5, 0x2601, 0x2900, 0, '\x01');
    FUN_00529bd0("Effect/blood01.tga",    0x4b5, 0x2601, 0x2900, 0, '\x01');
    FUN_00529bd0("Effect/blood.tga",      0x4b6, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/Explotion01.jpg",0x4bf, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/DinoE.jpg",      0x4c0, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/Shiny01.jpg",    0x4ce, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/Shiny02.jpg",    0x4cf, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/Shiny03.jpg",    0x4d0, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/eye01.jpg",      0x4d1, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/ring.jpg",       0x4d2, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/Chrome01.jpg",   0x492, 0x2601, 0x2901, 0, '\x01');
    FUN_00529740("Effect/blur01.jpg",     0x48d, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Effect/bab2.jpg",       0x493, 0x2601, 0x2901, 0, '\x01');
    FUN_00529740("Effect/motion_blur.jpg",0x48e, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Effect/motion_blur_r.jpg",0x48f,0x2600,0x2900, 0, '\x01');
    FUN_00529740("Effect/lightning2.jpg", 0x4a7, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/Thunder01.jpg",  0x49c, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/Spark01.jpg",    0x4e9, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/JointThunder01.jpg",0x4e6,0x2601,0x2900,0,'\x01');
    FUN_00529740("Effect/JointSpirit01.jpg", 0x4e5,0x2601,0x2900,0,'\x01');
    FUN_00529740("Effect/JointLaser01.jpg",  0x4ea,0x2601,0x2900,0,'\x01');
    FUN_00529740("Effect/JointEnergy01.jpg", 0x4eb,0x2601,0x2900,0,'\x01');
    FUN_00529740("Effect/JointLaser02.jpg",  0x4ed,0x2601,0x2901,0,'\x01');
    FUN_00529740("Monster/iui03.jpg",     0x4f6, 0x2601, 0x2901, 0, '\x01');
    FUN_00529bd0("Monster/magic_H.tga",   0x4fb, 0x2601, 0x2901, 0, '\x01');
    FUN_00529bd0("Item/lower_14m.tga",    0x4fc, 0x2601, 0x2901, 0, '\x01');
    FUN_00529740("Skill/Skull.jpg",       0x4fa, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Effect/motion_blur_r2.jpg",0x4ee,0x2601,0x2901,0,'\x01');
    FUN_00529740("Effect/Fire04.jpg",     0x4df, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/Flare.jpg",      0x4e1, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/Chrome02.jpg",   0x4f2, 0x2600, 0x2900, 0, '\x01');
    FUN_00529740("Effect/flareBlue.jpg",  0x4fd, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/NSkill.jpg",     0x4fe, 0x2601, 0x2900, 0, '\x01');
    FUN_00529740("Effect/Flare02.jpg",    0x4e2, 0x2601, 0x2901, 0, '\x01');
    // Model loaders
    FUN_00506170(); FUN_00507610(); FUN_005079d0();
    FUN_00508d10(); FUN_0050b710(); FUN_0050eb80(); FUN_0050f030();
    // Data/BMD loaders (locale-sensitive)
    if (DAT_0055a7c4 == '\0') {
        crt_sprintf(local_64, "Data2/Local/item_%s.txt", (char*)DAT_0055a7a4);
        Item_LoadTextData(local_64);
        Item_SaveBMD("Data/Local/Item.bmd");
        crt_sprintf(local_64, "Data2/Local/skill_%s.txt", (char*)DAT_0055a7a4);
        Skill_LoadTextData(local_64);
        Skill_SaveBMD("Data/Local/Skill.bmd");
        Gate_LoadTextData("Data2/Gate.txt");
        Gate_SaveBMD("Data/Gate.bmd");
        crt_sprintf(local_64, "Data2/Local/Filter_%s.txt", (char*)DAT_0055a7a4);
        FUN_004799d0(local_64);
        FUN_00479a50("Data/Local/Filter.bmd");
        crt_sprintf(local_64, "Data2/Local/FilterName_%s.txt", (char*)DAT_0055a7a4);
        FUN_00479cf0(local_64);
        FUN_00479d70("Data/Local/FilterName.bmd");
        crt_sprintf(local_64, "Data2/Local/Dialog_%s.txt", (char*)DAT_0055a7a4);
        FUN_00479cf0(local_64);
        FUN_00479d70("Data/Local/FilterName.bmd");
    } else {
        Item_LoadBMD("Data/Local/Item.bmd");
        Skill_LoadBMD("Data/Local/Skill.bmd");
        Gate_LoadBMD("Data/Gate.bmd");
        FUN_00479b30("Data/Local/Filter.bmd");
        FUN_00479e50("Data/Local/FilterName.bmd");
        Dialog_LoadBMD("Data/Local/Dialog_Spn.bmd");  // cliente traducido: solo Dialog_{Eng,Por,Spn}.bmd
        FUN_00401040((int)(uintptr_t)DAT_00583d8c, "Data/Local/Quest.bmd");
    }
    crt_sprintf(local_64, "Data/Local/NPCName.txt");   // cliente: archivo sin sufijo de locale
    NPCName_LoadTextData(local_64);
    // Sound preloads (tail of OpenBasicData @ 0x00510F65-0x00510FC5).
    // IDs y filenames verificados contra IDA Hex-Rays 00510320_OpenBasicData.c
    // y los binary patches del companion project (Mu-linux-97K/Source/Client/
    // Main/Sound.cpp:162-166). Ghidra había inventado los nombres
    // iPickUp/iGet/iDrop/iEquip — no existen como archivos .wav en Data/Sound.
    FUN_00404a10(4,    "Data\\Sound\\iTitle.wav",        1, '\0');
    FUN_00404a10(0x1b, "Data\\Sound\\iButtonMove.wav",   2, '\0');  // 27
    FUN_00404a10(0x19, "Data\\Sound\\iButtonClick.wav",  1, '\0');  // 25
    FUN_00404a10(0x1a, "Data\\Sound\\iButtonError.wav",  1, '\0');  // 26
    FUN_00404a10(0x1c, "Data\\Sound\\iCreateWindow.wav", 1, '\0');  // 28
    FUN_00404a10(0x25, "Data\\Sound\\iRepair.wav",       1, '\0');  // 37
    FUN_00404a10(0x26, "Data\\Sound\\iWhisper.wav",      1, '\0');  // 38
}
// FUN_00511060 — implemented in src/Monster/Monster_Data.cpp
// GL_CaptureScreenshot — implemented in src/Render/GL_State.cpp
// GL_GetModelViewMatrix — implemented in src/Render/GL_State.cpp
