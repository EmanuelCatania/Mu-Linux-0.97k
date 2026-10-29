// Entity_Init.cpp
// Entity slot initialisation and position persistence.
//
// IDA: CreateHero (0x0045F930)
// SaveMonsters @ 0x0045fa20 — Monster_SaveSetBase
//
// Entity pool base: DAT_07abf5d0
// Stride: 0x394 bytes per slot
// Max slots: 0x59740 / 0x394 = 640

#include "stdafx.h"
#include "Platform/IFileSystem.h"


// CreateHero — CreateHero (Ghidra-confirmed signature)
//   CHARACTER* CreateHero(int Index, int Class, int Skin,
//                         float x, float y, float Rotate)
// Creates hero character slot at Index. Calls CreateCharacterPointer with
// Type=0x186 (hero placeholder). Sets light, position, class, skin, body
// parts, weapon/wing/helper slots to -1. Calls SetCharacterScale +
// SetPlayerStop.
// IDA: CreateHero (0x0045F930)
unsigned char * __cdecl
CreateHero(int Index, int Class, int Skin, float x, float y, float Rotate)
{
  unsigned char *puVar1;
  unsigned short uVar2;

  puVar1 = (unsigned char *)(DAT_07abf5d0 + Index * 0x394);
  CreateCharacterPointer(puVar1, 0x186, 0, 0, Rotate);

  // Object.Light[0..2] = 0.3f (0x3e99999a)
  *(unsigned int *)&AsCharacter(puVar1)->Object.Light[0] = 0x3e99999a;
  *(unsigned int *)&AsCharacter(puVar1)->Object.Light[1] = 0x3e99999a;
  *(unsigned int *)&AsCharacter(puVar1)->Object.Light[2] = 0x3e99999a;

  // Object.Position[0..1] = (x, y) — world coords for char-preview slots
  AsCharacter(puVar1)->Object.Position[0] = x;
  AsCharacter(puVar1)->Object.Position[1] = y;

  // c->Class (byte), c->Skin (byte)
  AsCharacter(puVar1)->Class = (unsigned char)Class;
  AsCharacter(puVar1)->Skin = (unsigned char)Skin;

  // BodyPart[1..5].Type and Weapon/Wing/Helper types
  uVar2 = (unsigned short)Class & 7;
  *(unsigned short *)&AsCharacter(puVar1)->BodyPart[1].Type = uVar2 + 0x390;  // BodyPart[1]
  *(unsigned short *)&AsCharacter(puVar1)->BodyPart[2].Type = uVar2 + 0x397;  // BodyPart[2]
  *(short *)&AsCharacter(puVar1)->Key = (short)Index;
  *(unsigned short *)&AsCharacter(puVar1)->BodyPart[5].Type   = uVar2 + 0x3ac;  // BodyPart[5]
  *(unsigned short *)&AsCharacter(puVar1)->BodyPart[3].Type = uVar2 + 0x39e;  // BodyPart[3]
  *(unsigned short *)&AsCharacter(puVar1)->BodyPart[4].Type = uVar2 + 0x3a5;  // BodyPart[4]
  *(unsigned short *)&AsCharacter(puVar1)->Weapon[0].Type = 0xffff;         // Weapon[0].Type
  *(unsigned short *)&AsCharacter(puVar1)->Weapon[1].Type = 0xffff;         // Weapon[1].Type
  *(unsigned short *)&AsCharacter(puVar1)->Wing.Type = 0xffff;         // Wing.Type
  *(unsigned short *)&AsCharacter(puVar1)->Helper.Type = 0xffff;         // Helper.Type

  SetCharacterScale(AsCharacter(puVar1));    // SetCharacterScale
  SetPlayerStop(AsCharacter(puVar1));    // SetPlayerStop
  return puVar1;
}


// IDA: SaveMonsters (0x0045FA20).
// Exporta posiciones de monstruos; las coordenadas se truncan antes de reducir a byte.
void __cdecl Monster_SaveSetBase(LPCSTR path)
{
  FILE* file = Platform::GetFileSystem().Open(path, "wt");
  // DESVIACION: si falla la apertura, no escribir mediante un FILE nulo.
  if (!file) return;
  fprintf(file, "2\n");
  for (int offset = 0; offset < 0x59740; offset += 0x394) {
    const unsigned char* entity = (const unsigned char*)(uintptr_t)DAT_07abf5d0 + offset;
    if (entity[0] && entity[0x84] == 32) {
      const unsigned char x = (unsigned char)(long long)(*(const float*)(entity + 0x10) * 0.0099999998);
      const unsigned char y = (unsigned char)(long long)(*(const float*)(entity + 0x14) * 0.0099999998);
      fprintf(file, "%4d %4d 30 %4d %4d -1\n", entity[0x2eb], gMapManager.GetCurrentMap(), x, y);
    }
  }
  fprintf(file, "end\n");
  fclose(file);
}
