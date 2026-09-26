// Scene_AssetLoad.cpp
// Extracted from stubs_mouse_hover.cpp; IDA provenance comments retained.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// IDA: OpenBasicData @ 0x00510320 — Scene_LoadGameAssets
// Loads: cursor TGAs (slots 2-10), interface JPGs (0xE6-0x101, 0x500-0x507),
// effect JPGs/TGAs (0x47E-0x567), then invokes all model, data and sound preload routines.
void __cdecl Scene_LoadGameAssets(void) {
    CHAR local_64[100];
    // Cursor sprites
    OpenTGA("Interface/Cursor.tga",          2,  0x2601, 0x2900, 0, '\x01');
    OpenTGA("Interface/CursorPush.tga",       3,  0x2601, 0x2900, 0, '\x01');
    OpenTGA("Interface/CursorAttack.tga",     4,  0x2601, 0x2900, 0, '\x01');
    OpenTGA("Interface/CursorGet.tga",        5,  0x2601, 0x2900, 0, '\x01');
    OpenTGA("Interface/CursorTalk.tga",       6,  0x2601, 0x2900, 0, '\x01');
    OpenTGA("Interface/CursorRepair.tga",     7,  0x2601, 0x2900, 0, '\x01');
    OpenTGA("Interface/CursorLeanAgainst.tga",8,  0x2601, 0x2900, 0, '\x01');
    OpenTGA("Interface/CursorSitDown.tga",    9,  0x2601, 0x2900, 0, '\x01');
    OpenTGA("Interface/CursorDontMove.tga",  10,  0x2601, 0x2900, 0, '\x01');
    // Interface / HUD JPGs
    OpenJPG("Interface/Menu01_new.jpg",  0xe6, 0x2600, 0x2900, 0, '\x01');
    OpenJPG("Interface/Menu02.jpg",      0xe7, 0x2600, 0x2900, 0, '\x01');
    OpenJPG("Interface/Menu03_new.jpg",  0xe8, 0x2600, 0x2900, 0, '\x01');
    OpenTGA("Interface/Menu04.tga",      0xe9, 0x2600, 0x2900, 0, '\x01');
    OpenJPG("Interface/Menu_Red.jpg",    0xea, 0x2600, 0x2900, 0, '\x01');
    OpenJPG("Interface/Menu_Blue.jpg",   0xeb, 0x2600, 0x2900, 0, '\x01');
    OpenJPG("Interface/Menu_Party.jpg",  0xec, 0x2600, 0x2900, 0, '\x01');
    OpenJPG("Interface/Menu_Character.jpg",  0xed, 0x2600, 0x2900, 0, '\x01');
    OpenJPG("Interface/Menu_Inventory.jpg",  0xee, 0x2600, 0x2900, 0, '\x01');
    OpenJPG("Interface/dialogue2_1.jpg", 0xef, 0x2600, 0x2900, 0, '\x01');
    OpenJPG("Interface/Message_box.jpg", 0xf0, 0x2600, 0x2900, 0, '\x01');
    OpenJPG("Interface/ok.jpg",          0xf1, 0x2600, 0x2900, 0, '\x01');
    OpenJPG("Interface/ok2.jpg",         0xf2, 0x2600, 0x2900, 0, '\x01');
    OpenJPG("Interface/cancel.jpg",      0xf3, 0x2600, 0x2900, 0, '\x01');
    OpenJPG("Interface/cancel2.jpg",     0xf4, 0x2600, 0x2900, 0, '\x01');
    OpenJPG("Interface/Level_box.jpg",   0xf5, 0x2600, 0x2900, 0, '\x01');
    OpenJPG("Interface/level01.jpg",     0xf6, 0x2600, 0x2900, 0, '\x01');
    OpenJPG("Interface/Menu_Green.jpg",  0xf7, 0x2600, 0x2900, 0, '\x01');
    OpenJPG("Interface/dialogue1.jpg",   0xf8, 0x2600, 0x2900, 0, '\x01');
    OpenJPG("Interface/dialogue2.jpg",   0xf9, 0x2600, 0x2900, 0, '\x01');
    OpenJPG("Interface/Guild.jpg",       0xfa, 0x2600, 0x2900, 0, '\x01');
    OpenJPG("Interface/Message_box1.jpg",0xfb, 0x2600, 0x2900, 0, '\x01');
    OpenJPG("Interface/Message_box2.jpg",0xfc, 0x2600, 0x2900, 0, '\x01');
    // NIS (new interface system) frames
    OpenTGA("Interface/nis_rsframe.tga", 0x500, 0x2600, 0x2900, 0, '\x01');
    OpenJPG("Interface/nis_vframe.jpg",  0x501, 0x2600, 0x2900, 0, '\x01');
    OpenJPG("Interface/nis_bar.jpg",     0x502, 0x2600, 0x2900, 0, '\x01');
    OpenJPG("Interface/nis_back.jpg",    0x503, 0x2600, 0x2900, 0, '\x01');
    OpenJPG("Interface/nis_btnarrow.jpg",0x504, 0x2600, 0x2900, 0, '\x01');
    OpenJPG("Interface/nis_btnchat.jpg", 0x505, 0x2600, 0x2900, 0, '\x01');
    OpenJPG("Interface/nis_btnsize.jpg", 0x506, 0x2600, 0x2900, 0, '\x01');
    OpenJPG("Interface/nis_btnalpha.jpg",0x507, 0x2600, 0x2900, 0, '\x01');
    OpenJPG("Interface/bar.jpg",         0xfd, 0x2600, 0x2900, 0, '\x01');
    OpenJPG("Interface/back1.jpg",       0xfe, 0x2600, 0x2900, 0, '\x01');
    OpenJPG("Interface/back2.jpg",       0xff, 0x2600, 0x2900, 0, '\x01');
    OpenJPG("Interface/back3.jpg",      0x100, 0x2600, 0x2900, 0, '\x01');
    OpenJPG("Interface/Menu03_new_AG.jpg",0x101,0x2600, 0x2900, 0, '\x01');
    // Effect textures
    OpenJPG("Effect/Fire01.jpg",     0x4ab, 0x2601, 0x2900, 0, '\x01');
    OpenJPG("Effect/Fire02.jpg",     0x4ac, 0x2601, 0x2900, 0, '\x01');
    OpenJPG("Effect/Fire03.jpg",     0x4ad, 0x2601, 0x2900, 0, '\x01');
    OpenJPG("Effect/PoundingBall.jpg",0x567,0x2601, 0x2900, 0, '\x01');
    OpenJPG("Effect/fi01.jpg",       0x565, 0x2601, 0x2900, 0, '\x01');
    OpenTGA("Effect/fi02.tga",       0x566, 0x2601, 0x2900, 0, '\x01');
    OpenJPG("Effect/Flame01.jpg",    0x4b0, 0x2601, 0x2900, 0, '\x01');
    OpenJPG("Effect/flare01.jpg",    0x47e, 0x2601, 0x2900, 0, '\x01');
    OpenJPG("Effect/Magic_Ground2.jpg",0x4f0,0x2601,0x2900, 0, '\x01');
    OpenJPG("Effect/Magic_Circle1.jpg",0x4f1,0x2601,0x2900, 0, '\x01');
    OpenJPG("Effect/Spark02.jpg",    0x497, 0x2601, 0x2900, 0, '\x01');
    OpenJPG("Effect/Spark03.jpg",    0x498, 0x2601, 0x2900, 0, '\x01');
    OpenJPG("Effect/smoke01.jpg",    0x4c4, 0x2601, 0x2900, 0, '\x01');
    OpenTGA("Effect/smoke02.tga",    0x4c5, 0x2601, 0x2900, 0, '\x01');
    OpenTGA("Effect/blood01.tga",    0x4b5, 0x2601, 0x2900, 0, '\x01');
    OpenTGA("Effect/blood.tga",      0x4b6, 0x2601, 0x2900, 0, '\x01');
    OpenJPG("Effect/Explotion01.jpg",0x4bf, 0x2601, 0x2900, 0, '\x01');
    OpenJPG("Effect/DinoE.jpg",      0x4c0, 0x2601, 0x2900, 0, '\x01');
    OpenJPG("Effect/Shiny01.jpg",    0x4ce, 0x2601, 0x2900, 0, '\x01');
    OpenJPG("Effect/Shiny02.jpg",    0x4cf, 0x2601, 0x2900, 0, '\x01');
    OpenJPG("Effect/Shiny03.jpg",    0x4d0, 0x2601, 0x2900, 0, '\x01');
    OpenJPG("Effect/eye01.jpg",      0x4d1, 0x2601, 0x2900, 0, '\x01');
    OpenJPG("Effect/ring.jpg",       0x4d2, 0x2601, 0x2900, 0, '\x01');
    OpenJPG("Effect/Chrome01.jpg",   0x492, 0x2601, 0x2901, 0, '\x01');
    OpenJPG("Effect/blur01.jpg",     0x48d, 0x2600, 0x2900, 0, '\x01');
    OpenJPG("Effect/bab2.jpg",       0x493, 0x2601, 0x2901, 0, '\x01');
    OpenJPG("Effect/motion_blur.jpg",0x48e, 0x2600, 0x2900, 0, '\x01');
    OpenJPG("Effect/motion_blur_r.jpg",0x48f,0x2600,0x2900, 0, '\x01');
    OpenJPG("Effect/lightning2.jpg", 0x4a7, 0x2601, 0x2900, 0, '\x01');
    OpenJPG("Effect/Thunder01.jpg",  0x49c, 0x2601, 0x2900, 0, '\x01');
    OpenJPG("Effect/Spark01.jpg",    0x4e9, 0x2601, 0x2900, 0, '\x01');
    OpenJPG("Effect/JointThunder01.jpg",0x4e6,0x2601,0x2900,0,'\x01');
    OpenJPG("Effect/JointSpirit01.jpg", 0x4e5,0x2601,0x2900,0,'\x01');
    OpenJPG("Effect/JointLaser01.jpg",  0x4ea,0x2601,0x2900,0,'\x01');
    OpenJPG("Effect/JointEnergy01.jpg", 0x4eb,0x2601,0x2900,0,'\x01');
    OpenJPG("Effect/JointLaser02.jpg",  0x4ed,0x2601,0x2901,0,'\x01');
    OpenJPG("Monster/iui03.jpg",     0x4f6, 0x2601, 0x2901, 0, '\x01');
    OpenTGA("Monster/magic_H.tga",   0x4fb, 0x2601, 0x2901, 0, '\x01');
    OpenTGA("Item/lower_14m.tga",    0x4fc, 0x2601, 0x2901, 0, '\x01');
    OpenJPG("Skill/Skull.jpg",       0x4fa, 0x2600, 0x2900, 0, '\x01');
    OpenJPG("Effect/motion_blur_r2.jpg",0x4ee,0x2601,0x2901,0,'\x01');
    OpenJPG("Effect/Fire04.jpg",     0x4df, 0x2601, 0x2900, 0, '\x01');
    OpenJPG("Effect/Flare.jpg",      0x4e1, 0x2601, 0x2900, 0, '\x01');
    OpenJPG("Effect/Chrome02.jpg",   0x4f2, 0x2600, 0x2900, 0, '\x01');
    OpenJPG("Effect/flareBlue.jpg",  0x4fd, 0x2601, 0x2900, 0, '\x01');
    OpenJPG("Effect/NSkill.jpg",     0x4fe, 0x2601, 0x2900, 0, '\x01');
    OpenJPG("Effect/Flare02.jpg",    0x4e2, 0x2601, 0x2901, 0, '\x01');
    // Model loaders
    Model_LoadPlayerAndItemMeshes(); Model_LoadPlayerEquipmentTextures(); Model_LoadItemMeshes();
    Model_AssignItemTexturePrefixes(); Model_LoadSkillEffectAssets(); UI_LoadInterfaceTextures(); Sound_LoadGameSamples();
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
        Filter_LoadTextFile(local_64);
        Filter_SaveBMD("Data/Local/Filter.bmd");
        crt_sprintf(local_64, "Data2/Local/FilterName_%s.txt", (char*)DAT_0055a7a4);
        FilterName_LoadData(local_64);
        FilterName_SaveBMD("Data/Local/FilterName.bmd");
        crt_sprintf(local_64, "Data2/Local/Dialog_%s.txt", (char*)DAT_0055a7a4);
        FilterName_LoadData(local_64);
        FilterName_SaveBMD("Data/Local/FilterName.bmd");
    } else {
        Item_LoadBMD("Data/Local/Item.bmd");
        Skill_LoadBMD("Data/Local/Skill.bmd");
        Gate_LoadBMD("Data/Gate.bmd");
        OpenFilterFile("Data/Local/Filter.bmd");
        OpenNameFilterFile("Data/Local/FilterName.bmd");
        Dialog_LoadBMD("Data/Local/Dialog_Spn.bmd");  // cliente traducido: solo Dialog_{Eng,Por,Spn}.bmd
        CSQuest_OpenQuestScript((int)(uintptr_t)DAT_00583d8c, "Data/Local/Quest.bmd");
    }
    crt_sprintf(local_64, "Data/Local/NPCName.txt");   // cliente: archivo sin sufijo de locale
    NPCName_LoadTextData(local_64);
    // Sound preloads (tail of OpenBasicData @ 0x00510F65-0x00510FC5).
    // IDs y filenames verificados contra IDA Hex-Rays 00510320_OpenBasicData.c
    // y los binary patches del companion project (Mu-linux-97K/Source/Client/
    // Main/Sound.cpp:162-166). Ghidra había inventado los nombres
    // iPickUp/iGet/iDrop/iEquip — no existen como archivos .wav en Data/Sound.
    LoadWaveFile(4,    "Data\\Sound\\iTitle.wav",        1, '\0');
    LoadWaveFile(0x1b, "Data\\Sound\\iButtonMove.wav",   2, '\0');  // 27
    LoadWaveFile(0x19, "Data\\Sound\\iButtonClick.wav",  1, '\0');  // 25
    LoadWaveFile(0x1a, "Data\\Sound\\iButtonError.wav",  1, '\0');  // 26
    LoadWaveFile(0x1c, "Data\\Sound\\iCreateWindow.wav", 1, '\0');  // 28
    LoadWaveFile(0x25, "Data\\Sound\\iRepair.wav",       1, '\0');  // 37
    LoadWaveFile(0x26, "Data\\Sound\\iWhisper.wav",      1, '\0');  // 38
}
// Monster_LoadStartupData (IDA: Monster_Data_Load) — implemented in src/Monster/Monster_Data.cpp
// GL_CaptureScreenshot — implemented in src/Render/GL_State.cpp
// GL_GetModelViewMatrix — implemented in src/Render/GL_State.cpp
