// Scene_Resources.cpp
// Texture and model resource loading/unloading for login/account/char-select scenes.
//
// FUN_0050fcf0 @ 0x0050fcf0 — Scene_LoadAccountResources
// FUN_0050ff50 @ 0x0050ff50 — Scene_LoadCharSelectResources
// FUN_0050ff10 @ 0x0050ff10 — Scene_UnloadAccountResources
// FUN_005102c0 @ 0x005102c0 — Scene_UnloadCharSelectResources

#include "stdafx.h"

extern "C" { void DbgLogPublic(const char*); }

// FUN_0050fcf0 — Scene_LoadAccountResources
// Loads textures and 3D models for the Account Creation / New Character screens.
// Textures: Logo_0Account_new.tga, On_Botton.jpg, etc. (slots 0xc–0x14).
// Models: ship.smd (0x3c), logo objects (0xa0–0xa3).
// Preloads model data and activates rendering for slots 0x2601.
void FUN_0050fcf0(void)
{
  int iVar1;
  int iVar2;

  FUN_00529bd0(s_Logo_0Account_new_tga_005607a8,0xc,0x2600,0x2900,0,'\x01');
  FUN_00529740(s_Logo_0On_Botton_jpg_00560794,0xd,0x2600,0x2900,0,'\x01');
  FUN_00529740(s_Logo_0On_Botton2_jpg_0056077c,0xe,0x2600,0x2900,0,'\x01');
  FUN_00529740(s_Logo_0Text_Box_jpg_00560768,0xf,0x2600,0x2900,0,'\x01');
  FUN_00529bd0(s_Logo_0New_Account01_tga_00560750,0x10,0x2600,0x2900,0,'\x01');
  FUN_00529bd0(s_Logo_0New_Account02_tga_00560738,0x11,0x2600,0x2900,0,'\x01');
  FUN_00529740(s_Logo_0Box_jpg_00560728,0x12,0x2600,0x2900,0,'\x01');
  FUN_00529740(s_Interface_Progress_Back_jpg_0056070c,0x13,0x2600,0x2900,0,'\x01');
  FUN_00529740(s_Interface_Progress_jpg_005606f4,0x14,0x2600,0x2900,0,'\x01');
  if (DAT_0055a7c4 == '\0') {
    FUN_00505e90(0x3c,s_Data2_Object1__0055f120,s_ship_smd_0055ee54);
    FUN_00505e90(0xa0,s_Data2_Logo__005606dc,(const char*)&DAT_005606e8);
    FUN_00505e90(0xa1,s_Data2_Logo__005606dc,(const char*)&DAT_005606d0);
    FUN_00505e90(0xa2,s_Data2_Logo__005606dc,s_mu_smd_005606c8);
    FUN_00505e90(0xa3,s_Data2_Logo__005606dc,s_sun_smd_005606b4);
  }
  DbgLogPublic("Scene_LoadAccount: canary-A (before AccessModel Ship+Logos)");
  FUN_005060b0(0x3c,s_Data_Object1__0055f360,(const char*)&DAT_0055e834,1);
  iVar2 = 0xa0;
  do {
    FUN_005060b0(iVar2,s_Data_Logo__005606a0,(const char*)&DAT_005606ac,iVar2 + -0x9f);
    iVar1 = iVar2 + -0x9f;
    iVar2 = iVar2 + 1;
  } while (iVar1 < 4);
  DbgLogPublic("Scene_LoadAccount: canary-B (after AccessModel, before SetMaxTextures)");
  FUN_00505bd0(0x56a);
  DbgLogPublic("Scene_LoadAccount: canary-C (after SetMaxTextures, before OpenTexture Ship)");
  FUN_00505c80(0x3c,s_Object1__0055f354,0x2601,'\x01');
  DbgLogPublic("Scene_LoadAccount: canary-D (after OpenTexture Ship, before Logos)");
  iVar2 = 0;
  do {
    FUN_00505c80(iVar2 + 0xa0,s_Logo__00560698,0x2601,'\x01');
    iVar2 = iVar2 + 1;
  } while (iVar2 < 4);
  DbgLogPublic("Scene_LoadAccount: canary-E (after OpenTexture Logos, returning)");
  return;
}


// FUN_0050ff50 — Scene_LoadCharSelectResources
// Loads textures and 3D models for the Character Selection screen.
// Textures: Logo_Interface01–04.tga, New_Character201.tga, etc. (slots 0xc–0x1d).
// Models: main.smd (0xa4), warrior/fairy/dark/etc. (0xaa–0xad).
// Also sets scale for character preview models (0xbc–0x2f0) from DAT_05828d58.
void FUN_0050ff50(void)
{
  int iVar1;
  int iVar2;

  FUN_00529bd0(s_Logo_Interface01_tga_00560a4c,0xc,0x2600,0x2900,0,'\x01');
  FUN_00529bd0(s_Logo_Interface02_tga_00560a34,0xd,0x2600,0x2900,0,'\x01');
  FUN_00529bd0(s_Logo_Interface03_tga_00560a1c,0xe,0x2600,0x2900,0,'\x01');
  FUN_00529bd0(s_Logo_Interface04_tga_00560a04,0xf,0x2600,0x2900,0,'\x01');
  FUN_00529bd0(s_Logo_New_Character201_tga_005609e8,0x10,0x2600,0x2900,0,'\x01');
  FUN_00529740(s_Logo_New_Character202_jpg_005609cc,0x11,0x2600,0x2900,0,'\x01');
  FUN_00529bd0(s_Logo_Delete01_tga_005609b8,0x12,0x2600,0x2900,0,'\x01');
  FUN_00529740(s_Logo_Delete02_jpg_005609a4,0x13,0x2600,0x2900,0,'\x01');
  FUN_00529bd0(s_Logo_Ok01_tga_00560994,0x14,0x2600,0x2900,0,'\x01');
  FUN_00529740(s_Logo_Ok02_jpg_00560984,0x15,0x2600,0x2900,0,'\x01');
  FUN_00529bd0(s_Logo_New_Character01_tga_00560968,0x16,0x2600,0x2900,0,'\x01');
  FUN_00529bd0(s_Logo_New_Character02_tga_0056094c,0x17,0x2600,0x2900,0,'\x01');
  FUN_00529740(s_Logo_New_Character_Cancel_jpg_0056092c,0x18,0x2600,0x2900,0,'\x01');
  FUN_00529740(s_Logo_New_Character_Ok_jpg_00560910,0x19,0x2600,0x2900,0,'\x01');
  FUN_00529740(s_Logo_New_Character001_jpg_005608f4,0x1a,0x2600,0x2900,0,'\x01');
  FUN_00529740(s_Logo_New_Character002_jpg_005608d8,0x1b,0x2600,0x2900,0,'\x01');
  FUN_00529740(s_Logo_New_Character003_jpg_005608bc,0x1c,0x2600,0x2900,0,'\x01');
  FUN_00529740(s_Logo_New_Character004_jpg_005608a0,0x1d,0x2600,0x2900,0,'\x01');
  if (DAT_0055a7c4 == '\0') {
    FUN_00505e90(0xa4,s_Data2_Logo__005606dc,s_main_smd_00560894);
    FUN_00505e90(0xaa,s_Data2_Logo__005606dc,(const char*)&DAT_0056085c);
    FUN_00505e90(0xab,s_Data2_Logo__005606dc,s_warrior_smd_00560828);
    FUN_00505e90(0xac,s_Data2_Logo__005606dc,s_fairy_smd_0055c438);
    FUN_00505e90(0xad,s_Data2_Logo__005606dc,(const char*)&DAT_005607c8);
  }
  FUN_005060b0(0xa4,s_Data_Logo__005606a0,(const char*)&DAT_005606ac,5);
  iVar2 = 0xaa;
  do {
    FUN_005060b0(iVar2,s_Data_Logo__005606a0,(const char*)&DAT_005607c0,iVar2 + -0xa9);
    iVar1 = iVar2 + -0xa9;
    iVar2 = iVar2 + 1;
  } while (iVar1 < 4);
  FUN_00505bd0(0x56a);
  FUN_00505c80(0xa4,s_Logo__00560698,0x2601,'\x01');
  iVar2 = 0;
  do {
    FUN_00505c80(iVar2 + 0xaa,s_Logo__00560698,0x2600,'\x01');
    iVar2 = iVar2 + 1;
  } while (iVar2 < 4);
  iVar2 = 0;
  do {
    iVar1 = iVar2 + 0xbc;
    *(undefined4 *)(*(int *)(iVar2 + 0x7d08 + DAT_05828d58) + 4) = 0x3e99999a;
    *(undefined4 *)(*(int *)(iVar2 + 0x7d08 + DAT_05828d58) + 0x14) = 0x3e99999a;
    iVar2 = iVar1;
  } while (iVar1 < 0x2f0);
  return;
}


// FUN_0050ff10 — Scene_UnloadAccountResources
// Frees texture slots 0xc–0x14 and 0x56a–0x5a9 used by the account scene,
// then calls FUN_004ffd50 (clear model cache) and FUN_0045abb0(-1) (reset entities).
void FUN_0050ff10(void)
{
  int iVar1;

  iVar1 = 0;
  do {
    FUN_0052a050(iVar1 + 0xc);
    iVar1 = iVar1 + 1;
  } while (iVar1 < 9);
  iVar1 = 0x56a;
  do {
    FUN_0052a050(iVar1);
    iVar1 = iVar1 + 1;
  } while (iVar1 < 0x5aa);
  FUN_004ffd50();
  FUN_0045abb0(-1);
  return;
}


// FUN_005102c0 — Scene_UnloadCharSelectResources
// Frees texture slots 0xc–0x1d and 0x56a–0x5a9 used by the char-select scene,
// clears model data for slots 0x7cd8–0x7fc8 (stride 0xbc),
// then calls FUN_004ffd50 and FUN_0045abb0(-1).
void FUN_005102c0(void)
{
  int iVar1;

  iVar1 = 0;
  do {
    FUN_0052a050(iVar1 + 0xc);
    iVar1 = iVar1 + 1;
  } while (iVar1 < 0x12);
  iVar1 = 0x56a;
  do {
    FUN_0052a050(iVar1);
    iVar1 = iVar1 + 1;
  } while (iVar1 < 0x5aa);
  FUN_004ffd50();
  iVar1 = 0x7cd8;
  do {
    FUN_00442090(DAT_05828d58 + iVar1);
    iVar1 = iVar1 + 0xbc;
  } while (iVar1 < 0x7fc8);
  FUN_0045abb0(-1);
  return;
}
