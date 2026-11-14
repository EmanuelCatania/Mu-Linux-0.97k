#include <cstdint>
// Monster_Data.cpp
// Monster_Data_Load @ 0x00511060
//
// Loads monster script / base data files.
// Only executed if DAT_083a410c != 0 (multiplayer / server-mode flag).
//
// Sub-functions:
//   Monster_LoadScriptTable — load monster script table (enable/disable state via DAT_0055a7c4)
//   Monster_ParseSetBase2 — parse and load "Data2/MonsterSetBase2.txt" monster definitions
//
// Globals:
//   DAT_083a410c   — if non-zero: server/party-enabled mode
//   DAT_0055a7c4   — loader phase flag (0 = pre-load, 1 = post-load)
//   s_Data2_MonsterSetBase2_txt (0x00561530) — filename literal

#include "stdafx.h"
#include "audio/SoundTable.h"

// IDA: Monster_Data_Load
void __cdecl Monster_LoadStartupData(void)
{
    if (DAT_083a410c != '\0') {
        DAT_0055a7c4 = 0;
        Monster_LoadScriptTable();           // load monster script table
        DAT_0055a7c4 = 1;
        if (DAT_083a410c != '\0') {
            Monster_ParseSetBase2(s_Data2_MonsterSetBase2_txt_00561530);  // load MonsterSetBase2.txt
        }
    }
    return;
}


// FUN_0050b510 @ 0x0050b510 — Monster_LoadScriptTable
//
// Monster_LoadScriptTable — loads the monster script/model table.
//
// Calls SetMaxTextures to initialize the table with 0x41a entries.
// Then, only in pre-load phase (DAT_0055a7c4 == 0), registers BMD model
// files from Data2/Monster/ for monster types 0x13d–0x14c and invokes
// OpenMonsterModel for monster IDs 0x32–0x3e to set up their sound+animation
// data.
//
// Globals:
//   DAT_0055a7c4               — loader phase (0 = pre-load)
//   s_Data2_Monster__0055e06c  — "Data2/Monster/" path prefix
//   s_swordbasic_smd_0055e07c  — "swordbasic.smd" filename

void Monster_LoadScriptTable(void)

{
  int iVar1;

  SetMaxTextures(0x41a);
  if (DAT_0055a7c4 == '\0') {
    iVar1 = 0x32;
    do {
      OpenMonsterModel(iVar1);
      iVar1 = iVar1 + 1;
    } while (iVar1 < 0x3f);
  }
  return;
}


// FUN_0047d020 @ 0x0047d020 — Monster_ParseSetBase2
//
// Monster_ParseSetBase2 — parses "Data2/MonsterSetBase2.txt" to
// populate the monster spawn table.
//
// Opens the file via crt_fopen, then reads records with TextParser_GetToken.
// For each record:
//   - field 0 (ftol) = monster type
//   - fields 1,2,3  = skipped
//   - field 4 (ftol) = spawn X
//   - field 5        = skipped
//   - field 6 (ftol) = spawn Y
//   - Stops on record type 2 or when the sentinel string (DAT_00559088)
//     matches TextParserTokenString.
// Creates the monster via CreateMonster and sets field +0x84 = ' '.
//
// Globals:
//   SMDFile_0  — file handle
//   DAT_07d7807c  — next spawn slot index
//   HeroKey  — max spawn slot (skips if equal)
//   DAT_00559088  — sentinel string "END"

void __cdecl Monster_ParseSetBase2(LPCSTR param_1)

{
  byte bVar1;
  int iVar2;
  byte *pbVar3;
  char *pcVar4;
  byte *pbVar5;
  bool bVar6;
  longlong lVar7;
  longlong lVar8;
  longlong lVar9;

  SMDFile_0 = (FILE *)crt_fopen(param_1,DAT_005580ac);
  if (SMDFile_0 != (FILE *)0x0) {
    TextParser_GetToken();
    while( true ) {
      iVar2 = TextParser_GetToken();
      lVar7 = (longlong)ParserTokenNumber;   // IDA sub_47D020: (std::int64_t)TokenNumber
      if (iVar2 == 2) break;
      if (iVar2 == 0) {
        pbVar5 = (byte*)TokenString;
        pbVar3 = (byte*)&DAT_00559088;
        do {
          bVar1 = *pbVar3;
          bVar6 = bVar1 < *pbVar5;
          if (bVar1 != *pbVar5) {
LAB_0047d096:
            iVar2 = (1 - (uint)bVar6) - (uint)(bVar6 != 0);
            goto LAB_0047d09b;
          }
          if (bVar1 == 0) break;
          bVar1 = pbVar3[1];
          bVar6 = bVar1 < pbVar5[1];
          if (bVar1 != pbVar5[1]) goto LAB_0047d096;
          pbVar3 = pbVar3 + 2;
          pbVar5 = pbVar5 + 2;
        } while (bVar1 != 0);
        iVar2 = 0;
LAB_0047d09b:
        if (iVar2 == 0) break;
      }
      TextParser_GetToken();
      TextParser_GetToken();
      TextParser_GetToken();
      lVar8 = (longlong)ParserTokenNumber;   // IDA sub_47D020: (std::int64_t)TokenNumber
      TextParser_GetToken();
      lVar9 = (longlong)ParserTokenNumber;   // IDA sub_47D020: (std::int64_t)TokenNumber
      TextParser_GetToken();
      if (DAT_07d7807c == HeroKey) {
        DAT_07d7807c = DAT_07d7807c + 1;
      }
      iVar2 = DAT_07d7807c;
      DAT_07d7807c = DAT_07d7807c + 1;
      pcVar4 = CreateMonster((uint)lVar7,(int)lVar8,(int)lVar9,iVar2,0);
      if (pcVar4 != (char *)0x0) {
        pcVar4[0x84] = ' ';
      }
    }
    crt_fclose(SMDFile_0);
  }
  return;
}


// OpenMonsterModel @ 0x005098c0
//
// Monster_SetupSoundAnim — registers sound and animation data for one
// monster type (param_1 = monster index 0x00–0x3f).
//
// Only runs when DAT_0055a7c4 == 0 (pre-load) OR when the monster has
// no existing animations (sound/anim counts == 0).
// Calls AccessModel to load the base monster script, then sets default
// animation speeds and special flags in the animation block at
// *(int*)(iVar2 + 0x30).
// Sounds come from [monster.N] in Content/audio/sound/sounds.toml and
// animation frame sets via Model_SetAnimationSlots.
//
// Globals:
//   DAT_0055a7c4  — loader phase flag
//   DAT_05828d58  — monster table base pointer

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl OpenMonsterModel(int param_1)

{
  int iVar1;
  intptr_t iVar2;
  intptr_t iVar3;
  float fVar4;
  int iVar5;

  iVar1 = param_1 + 0x10e;
  iVar2 = DAT_05828d58 + iVar1 * BMD_STRIDE;
  if (((DAT_0055a7c4 == '\0') || ((*(short *)(iVar2 + 0x26) < 1 && (*(short *)(iVar2 + 0x24) < 1))))
     && (AccessModel(iVar1,s_Data_Monster__0055ddf8,s_Monster_0055de08,param_1 + 1),
        *(short *)(iVar2 + 0x24) != 0)) {
    OpenTexture(iVar1,s_Monster__0055ddec,0x2600,'\x01');
    *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(4)) = 0x3e800000;
    *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(0x14)) = 0x3e4ccccd;
    *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(0x24)) = 0x3eae147b;
    *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(0x34)) = 0x3ea8f5c3;
    *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(0x44)) = 0x3ea8f5c3;
    *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(0x54)) = 0x3f000000;
    iVar5 = 0;
    *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(100)) = 0x3f0ccccd;
    *(undefined1 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(0x60)) = 1;
    do {
      if (param_1 == 3) {
        iVar3 = (intptr_t)AsModel(iVar2)->Actions;
        fVar4 = *(float *)(iVar3 + 4 + iVar5) * _DAT_00552a1c;
LAB_005099d1:
        *(float *)(iVar3 + 4 + iVar5) = fVar4;
      }
      else {
        if ((param_1 == 5) || (param_1 == 0x19)) {
          *(float *)((intptr_t)AsModel(iVar2)->Actions + 4 + iVar5) =
               *(float *)((intptr_t)AsModel(iVar2)->Actions + 4 + iVar5) * _DAT_00552928;
        }
        if ((param_1 == 0x25) || (param_1 == 0x2a)) {
          iVar3 = (intptr_t)AsModel(iVar2)->Actions;
          fVar4 = *(float *)(iVar3 + 4 + iVar5) * _DAT_005528b4;
          goto LAB_005099d1;
        }
      }
      iVar5 = iVar5 + (int)sizeof(ModelAnimationAction);
    } while (iVar5 < 6 * (int)sizeof(ModelAnimationAction));
    switch(param_1) {
    case 2:
      *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(0x24)) = 0x3f333333;
      break;
    case 6:
      *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(0x24)) = 0x3f19999a;
      break;
    case 8:
      *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(0x24)) = 0x3f333333;
      break;
    case 9:
      *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(0x24)) = 0x3f99999a;
      break;
    case 10:
      *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(0x24)) = 0x3e8f5c29;
      break;
    case 0xc:
    case 0x1c:
      *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(0x24)) = 0x3e99999a;
      break;
    case 0xd:
      *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(0x24)) = 0x3e8f5c29;
      break;
    case 0x11:
    case 0x15:
      *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(0x24)) = 0x3f000000;
      break;
    case 0x13:
      *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(0x24)) = 0x3f19999a;
      break;
    case 0x14:
      *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(0x24)) = 0x3ecccccd;
      break;
    case 0x27:
      *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(0x24)) = 0x3e6147ae;
      break;
    case 0x29:
      *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(0x24)) = 0x3e3851ec;
      break;
    case 0x2a:
      *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(0x34)) = 0x3eb33333;
      *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(0x44)) = 0x3eb33333;
      break;
    case 0x2c:
      *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(100)) = 0x3e99999a;
    }
    switch(param_1) {
    case 0xe:
    case 0x1f:
    case 0x27:
    case 0x28:
      *(undefined1 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(0x10)) = 1;
    }
    // Los sonidos de cada tipo salen de [monster.N] en Content/audio/sound/sounds.toml.
    SoundTable_LoadMonster(param_1);
    switch(param_1) {
    case 0:
    case 0x1e:
      Model_SetAnimationSlots(iVar1,0,1,2,3,4);
      *(undefined4 *)(DAT_05828d58 + BMD_STRIDE * 270 + 0x54 + BMD_FIELD_SHIFT + param_1 * BMD_STRIDE) = 0x14;
      return;
    case 1:
      Model_SetAnimationSlots(iVar1,5,6,7,8,9);
      *(undefined4 *)(DAT_05828d58 + BMD_STRIDE * 271 + 0x54 + BMD_FIELD_SHIFT) = 5;
      return;
    case 2:
      Model_SetAnimationSlots(iVar1,10,0xb,0xb,0xb,0xc);
      *(undefined4 *)(DAT_05828d58 + BMD_STRIDE * 272 + 0x54 + BMD_FIELD_SHIFT) = 7;
      return;
    case 3:
    case 0x1d:
    case 0x27:
      Model_SetAnimationSlots(iVar1,0xf,0x10,0x11,0x12,0x13);
      if (param_1 == 3) {
        *(undefined4 *)(DAT_05828d58 + BMD_STRIDE * 273 + 0x54 + BMD_FIELD_SHIFT) = 0x10;
        return;
      }
      if (param_1 == 0x1d) {
        *(undefined4 *)(DAT_05828d58 + BMD_STRIDE * 299 + 0x54 + BMD_FIELD_SHIFT) = 0x14;
        return;
      }
      *(undefined4 *)(DAT_05828d58 + BMD_STRIDE * 270 + 0x54 + BMD_FIELD_SHIFT + param_1 * BMD_STRIDE) = 0x13;
      return;
    case 4:
      Model_SetAnimationSlots(iVar1,0x14,0x15,0x16,0x17,0x18);
      *(undefined4 *)(DAT_05828d58 + BMD_STRIDE * 274 + 0x54 + BMD_FIELD_SHIFT) = 0x14;
      return;
    case 5:
      Model_SetAnimationSlots(iVar1,0x19,0x1a,0x1b,0x1c,0x1d);
      *(undefined4 *)(DAT_05828d58 + BMD_STRIDE * 275 + 0x54 + BMD_FIELD_SHIFT) = 0x14;
      return;
    case 6:
      Model_SetAnimationSlots(iVar1,0x1e,0x1f,0x1e,0x1f,0x1f);
      return;
    case 7:
      Model_SetAnimationSlots(iVar1,0x23,0x24,0x25,0x26,0x27);
      *(undefined4 *)(DAT_05828d58 + BMD_STRIDE * 277 + 0x54 + BMD_FIELD_SHIFT) = 0x14;
      return;
    case 8:
      Model_SetAnimationSlots(iVar1,0x20,0x21,0x21,0x21,0x22);
      *(undefined4 *)(DAT_05828d58 + BMD_STRIDE * 278 + 0x54 + BMD_FIELD_SHIFT) = 0x10;
      return;
    case 9:
      Model_SetAnimationSlots(iVar1,0xd,0xd,0xd,0xd,0xd);
      return;
    case 10:
      Model_SetAnimationSlots(iVar1,0x28,0x29,0x2a,0x2b,0x2c);
      *(undefined4 *)(DAT_05828d58 + BMD_STRIDE * 280 + 0x54 + BMD_FIELD_SHIFT) = 0x14;
      return;
    case 0xb:
      Model_SetAnimationSlots(iVar1,0x2d,0x2e,0x2f,0x30,0x31);
      *(undefined4 *)(DAT_05828d58 + BMD_STRIDE * 281 + 0x54 + BMD_FIELD_SHIFT) = 0x14;
      return;
    case 0xc:
    case 0xd:
      Model_SetAnimationSlots(0x11a,0x44,0x44,0x46,0x46,0x47);
      Model_SetAnimationSlots(0x11b,0x44,0x45,0x46,0x46,0x47);
      *(undefined4 *)(DAT_05828d58 + BMD_STRIDE * 282 + 0x54 + BMD_FIELD_SHIFT) = 0x14;
      *(undefined4 *)(DAT_05828d58 + BMD_STRIDE * 283 + 0x54 + BMD_FIELD_SHIFT) = 0x14;
      return;
    case 0xe:
      Model_SetAnimationSlots(iVar1,0xffff,0xffff,0x41,0x42,0x43);
      *(undefined4 *)(DAT_05828d58 + BMD_STRIDE * 284 + 0x54 + BMD_FIELD_SHIFT) = 0x14;
      *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(0x14)) = 0x3eb33333;
      return;
    case 0xf:
      Model_SetAnimationSlots(iVar1,0x32,0x33,0x32,0x32,0x34);
      *(undefined4 *)(DAT_05828d58 + BMD_STRIDE * 285 + 0x54 + BMD_FIELD_SHIFT) = 0x13;
      return;
    case 0x10:
      Model_SetAnimationSlots(iVar1,0x38,0x39,0x3a,0x3a,0x3b);
      return;
    case 0x11:
      Model_SetAnimationSlots(iVar1,0x35,0x35,0x37,0x37,0x37);
      return;
    case 0x12:
      Model_SetAnimationSlots(iVar1,0x3c,0x3d,0x3e,0x3f,0x40);
      *(undefined4 *)(DAT_05828d58 + BMD_STRIDE * 288 + 0x54 + BMD_FIELD_SHIFT) = 0x10;
      return;
    case 0x13:
      Model_SetAnimationSlots(iVar1,0x48,0x49,0x4a,0x4b,0x4c);
      *(undefined4 *)(DAT_05828d58 + BMD_STRIDE * 289 + 0x54 + BMD_FIELD_SHIFT) = 6;
      return;
    case 0x14:
      Model_SetAnimationSlots(iVar1,0x4d,0x4e,0x4f,0x50,0x51);
      return;
    case 0x15:
      Model_SetAnimationSlots(iVar1,0x52,0x52,0x53,0x53,0x54);
      *(undefined4 *)(DAT_05828d58 + BMD_STRIDE * 291 + 0x54 + BMD_FIELD_SHIFT) = 5;
      return;
    case 0x16:
      Model_SetAnimationSlots(iVar1,0x55,0x56,0x57,0x58,0x59);
      *(undefined4 *)(DAT_05828d58 + BMD_STRIDE * 292 + 0x54 + BMD_FIELD_SHIFT) = 6;
      return;
    case 0x17:
      Model_SetAnimationSlots(iVar1,0x5a,0x5b,0x5c,0x5d,0x5e);
      *(undefined4 *)(DAT_05828d58 + BMD_STRIDE * 293 + 0x54 + BMD_FIELD_SHIFT) = 6;
      return;
    case 0x18:
      Model_SetAnimationSlots(iVar1,0x5f,0x60,0x61,0x62,99);
      *(undefined4 *)(DAT_05828d58 + BMD_STRIDE * 294 + 0x54 + BMD_FIELD_SHIFT) = 0x10;
      return;
    case 0x19:
      Model_SetAnimationSlots(iVar1,100,0x65,0x66,0x67,0x68);
      *(undefined4 *)(DAT_05828d58 + BMD_STRIDE * 295 + 0x54 + BMD_FIELD_SHIFT) = 5;
      return;
    case 0x1a:
      Model_SetAnimationSlots(iVar1,0x69,0x69,0x6a,0x6a,0x6b);
      *(undefined4 *)(DAT_05828d58 + BMD_STRIDE * 296 + 0x54 + BMD_FIELD_SHIFT) = 6;
      return;
    case 0x1b:
      Model_SetAnimationSlots(iVar1,0x6c,0x6d,0x6e,0x6f,0x70);
      *(undefined4 *)(DAT_05828d58 + BMD_STRIDE * 297 + 0x54 + BMD_FIELD_SHIFT) = 6;
      *(undefined1 *)(DAT_05828d58 + BMD_STRIDE * 297 + 0x88 + BMD_FIELD_SHIFT) = 1;
      return;
    case 0x1c:
      Model_SetAnimationSlots(iVar1,0x71,0x72,0x73,0x74,0x75);
      *(undefined4 *)(DAT_05828d58 + BMD_STRIDE * 298 + 0x54 + BMD_FIELD_SHIFT) = 5;
      return;
    case 0x1f:
      Model_SetAnimationSlots(iVar1,0x7b,0x7b,0x7c,0x7c,0x7d);
      *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(0x34)) = 0x3f000000;
      *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(0x44)) = 0x3f333333;
      *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(0x14)) = 0x3f4ccccd;
      *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(0x74)) = 0x3f4ccccd;
      return;
    case 0x20:
      Model_SetAnimationSlots(iVar1,0x7e,0x7f,0x80,0x81,0x7f);
      *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(0x84)) = 0x3ecccccd;
      *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(0x94)) = 0x3ecccccd;
      *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(0x74)) = 0x3ecccccd;
      *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(0xa4)) = 0x3ecccccd;
      AsModel(iVar2)->BoneHead = 6;
      return;
    case 0x21:
      Model_SetAnimationSlots(iVar1,0x82,0x82,0x83,0x83,0x82);
      return;
    case 0x22:
      Model_SetAnimationSlots(iVar1,0x84,0x85,0x68,0x68,0x85);
      *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(0x34)) = 0x3f000000;
      *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(0x44)) = 0x3f000000;
      AsModel(iVar2)->BoneHead = 0x14;
      return;
    case 0x23:
      Model_SetAnimationSlots(iVar1,0x87,0x87,0x88,0x88,0x89);
      *(undefined4 *)(DAT_05828d58 + BMD_STRIDE * 305 + 0x54 + BMD_FIELD_SHIFT) = 0x13;
      return;
    case 0x24:
    case 0x28:
      Model_SetAnimationSlots(iVar1,0x8a,0x8b,0x8a,0x8b,0x8c);
      if (param_1 == 0x24) {
        *(undefined4 *)(DAT_05828d58 + BMD_STRIDE * 306 + 0x54 + BMD_FIELD_SHIFT) = 0x13;
        return;
      }
      *(undefined4 *)(DAT_05828d58 + BMD_STRIDE * 270 + 0x54 + BMD_FIELD_SHIFT + param_1 * BMD_STRIDE) = 6;
      return;
    case 0x25:
      Model_SetAnimationSlots(iVar1,0x8d,0x8d,0x8e,0x8e,0x8d);
      *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(0x34)) = 0x3e19999a;
      *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(0x44)) = 0x3e19999a;
      *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(100)) = 0x3e4ccccd;
      return;
    case 0x29:
      Model_SetAnimationSlots(iVar1,0x8f,0x8f,0x90,0x90,0x90);
      *(undefined4 *)(DAT_05828d58 + BMD_STRIDE * 311 + 0x54 + BMD_FIELD_SHIFT) = 3;
      return;
    case 0x2a:
      Model_SetAnimationSlots(iVar1,0x91,0x92,0x93,0x94,0x95);
      OpenJPG(s_Monster_bv01_2_jpg_0055cfb8,0x56b,0x2601,0x2901,0,'\x01');
      OpenJPG(s_Monster_bv02_2_jpg_0055cfa4,0x56c,0x2601,0x2901,0,'\x01');
      *(undefined4 *)(DAT_05828d58 + BMD_STRIDE * 312 + 0x54 + BMD_FIELD_SHIFT) = 0x14;
      return;
    case 0x2b:
      Model_SetAnimationSlots(iVar1,0x96,0x96,0x97,0x98,0x99);
      *(undefined4 *)(DAT_05828d58 + BMD_STRIDE * 313 + 0x54 + BMD_FIELD_SHIFT) = 7;
      return;
    case 0x2c:
      Model_SetAnimationSlots(iVar1,0x9a,0x9a,0x9b,0x9b,0x9c);
      *(undefined4 *)(DAT_05828d58 + BMD_STRIDE * 314 + 0x54 + BMD_FIELD_SHIFT) = 6;
      return;
    case 0x2d:
      Model_SetAnimationSlots(iVar1,0x9d,0x9e,0x9f,0x9f,0x9f);
      *(undefined4 *)(DAT_05828d58 + BMD_STRIDE * 315 + 0x54 + BMD_FIELD_SHIFT) = 6;
      return;
    case 0x2e:
      Model_SetAnimationSlots(iVar1,0xffff,0xffff,0xa2,0xa2,4);
      *(undefined4 *)(DAT_05828d58 + BMD_STRIDE * 316 + 0x54 + BMD_FIELD_SHIFT) = 7;
      return;
    case 0x2f:
      Model_SetAnimationSlots(iVar1,0x56,0x56,0xa1,0xa1,4);
      *(undefined4 *)(DAT_05828d58 + BMD_STRIDE * 317 + 0x54 + BMD_FIELD_SHIFT) = 0x14;
      return;
    case 0x30:
      Model_SetAnimationSlots(iVar1,0xa0,0xa4,0xffff,0xffff,0xa3);
      *(undefined4 *)(DAT_05828d58 + BMD_STRIDE * 318 + 0x54 + BMD_FIELD_SHIFT) = 0x14;
      return;
    case 0x31:
      *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(100)) = 0x3e6147ae;
      Model_SetAnimationSlots(iVar1,0xb1,0xb1,0xb2,0xb2,0xb3);
      return;
    case 0x32:
      *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(100)) = 0x3e6147ae;
      Model_SetAnimationSlots(iVar1,0xaf,0xaf,0xaf,0xaf,0xb0);
      return;
    case 0x33:
      *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(100)) = 0x3e6147ae;
      Model_SetAnimationSlots(iVar1,0xab,0xffff,0xac,0xac,0xad);
      return;
    case 0x34:
      OpenTGA(s_Monster_iui02_tga_0055cddc,0x1ed,0x2600,0x2900,0,'\x01');
      OpenTGA(s_Monster_iui03_tga_0055cdc8,0x1ef,0x2600,0x2900,0,'\x01');
      *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(100)) = 0x3e6147ae;
      Model_SetAnimationSlots(iVar1,0xb4,0xb4,0xb5,0xb5,0xb6);
      return;
    case 0x35:
      *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(100)) = 0x3e6147ae;
      Model_SetAnimationSlots(iVar1,0xa8,0xa8,0xa9,0xa9,0xaa);
      return;
    case 0x36:
      *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(100)) = 0x3e6147ae;
      Model_SetAnimationSlots(iVar1,0xa5,0xa5,0xa6,0xa6,0xa7);
      return;
    case 0x37:
      Model_SetAnimationSlots(iVar1,0xb7,0xb8,0xb9,0xb9,0xffff);
    case 0x38:
      *(undefined4 *)((intptr_t)AsModel(iVar2)->Actions + ModelActionByte(100)) = 0x3e6147ae;
      return;
    case 0x39:
      Model_SetAnimationSlots(iVar1,0xc0,0xffff,0xc2,0xffff,0xc1);
      break;
    case 0x3a:
      Model_SetAnimationSlots(iVar1,0xbc,0xffff,0xbe,0xffff,0xbf);
      return;
    case 0x3b:
      Model_SetAnimationSlots(iVar1,0xbc,0xffff,0xbe,0xffff,0xbd);
      return;
    case 0x3e:
      Model_SetAnimationSlots(iVar1,0xba,0xffff,0xffff,0xffff,0xbb);
      return;
    }
  }
  return;
}
