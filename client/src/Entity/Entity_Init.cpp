// Entity_Init.cpp
// Entity slot initialisation and position persistence.
//
// FUN_0045f930 @ 0x0045f930 — Entity_InitSlot
// FUN_0045fa20 @ 0x0045fa20 — Entity_SavePositions
//
// Entity pool base: DAT_07abf5d0
// Stride: 0x394 bytes per slot
// Max slots: 0x59740 / 0x394 = 640

#include "stdafx.h"


// FUN_0045f930 — CreateHero (Ghidra-confirmed signature)
//   CHARACTER* CreateHero(int Index, int Class, int Skin,
//                         float x, float y, float Rotate)
// Creates hero character slot at Index. Calls CreateCharacterPointer with
// Type=0x186 (hero placeholder). Sets light, position, class, skin, body
// parts, weapon/wing/helper slots to -1. Calls SetCharacterScale +
// SetPlayerStop.
unsigned char * __cdecl
FUN_0045f930(int Index, int Class, int Skin, float x, float y, float Rotate)
{
  unsigned char *puVar1;
  unsigned short uVar2;

  puVar1 = (unsigned char *)(DAT_07abf5d0 + Index * 0x394);
  FUN_0045adc0(puVar1, 0x186, 0, 0, Rotate);

  // Object.Light[0..2] = 0.3f (0x3e99999a)
  *(unsigned int *)(puVar1 + 0xe8) = 0x3e99999a;
  *(unsigned int *)(puVar1 + 0xec) = 0x3e99999a;
  *(unsigned int *)(puVar1 + 0xf0) = 0x3e99999a;

  // Object.Position[0..1] = (x, y) — world coords for char-preview slots
  *(float *)(puVar1 + 0x10) = x;
  *(float *)(puVar1 + 0x14) = y;

  // c->Class = Class (byte), c->Skin = Skin (byte)
  puVar1[0x1bc] = (unsigned char)Class;
  puVar1[0x1bd] = (unsigned char)Skin;

  // BodyPart[1..5].Type and Weapon/Wing/Helper types
  uVar2 = (unsigned short)Class & 7;
  *(unsigned short *)(puVar1 + 0x1f8) = uVar2 + 0x390;  // BodyPart[1]
  *(unsigned short *)(puVar1 + 0x210) = uVar2 + 0x397;  // BodyPart[2]
  *(short         *)(puVar1 + 0x1dc) = (short)Index;
  *(unsigned short *)(puVar1 + 600)   = uVar2 + 0x3ac;  // BodyPart[5]
  *(unsigned short *)(puVar1 + 0x228) = uVar2 + 0x39e;  // BodyPart[3]
  *(unsigned short *)(puVar1 + 0x240) = uVar2 + 0x3a5;  // BodyPart[4]
  *(unsigned short *)(puVar1 + 0x270) = 0xffff;         // Weapon[0].Type
  *(unsigned short *)(puVar1 + 0x288) = 0xffff;         // Weapon[1].Type
  *(unsigned short *)(puVar1 + 0x2a0) = 0xffff;         // Wing.Type
  *(unsigned short *)(puVar1 + 0x2b8) = 0xffff;         // Helper.Type

  FUN_0045c050((int)puVar1);    // SetCharacterScale
  FUN_004430c0((int)puVar1);    // SetPlayerStop
  return puVar1;
}


// FUN_0045fa20 — Entity_SavePositions
// Writes entity positions to a file param_1 in a proprietary format.
// Opens file, iterates active entities (stride 0x394, count up to 0x59740/0x394),
// writes position with format string s__4d__4d_30__4d__4d__1_00559b58, closes file.
void __cdecl FUN_0045fa20(LPCSTR param_1)
{
  FILE *pFVar1;
  int iVar2;
  int iVar3;

  pFVar1 = (FILE *)FUN_0054173f(param_1,DAT_00559b74);
  FUN_00543274((int *)pFVar1,&DAT_00559b70);
  iVar2 = 0;
  iVar3 = DAT_07abf5d0;
  do {
    if ((*(char *)(iVar2 + iVar3) != '\0') && (*(char *)(iVar2 + 0x84 + iVar3) == ' ')) {
      __ftol();
      __ftol();
      FUN_00543274((int *)pFVar1,(byte *)s__4d__4d_30__4d__4d__1_00559b58);
      iVar3 = DAT_07abf5d0;
    }
    iVar2 = iVar2 + 0x394;
  } while (iVar2 < 0x59740);
  FUN_00543274((int *)pFVar1,&DAT_00559b50);
  FUN_0054150f(pFVar1);
  return;
}
