// Monster_Data.cpp
// FUN_00511060 @ 0x00511060
//
// Loads monster script / base data files.
// Only executed if DAT_083a410c != 0 (multiplayer / server-mode flag).
//
// Sub-functions:
//   FUN_0050b510 — load monster script table (enable/disable state via DAT_0055a7c4)
//   FUN_0047d020 — parse and load "Data2/MonsterSetBase2.txt" monster definitions
//
// Globals:
//   DAT_083a410c   — if non-zero: server/party-enabled mode
//   DAT_0055a7c4   — loader phase flag (0 = pre-load, 1 = post-load)
//   s_Data2_MonsterSetBase2_txt (0x00561530) — filename literal

#include "stdafx.h"

void __cdecl FUN_00511060(void)
{
    if (DAT_083a410c != '\0') {
        DAT_0055a7c4 = 0;
        FUN_0050b510();           // load monster script table
        DAT_0055a7c4 = 1;
        if (DAT_083a410c != '\0') {
            FUN_0047d020(s_Data2_MonsterSetBase2_txt_00561530);  // load MonsterSetBase2.txt
        }
    }
    return;
}


// FUN_0050b510 @ 0x0050b510
//
// Monster_LoadScriptTable — loads the monster script/model table.
//
// Calls FUN_00505bd0 to initialize the table with 0x41a entries.
// Then, only in pre-load phase (DAT_0055a7c4 == 0), registers BMD model
// files from Data2/Monster/ for monster types 0x13d–0x14c and invokes
// FUN_005098c0 for monster IDs 0x32–0x3e to set up their sound+animation
// data.
//
// Globals:
//   DAT_0055a7c4               — loader phase (0 = pre-load)
//   s_Data2_Monster__0055e06c  — "Data2/Monster/" path prefix
//   s_swordbasic_smd_0055e07c  — "swordbasic.smd" filename

void FUN_0050b510(void)

{
  int iVar1;

  FUN_00505bd0(0x41a);
  if (DAT_0055a7c4 == '\0') {
    FUN_00505e90((int)0x13d,s_Data2_Monster__0055e06c,s_swordbasic_smd_0055e07c);
    FUN_00505e90((int)0x147,s_Data2_Monster__0055e06c,&DAT_0055dff0);
    FUN_00505e90((int)0x148,s_Data2_Monster__0055e06c,&DAT_0055df70);
    FUN_00505e90((int)0x149,s_Data2_Monster__0055e06c,&DAT_0055dee0);
    FUN_00505e90((int)0x14a,s_Data2_Monster__0055e06c,&DAT_0055dec0);
    FUN_00505e90((int)0x14b,s_Data2_Monster__0055e06c,&DAT_0055de84);
    FUN_00505e90((int)0x14c,s_Data2_Monster__0055e06c,&DAT_0055de10);
    iVar1 = 0x32;
    do {
      FUN_005098c0(iVar1);
      iVar1 = iVar1 + 1;
    } while (iVar1 < 0x3f);
  }
  return;
}


// FUN_0047d020 @ 0x0047d020
//
// Monster_ParseSetBase2 — parses "Data2/MonsterSetBase2.txt" to
// populate the monster spawn table.
//
// Opens the file via FUN_0054173f, then reads records with FUN_0047a1f0.
// For each record:
//   - field 0 (ftol) = monster type
//   - fields 1,2,3  = skipped
//   - field 4 (ftol) = spawn X
//   - field 5        = skipped
//   - field 6 (ftol) = spawn Y
//   - Stops on record type 2 or when the sentinel string (DAT_00559088)
//     matches DAT_07cf1ef0.
// Creates the monster via FUN_0045ccf0 and sets field +0x84 = ' '.
//
// Globals:
//   DAT_07d7806c  — file handle
//   DAT_07d7807c  — next spawn slot index
//   DAT_05826cac  — max spawn slot (skips if equal)
//   DAT_00559088  — sentinel string "END"

void __cdecl FUN_0047d020(LPCSTR param_1)

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

  DAT_07d7806c = (FILE *)FUN_0054173f(param_1,DAT_005580ac);
  if (DAT_07d7806c != (FILE *)0x0) {
    FUN_0047a1f0();
    while( true ) {
      iVar2 = FUN_0047a1f0();
      lVar7 = (longlong)_DAT_083a40f8;   // IDA sub_47D020: (__int64)TokenNumber
      if (iVar2 == 2) break;
      if (iVar2 == 0) {
        pbVar5 = (byte*)&DAT_07cf1ef0;
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
      FUN_0047a1f0();
      FUN_0047a1f0();
      FUN_0047a1f0();
      lVar8 = (longlong)_DAT_083a40f8;   // IDA sub_47D020: (__int64)TokenNumber
      FUN_0047a1f0();
      lVar9 = (longlong)_DAT_083a40f8;   // IDA sub_47D020: (__int64)TokenNumber
      FUN_0047a1f0();
      if (DAT_07d7807c == DAT_05826cac) {
        DAT_07d7807c = DAT_07d7807c + 1;
      }
      iVar2 = DAT_07d7807c;
      DAT_07d7807c = DAT_07d7807c + 1;
      pcVar4 = FUN_0045ccf0((uint)lVar7,(int)lVar8,(int)lVar9,iVar2,0);
      if (pcVar4 != (char *)0x0) {
        pcVar4[0x84] = ' ';
      }
    }
    FUN_0054150f(DAT_07d7806c);
  }
  return;
}


// FUN_005098c0 @ 0x005098c0
//
// Monster_SetupSoundAnim — registers sound and animation data for one
// monster type (param_1 = monster index 0x00–0x3f).
//
// Only runs when DAT_0055a7c4 == 0 (pre-load) OR when the monster has
// no existing animations (sound/anim counts == 0).
// Calls FUN_005060b0 to load the base monster script, then sets default
// animation speeds and special flags in the animation block at
// *(int*)(iVar2 + 0x30).
// Sound files are registered via FUN_00404a10 and animation frame sets
// via FUN_00509810.
//
// Globals:
//   DAT_0055a7c4  — loader phase flag
//   DAT_05828d58  — monster table base pointer

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __cdecl FUN_005098c0(int param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  float fVar4;
  int iVar5;

  iVar1 = param_1 + 0x10e;
  iVar2 = DAT_05828d58 + iVar1 * 0xbc;
  if (((DAT_0055a7c4 == '\0') || ((*(short *)(iVar2 + 0x26) < 1 && (*(short *)(iVar2 + 0x24) < 1))))
     && (FUN_005060b0(iVar1,s_Data_Monster__0055ddf8,s_Monster_0055de08,param_1 + 1),
        *(short *)(iVar2 + 0x24) != 0)) {
    FUN_00505c80(iVar1,s_Monster__0055ddec,0x2600,'\x01');
    *(undefined4 *)(*(int *)(iVar2 + 0x30) + 4) = 0x3e800000;
    *(undefined4 *)(*(int *)(iVar2 + 0x30) + 0x14) = 0x3e4ccccd;
    *(undefined4 *)(*(int *)(iVar2 + 0x30) + 0x24) = 0x3eae147b;
    *(undefined4 *)(*(int *)(iVar2 + 0x30) + 0x34) = 0x3ea8f5c3;
    *(undefined4 *)(*(int *)(iVar2 + 0x30) + 0x44) = 0x3ea8f5c3;
    *(undefined4 *)(*(int *)(iVar2 + 0x30) + 0x54) = 0x3f000000;
    iVar5 = 0;
    *(undefined4 *)(*(int *)(iVar2 + 0x30) + 100) = 0x3f0ccccd;
    *(undefined1 *)(*(int *)(iVar2 + 0x30) + 0x60) = 1;
    do {
      if (param_1 == 3) {
        iVar3 = *(int *)(iVar2 + 0x30);
        fVar4 = *(float *)(iVar3 + 4 + iVar5) * _DAT_00552a1c;
LAB_005099d1:
        *(float *)(iVar3 + 4 + iVar5) = fVar4;
      }
      else {
        if ((param_1 == 5) || (param_1 == 0x19)) {
          *(float *)(*(int *)(iVar2 + 0x30) + 4 + iVar5) =
               *(float *)(*(int *)(iVar2 + 0x30) + 4 + iVar5) * _DAT_00552928;
        }
        if ((param_1 == 0x25) || (param_1 == 0x2a)) {
          iVar3 = *(int *)(iVar2 + 0x30);
          fVar4 = *(float *)(iVar3 + 4 + iVar5) * _DAT_005528b4;
          goto LAB_005099d1;
        }
      }
      iVar5 = iVar5 + 0x10;
    } while (iVar5 < 0x60);
    switch(param_1) {
    case 2:
      *(undefined4 *)(*(int *)(iVar2 + 0x30) + 0x24) = 0x3f333333;
      break;
    case 6:
      *(undefined4 *)(*(int *)(iVar2 + 0x30) + 0x24) = 0x3f19999a;
      break;
    case 8:
      *(undefined4 *)(*(int *)(iVar2 + 0x30) + 0x24) = 0x3f333333;
      break;
    case 9:
      *(undefined4 *)(*(int *)(iVar2 + 0x30) + 0x24) = 0x3f99999a;
      break;
    case 10:
      *(undefined4 *)(*(int *)(iVar2 + 0x30) + 0x24) = 0x3e8f5c29;
      break;
    case 0xc:
    case 0x1c:
      *(undefined4 *)(*(int *)(iVar2 + 0x30) + 0x24) = 0x3e99999a;
      break;
    case 0xd:
      *(undefined4 *)(*(int *)(iVar2 + 0x30) + 0x24) = 0x3e8f5c29;
      break;
    case 0x11:
    case 0x15:
      *(undefined4 *)(*(int *)(iVar2 + 0x30) + 0x24) = 0x3f000000;
      break;
    case 0x13:
      *(undefined4 *)(*(int *)(iVar2 + 0x30) + 0x24) = 0x3f19999a;
      break;
    case 0x14:
      *(undefined4 *)(*(int *)(iVar2 + 0x30) + 0x24) = 0x3ecccccd;
      break;
    case 0x27:
      *(undefined4 *)(*(int *)(iVar2 + 0x30) + 0x24) = 0x3e6147ae;
      break;
    case 0x29:
      *(undefined4 *)(*(int *)(iVar2 + 0x30) + 0x24) = 0x3e3851ec;
      break;
    case 0x2a:
      *(undefined4 *)(*(int *)(iVar2 + 0x30) + 0x34) = 0x3eb33333;
      *(undefined4 *)(*(int *)(iVar2 + 0x30) + 0x44) = 0x3eb33333;
      break;
    case 0x2c:
      *(undefined4 *)(*(int *)(iVar2 + 0x30) + 100) = 0x3e99999a;
    }
    switch(param_1) {
    case 0xe:
    case 0x1f:
    case 0x27:
    case 0x28:
      *(undefined1 *)(*(int *)(iVar2 + 0x30) + 0x10) = 1;
    }
    switch(param_1) {
    case 0:
    case 0x1e:
      FUN_00404a10(0xaa, "Data\\Sound\\mBull1.wav",2,'\x01');
      FUN_00404a10(0xab, "Data\\Sound\\mBull2.wav",2,'\x01');
      FUN_00404a10(0xac, "Data\\Sound\\mBullAttack1.wav",2,'\x01');
      FUN_00404a10(0xad, "Data\\Sound\\mBullAttack2.wav",2,'\x01');
      FUN_00404a10(0xae, "Data\\Sound\\mBullDie.wav",2,'\x01');
      FUN_00509810(iVar1,0,1,2,3,4);
      *(undefined4 *)(DAT_05828d58 + 0xc69c + param_1 * 0xbc) = 0x14;
      return;
    case 1:
      FUN_00404a10(0xaf, "Data\\Sound\\mHound1.wav",2,'\x01');
      FUN_00404a10(0xb0, "Data\\Sound\\mHound2.wav",2,'\x01');
      FUN_00404a10(0xb1, "Data\\Sound\\mHoundAttack1.wav",2,'\x01');
      FUN_00404a10(0xb2, "Data\\Sound\\mHoundAttack2.wav",2,'\x01');
      FUN_00404a10(0xb3, "Data\\Sound\\mHoundDie.wav",2,'\x01');
      FUN_00509810(iVar1,5,6,7,8,9);
      *(undefined4 *)(DAT_05828d58 + 0xc758) = 5;
      return;
    case 2:
      FUN_00404a10(0xb4, "Data\\Sound\\mBudge1.wav",2,'\x01');
      FUN_00404a10(0xb5, "Data\\Sound\\mBudgeAttack1.wav",2,'\x01');
      FUN_00404a10(0xb6, "Data\\Sound\\mBudgeDie.wav",2,'\x01');
      FUN_00509810(iVar1,10,0xb,0xb,0xb,0xc);
      *(undefined4 *)(DAT_05828d58 + 0xc814) = 7;
      return;
    case 3:
    case 0x1d:
    case 0x27:
      FUN_00404a10(0xb9, "Data\\Sound\\mDarkKnight1.wav",2,'\x01');
      FUN_00404a10(0xba, "Data\\Sound\\mDarkKnight2.wav",2,'\x01');
      FUN_00404a10(0xbb, "Data\\Sound\\mDarkKnightAttack1.wav",2,'\x01');
      FUN_00404a10(0xbc, "Data\\Sound\\mDarkKnightAttack2.wav",2,'\x01');
      FUN_00404a10(0xbd, "Data\\Sound\\mDarkKnightDie.wav",2,'\x01');
      FUN_00509810(iVar1,0xf,0x10,0x11,0x12,0x13);
      if (param_1 == 3) {
        *(undefined4 *)(DAT_05828d58 + 0xc8d0) = 0x10;
        return;
      }
      if (param_1 == 0x1d) {
        *(undefined4 *)(DAT_05828d58 + 0xdbe8) = 0x14;
        return;
      }
      *(undefined4 *)(DAT_05828d58 + 0xc69c + param_1 * 0xbc) = 0x13;
      return;
    case 4:
      FUN_00404a10(0xbe, "Data\\Sound\\mWizard1.wav",2,'\x01');
      FUN_00404a10(0xbf, "Data\\Sound\\mWizard2.wav",2,'\x01');
      FUN_00404a10(0xc0, "Data\\Sound\\mWizardAttack1.wav",2,'\x01');
      FUN_00404a10(0xc1, "Data\\Sound\\mWizardAttack2.wav",2,'\x01');
      FUN_00404a10(0xc2, "Data\\Sound\\mWizardDie.wav",2,'\x01');
      FUN_00509810(iVar1,0x14,0x15,0x16,0x17,0x18);
      *(undefined4 *)(DAT_05828d58 + 0xc98c) = 0x14;
      return;
    case 5:
      FUN_00404a10(0xc3, "Data\\Sound\\mGiant1.wav",2,'\x01');
      FUN_00404a10(0xc4, "Data\\Sound\\mGiant2.wav",2,'\x01');
      FUN_00404a10(0xc5, "Data\\Sound\\mGiantAttack1.wav",2,'\x01');
      FUN_00404a10(0xc6, "Data\\Sound\\mGiantAttack2.wav",2,'\x01');
      FUN_00404a10(199, "Data\\Sound\\mGiantDie.wav",2,'\x01');
      FUN_00509810(iVar1,0x19,0x1a,0x1b,0x1c,0x1d);
      *(undefined4 *)(DAT_05828d58 + 0xca48) = 0x14;
      return;
    case 6:
      FUN_00404a10(200, "Data\\Sound\\mLarva1.wav",2,'\x01');
      FUN_00404a10(0xc9, "Data\\Sound\\mLarva2.wav",2,'\x01');
      FUN_00509810(iVar1,0x1e,0x1f,0x1e,0x1f,0x1f);
      return;
    case 7:
      FUN_00404a10(0xcd, "Data\\Sound\\mGhost1.wav",2,'\x01');
      FUN_00404a10(0xce, "Data\\Sound\\mGhost2.wav",2,'\x01');
      FUN_00404a10(0xcf, "Data\\Sound\\mGhostAttack1.wav",2,'\x01');
      FUN_00404a10(0xd0, "Data\\Sound\\mGhostAttack2.wav",2,'\x01');
      FUN_00404a10(0xd1, "Data\\Sound\\mGhostDie.wav",2,'\x01');
      FUN_00509810(iVar1,0x23,0x24,0x25,0x26,0x27);
      *(undefined4 *)(DAT_05828d58 + 0xcbc0) = 0x14;
      return;
    case 8:
      FUN_00404a10(0xca, "Data\\Sound\\mHellSpider1.wav",2,'\x01');
      FUN_00404a10(0xcb, "Data\\Sound\\mHellSpiderAttack1.wav",2,'\x01');
      FUN_00404a10(0xcc, "Data\\Sound\\mHellSpiderDie.wav",2,'\x01');
      FUN_00509810(iVar1,0x20,0x21,0x21,0x21,0x22);
      *(undefined4 *)(DAT_05828d58 + 0xcc7c) = 0x10;
      return;
    case 9:
      FUN_00404a10(0xb7, "Data\\Sound\\mSpider1.wav",2,'\x01');
      FUN_00509810(iVar1,0xd,0xd,0xd,0xd,0xd);
      return;
    case 10:
      FUN_00404a10(0xd2, "Data\\Sound\\mOgre1.wav",2,'\x01');
      FUN_00404a10(0xd3, "Data\\Sound\\mOgre2.wav",2,'\x01');
      FUN_00404a10(0xd4, "Data\\Sound\\mOgreAttack1.wav",2,'\x01');
      FUN_00404a10(0xd5, "Data\\Sound\\mOgreAttack2.wav",2,'\x01');
      FUN_00404a10(0xd6, "Data\\Sound\\mOgreDie.wav",2,'\x01');
      FUN_00509810(iVar1,0x28,0x29,0x2a,0x2b,0x2c);
      *(undefined4 *)(DAT_05828d58 + 0xcdf4) = 0x14;
      return;
    case 0xb:
      FUN_00404a10(0xd7, "Data\\Sound\\mGorgon1.wav",2,'\x01');
      FUN_00404a10(0xd8, "Data\\Sound\\mGorgon2.wav",2,'\x01');
      FUN_00404a10(0xd9, "Data\\Sound\\mGorgonAttack1.wav",2,'\x01');
      FUN_00404a10(0xda, "Data\\Sound\\mGorgonAttack2.wav",2,'\x01');
      FUN_00404a10(0xdb, "Data\\Sound\\mGorgonDie.wav",2,'\x01');
      FUN_00509810(iVar1,0x2d,0x2e,0x2f,0x30,0x31);
      *(undefined4 *)(DAT_05828d58 + 0xceb0) = 0x14;
      return;
    case 0xc:
    case 0xd:
      FUN_00404a10(0xee, "Data\\Sound\\mYeti1.wav",2,'\x01');
      FUN_00404a10(0xef, "Data\\Sound\\mYeti2.wav",2,'\x01');
      FUN_00404a10(0xf0, "Data\\Sound\\mYetiAttack1.wav",2,'\x01');
      FUN_00404a10(0xf1, "Data\\Sound\\mYetiDie.wav",2,'\x01');
      FUN_00509810(0x11a,0x44,0x44,0x46,0x46,0x47);
      FUN_00509810(0x11b,0x44,0x45,0x46,0x46,0x47);
      *(undefined4 *)(DAT_05828d58 + 0xcf6c) = 0x14;
      *(undefined4 *)(DAT_05828d58 + 0xd028) = 0x14;
      return;
    case 0xe:
      FUN_00404a10(0xeb, "Data\\Sound\\mAssassinAttack1.wav",2,'\x01');
      FUN_00404a10(0xec, "Data\\Sound\\mAssassinAttack2.wav",2,'\x01');
      FUN_00404a10(0xed, "Data\\Sound\\mAssassinDie.wav",2,'\x01');
      FUN_00509810(iVar1,0xffff,0xffff,0x41,0x42,0x43);
      *(undefined4 *)(DAT_05828d58 + 0xd0e4) = 0x14;
      *(undefined4 *)(*(int *)(iVar2 + 0x30) + 0x14) = 0x3eb33333;
      return;
    case 0xf:
      FUN_00404a10(0xdc, "Data\\Sound\\mIceMonster1.wav",2,'\x01');
      FUN_00404a10(0xdd, "Data\\Sound\\mIceMonster2.wav",2,'\x01');
      FUN_00404a10(0xde, "Data\\Sound\\mIceMonsterDie.wav",2,'\x01');
      FUN_00509810(iVar1,0x32,0x33,0x32,0x32,0x34);
      *(undefined4 *)(DAT_05828d58 + 0xd1a0) = 0x13;
      return;
    case 0x10:
      FUN_00404a10(0xe2, "Data\\Sound\\mHomord1.wav",2,'\x01');
      FUN_00404a10(0xe3, "Data\\Sound\\mHomord2.wav",2,'\x01');
      FUN_00404a10(0xe4, "Data\\Sound\\mHomordAttack1.wav",2,'\x01');
      FUN_00404a10(0xe5, "Data\\Sound\\mHomordDie.wav",2,'\x01');
      FUN_00509810(iVar1,0x38,0x39,0x3a,0x3a,0x3b);
      return;
    case 0x11:
      FUN_00404a10(0xdf, "Data\\Sound\\mWorm1.wav",2,'\x01');
      FUN_00404a10(0xe0, "Data\\Sound\\mWorm2.wav",2,'\x01');
      FUN_00404a10(0xe1, "Data\\Sound\\mWormDie.wav",2,'\x01');
      FUN_00509810(iVar1,0x35,0x35,0x37,0x37,0x37);
      return;
    case 0x12:
      FUN_00404a10(0xe6, "Data\\Sound\\mIceQueen1.wav",2,'\x01');
      FUN_00404a10(0xe7, "Data\\Sound\\mIceQueen2.wav",2,'\x01');
      FUN_00404a10(0xe8, "Data\\Sound\\mIceQueenAttack1.wav",2,'\x01');
      FUN_00404a10(0xe9, "Data\\Sound\\mIceQueenAttack2.wav",2,'\x01');
      FUN_00404a10(0xea, "Data\\Sound\\mIceQueenDie.wav",2,'\x01');
      FUN_00509810(iVar1,0x3c,0x3d,0x3e,0x3f,0x40);
      *(undefined4 *)(DAT_05828d58 + 0xd3d4) = 0x10;
      return;
    case 0x13:
      FUN_00404a10(0xf2, "Data\\Sound\\mGoblin1.wav",2,'\x01');
      FUN_00404a10(0xf3, "Data\\Sound\\mGoblin2.wav",2,'\x01');
      FUN_00404a10(0xf4, "Data\\Sound\\mGoblinAttack1.wav",2,'\x01');
      FUN_00404a10(0xf5, "Data\\Sound\\mGoblinAttack2.wav",2,'\x01');
      FUN_00404a10(0xf6, "Data\\Sound\\mGoblinDie.wav",2,'\x01');
      FUN_00509810(iVar1,0x48,0x49,0x4a,0x4b,0x4c);
      *(undefined4 *)(DAT_05828d58 + 0xd490) = 6;
      return;
    case 0x14:
      FUN_00404a10(0xf7, "Data\\Sound\\mScorpion1.wav",2,'\x01');
      FUN_00404a10(0xf8, "Data\\Sound\\mScorpion2.wav",2,'\x01');
      FUN_00404a10(0xf9, "Data\\Sound\\mScorpionAttack1.wav",2,'\x01');
      FUN_00404a10(0xfa, "Data\\Sound\\mScorpionAttack2.wav",2,'\x01');
      FUN_00404a10(0xfb, "Data\\Sound\\mScorpionDie.wav",2,'\x01');
      FUN_00509810(iVar1,0x4d,0x4e,0x4f,0x50,0x51);
      return;
    case 0x15:
      FUN_00404a10(0xfc, "Data\\Sound\\mBeetle1.wav",2,'\x01');
      FUN_00404a10(0xfd, "Data\\Sound\\mBeetleAttack1.wav",2,'\x01');
      FUN_00404a10(0xfe, "Data\\Sound\\mBeetleDie.wav",2,'\x01');
      FUN_00509810(iVar1,0x52,0x52,0x53,0x53,0x54);
      *(undefined4 *)(DAT_05828d58 + 0xd608) = 5;
      return;
    case 0x16:
      FUN_00404a10(0xff, "Data\\Sound\\mHunter1.wav",2,'\x01');
      FUN_00404a10(0x100, "Data\\Sound\\mHunter2.wav",2,'\x01');
      FUN_00404a10(0x101, "Data\\Sound\\mHunterAttack1.wav",2,'\x01');
      FUN_00404a10(0x102, "Data\\Sound\\mHunterAttack2.wav",2,'\x01');
      FUN_00404a10(0x103, "Data\\Sound\\mHunterDie.wav",2,'\x01');
      FUN_00509810(iVar1,0x55,0x56,0x57,0x58,0x59);
      *(undefined4 *)(DAT_05828d58 + 0xd6c4) = 6;
      return;
    case 0x17:
      FUN_00404a10(0x104, "Data\\Sound\\mWoodMon1.wav",2,'\x01');
      FUN_00404a10(0x105, "Data\\Sound\\mWoodMon2.wav",2,'\x01');
      FUN_00404a10(0x106, "Data\\Sound\\mWoodMonAttack1.wav",2,'\x01');
      FUN_00404a10(0x107, "Data\\Sound\\mWoodMonAttack2.wav",2,'\x01');
      FUN_00404a10(0x108, "Data\\Sound\\mWoodMonDie.wav",2,'\x01');
      FUN_00509810(iVar1,0x5a,0x5b,0x5c,0x5d,0x5e);
      *(undefined4 *)(DAT_05828d58 + 0xd780) = 6;
      return;
    case 0x18:
      FUN_00404a10(0x109, "Data\\Sound\\mArgon1.wav",2,'\x01');
      FUN_00404a10(0x10a, "Data\\Sound\\mArgon2.wav",2,'\x01');
      FUN_00404a10(0x10b, "Data\\Sound\\mArgonAttack1.wav",2,'\x01');
      FUN_00404a10(0x10c, "Data\\Sound\\mArgonAttack2.wav",2,'\x01');
      FUN_00404a10(0x10d, "Data\\Sound\\mArgonDie.wav",2,'\x01');
      FUN_00509810(iVar1,0x5f,0x60,0x61,0x62,99);
      *(undefined4 *)(DAT_05828d58 + 0xd83c) = 0x10;
      return;
    case 0x19:
      FUN_00404a10(0x10e, "Data\\Sound\\mGolem1.wav",2,'\x01');
      FUN_00404a10(0x10f, "Data\\Sound\\mGolem2.wav",2,'\x01');
      FUN_00404a10(0x110, "Data\\Sound\\mGolemAttack1.wav",2,'\x01');
      FUN_00404a10(0x111, "Data\\Sound\\mGolemAttack2.wav",2,'\x01');
      FUN_00404a10(0x112, "Data\\Sound\\mGolemDie.wav",2,'\x01');
      FUN_00509810(iVar1,100,0x65,0x66,0x67,0x68);
      *(undefined4 *)(DAT_05828d58 + 0xd8f8) = 5;
      return;
    case 0x1a:
      FUN_00404a10(0x113, "Data\\Sound\\mYeti1.wav",2,'\x01');
      FUN_00404a10(0x114, "Data\\Sound\\mSatanAttack1.wav",2,'\x01');
      FUN_00404a10(0x115, "Data\\Sound\\mYetiDie.wav",2,'\x01');
      FUN_00509810(iVar1,0x69,0x69,0x6a,0x6a,0x6b);
      *(undefined4 *)(DAT_05828d58 + 0xd9b4) = 6;
      return;
    case 0x1b:
      FUN_00404a10(0x116, "Data\\Sound\\mBalrog1.wav",2,'\x01');
      FUN_00404a10(0x117, "Data\\Sound\\mBalrog2.wav",2,'\x01');
      FUN_00404a10(0x118, "Data\\Sound\\mWizardAttack2.wav",2,'\x01');
      FUN_00404a10(0x119, "Data\\Sound\\mGorgonAttack2.wav",2,'\x01');
      FUN_00404a10(0x11a, "Data\\Sound\\mBalrogDie.wav",2,'\x01');
      FUN_00509810(iVar1,0x6c,0x6d,0x6e,0x6f,0x70);
      *(undefined4 *)(DAT_05828d58 + 0xda70) = 6;
      *(undefined1 *)(DAT_05828d58 + 0xdaa4) = 1;
      return;
    case 0x1c:
      FUN_00404a10(0x11b, "Data\\Sound\\mShadow1.wav",2,'\x01');
      FUN_00404a10(0x11c, "Data\\Sound\\mShadow2.wav",2,'\x01');
      FUN_00404a10(0x11d, "Data\\Sound\\mShadowAttack1.wav",2,'\x01');
      FUN_00404a10(0x11e, "Data\\Sound\\mShadowAttack2.wav",2,'\x01');
      FUN_00404a10(0x11f, "Data\\Sound\\mShadowDie.wav",2,'\x01');
      FUN_00509810(iVar1,0x71,0x72,0x73,0x74,0x75);
      *(undefined4 *)(DAT_05828d58 + 0xdb2c) = 5;
      return;
    case 0x1f:
      FUN_00404a10(0x125, "Data\\Sound\\mYeti1.wav",2,'\x01');
      FUN_00404a10(0x126, "Data\\Sound\\mBullAttack1.wav",2,'\x01');
      FUN_00404a10(0x127, "Data\\Sound\\mYetiDie.wav",2,'\x01');
      FUN_00509810(iVar1,0x7b,0x7b,0x7c,0x7c,0x7d);
      *(undefined4 *)(*(int *)(iVar2 + 0x30) + 0x34) = 0x3f000000;
      *(undefined4 *)(*(int *)(iVar2 + 0x30) + 0x44) = 0x3f333333;
      *(undefined4 *)(*(int *)(iVar2 + 0x30) + 0x14) = 0x3f4ccccd;
      *(undefined4 *)(*(int *)(iVar2 + 0x30) + 0x74) = 0x3f4ccccd;
      return;
    case 0x20:
      FUN_00404a10(0x128, "Data\\Sound\\mBali1.wav",2,'\x01');
      FUN_00404a10(0x129, "Data\\Sound\\mBali2.wav",2,'\x01');
      FUN_00404a10(0x12a, "Data\\Sound\\mBaliAttack1.wav",2,'\x01');
      FUN_00404a10(299, "Data\\Sound\\mBaliAttack2.wav",2,'\x01');
      FUN_00509810(iVar1,0x7e,0x7f,0x80,0x81,0x7f);
      *(undefined4 *)(*(int *)(iVar2 + 0x30) + 0x84) = 0x3ecccccd;
      *(undefined4 *)(*(int *)(iVar2 + 0x30) + 0x94) = 0x3ecccccd;
      *(undefined4 *)(*(int *)(iVar2 + 0x30) + 0x74) = 0x3ecccccd;
      *(undefined4 *)(*(int *)(iVar2 + 0x30) + 0xa4) = 0x3ecccccd;
      *(undefined4 *)(iVar2 + 0x54) = 6;
      return;
    case 0x21:
      FUN_00404a10(300, "Data\\Sound\\mBahamut1.wav",2,'\x01');
      FUN_00404a10(0x12d, "Data\\Sound\\mYeti1.wav",2,'\x01');
      FUN_00509810(iVar1,0x82,0x82,0x83,0x83,0x82);
      return;
    case 0x22:
      FUN_00404a10(0x12e, "Data\\Sound\\mBepar1.wav",2,'\x01');
      FUN_00404a10(0x12f, "Data\\Sound\\mBepar2.wav",2,'\x01');
      FUN_00404a10(0x130, "Data\\Sound\\mBalrog1.wav",2,'\x01');
      FUN_00509810(iVar1,0x84,0x85,0x68,0x68,0x85);
      *(undefined4 *)(*(int *)(iVar2 + 0x30) + 0x34) = 0x3f000000;
      *(undefined4 *)(*(int *)(iVar2 + 0x30) + 0x44) = 0x3f000000;
      *(undefined4 *)(iVar2 + 0x54) = 0x14;
      return;
    case 0x23:
      FUN_00404a10(0x131, "Data\\Sound\\mValkyrie1.wav",2,'\x01');
      FUN_00404a10(0x132, "Data\\Sound\\mBaliAttack2.wav",2,'\x01');
      FUN_00404a10(0x133, "Data\\Sound\\mValkyrieDie.wav",2,'\x01');
      FUN_00509810(iVar1,0x87,0x87,0x88,0x88,0x89);
      *(undefined4 *)(DAT_05828d58 + 0xe050) = 0x13;
      return;
    case 0x24:
    case 0x28:
      FUN_00404a10(0x134, "Data\\Sound\\mLizardKing1.wav",2,'\x01');
      FUN_00404a10(0x135, "Data\\Sound\\mLizardKing2.wav",2,'\x01');
      FUN_00404a10(0x136, "Data\\Sound\\mGorgonDie.wav",2,'\x01');
      FUN_00509810(iVar1,0x8a,0x8b,0x8a,0x8b,0x8c);
      if (param_1 == 0x24) {
        *(undefined4 *)(DAT_05828d58 + 0xe10c) = 0x13;
        return;
      }
      *(undefined4 *)(DAT_05828d58 + 0xc69c + param_1 * 0xbc) = 6;
      return;
    case 0x25:
      FUN_00404a10(0x137, "Data\\Sound\\mHydra1.wav",2,'\x01');
      FUN_00404a10(0x138, "Data\\Sound\\mHydraAttack1.wav",2,'\x01');
      FUN_00509810(iVar1,0x8d,0x8d,0x8e,0x8e,0x8d);
      *(undefined4 *)(*(int *)(iVar2 + 0x30) + 0x34) = 0x3e19999a;
      *(undefined4 *)(*(int *)(iVar2 + 0x30) + 0x44) = 0x3e19999a;
      *(undefined4 *)(*(int *)(iVar2 + 0x30) + 100) = 0x3e4ccccd;
      return;
    case 0x29:
      FUN_00404a10(0x139,s_Data_Sound_iron1_wav_0055d074,2,'\x01');
      FUN_00404a10(0x13a,s_Data_Sound_iron_attack1_wav_0055d058,2,'\x01');
      FUN_00509810(iVar1,0x8f,0x8f,0x90,0x90,0x90);
      *(undefined4 *)(DAT_05828d58 + 0xe4b8) = 3;
      return;
    case 0x2a:
      FUN_00404a10(0x13b,s_Data_Sound_jaikan1_wav_0055d040,2,'\x01');
      FUN_00404a10(0x13c,s_Data_Sound_jaikan2_wav_0055d028,2,'\x01');
      FUN_00404a10(0x13d,s_Data_Sound_jaikan_attack1_wav_0055d008,2,'\x01');
      FUN_00404a10(0x13e,s_Data_Sound_jaikan_attack2_wav_0055cfe8,2,'\x01');
      FUN_00404a10(0x13f,s_Data_Sound_jaikan_die_wav_0055cfcc,2,'\x01');
      FUN_00509810(iVar1,0x91,0x92,0x93,0x94,0x95);
      FUN_00529740(s_Monster_bv01_2_jpg_0055cfb8,0x56b,0x2601,0x2901,0,'\x01');
      FUN_00529740(s_Monster_bv02_2_jpg_0055cfa4,0x56c,0x2601,0x2901,0,'\x01');
      *(undefined4 *)(DAT_05828d58 + 0xe574) = 0x14;
      return;
    case 0x2b:
      FUN_00404a10(0x140,s_Data_Sound_blood1_wav_0055cf8c,2,'\x01');
      FUN_00404a10(0x141,s_Data_Sound_blood_attack1_wav_0055cf6c,2,'\x01');
      FUN_00404a10(0x142,s_Data_Sound_blood_attack2_wav_0055cf4c,2,'\x01');
      FUN_00404a10(0x143,s_Data_Sound_blood_die_wav_0055cf30,2,'\x01');
      FUN_00509810(iVar1,0x96,0x96,0x97,0x98,0x99);
      *(undefined4 *)(DAT_05828d58 + 0xe630) = 7;
      return;
    case 0x2c:
      FUN_00404a10(0x144,s_Data_Sound_death1_wav_0055cf18,2,'\x01');
      FUN_00404a10(0x145,s_Data_Sound_death_attack1_wav_0055cef8,2,'\x01');
      FUN_00404a10(0x146,s_Data_Sound_death_die_wav_0055cedc,2,'\x01');
      FUN_00509810(iVar1,0x9a,0x9a,0x9b,0x9b,0x9c);
      *(undefined4 *)(DAT_05828d58 + 0xe6ec) = 6;
      return;
    case 0x2d:
      FUN_00404a10(0x147,s_Data_Sound_mutant1_wav_0055cec4,2,'\x01');
      FUN_00404a10(0x148,s_Data_Sound_mutant2_wav_0055ceac,2,'\x01');
      FUN_00404a10(0x149,s_Data_Sound_mutant_attack1_wav_0055ce8c,2,'\x01');
      FUN_00509810(iVar1,0x9d,0x9e,0x9f,0x9f,0x9f);
      *(undefined4 *)(DAT_05828d58 + 0xe7a8) = 6;
      return;
    case 0x2e:
      FUN_00404a10(0x14c,s_Data_Sound_mOrcArcherAttack1_wav_0055ce68,2,'\x01');
      FUN_00404a10(0xae, "Data\\Sound\\mBullDie.wav",2,'\x01');
      FUN_00509810(iVar1,0xffff,0xffff,0xa2,0xa2,4);
      *(undefined4 *)(DAT_05828d58 + 0xe864) = 7;
      return;
    case 0x2f:
      FUN_00404a10(0x100, "Data\\Sound\\mHunter2.wav",2,'\x01');
      FUN_00404a10(0xae, "Data\\Sound\\mBullDie.wav",2,'\x01');
      FUN_00404a10(0x14b,s_Data_Sound_mOrcCapAttack1_wav_0055ce48,2,'\x01');
      FUN_00509810(iVar1,0x56,0x56,0xa1,0xa1,4);
      *(undefined4 *)(DAT_05828d58 + 0xe920) = 0x14;
      return;
    case 0x30:
      FUN_00404a10(0x14a,s_Data_Sound_mCursedKing1_wav_0055ce2c,2,'\x01');
      FUN_00404a10(0x14e,s_Data_Sound_mCursedKing2_wav_0055ce10,2,'\x01');
      FUN_00404a10(0x14d,s_Data_Sound_mCursedKingDie1_wav_0055cdf0,2,'\x01');
      FUN_00509810(iVar1,0xa0,0xa4,0xffff,0xffff,0xa3);
      *(undefined4 *)(DAT_05828d58 + 0xe9dc) = 0x14;
      return;
    case 0x31:
      *(undefined4 *)(*(int *)(iVar2 + 0x30) + 100) = 0x3e6147ae;
      FUN_00404a10(0x15b,s_Data_Sound_mMolt1_wav_0055cd50,2,'\x01');
      FUN_00404a10(0x15c,s_Data_Sound_mMoltAttack1_wav_0055cd34,2,'\x01');
      FUN_00404a10(0x15d,s_Data_Sound_mMoltDie_wav_0055cd1c,2,'\x01');
      FUN_00509810(iVar1,0xb1,0xb1,0xb2,0xb2,0xb3);
      return;
    case 0x32:
      *(undefined4 *)(*(int *)(iVar2 + 0x30) + 100) = 0x3e6147ae;
      FUN_00404a10(0x159,s_Data_Sound_mAlquamosAttack1_wav_0055ccfc,2,'\x01');
      FUN_00404a10(0x15a,s_Data_Sound_mAlquamosDie_wav_0055cce0,2,'\x01');
      FUN_00509810(iVar1,0xaf,0xaf,0xaf,0xaf,0xb0);
      return;
    case 0x33:
      *(undefined4 *)(*(int *)(iVar2 + 0x30) + 100) = 0x3e6147ae;
      FUN_00404a10(0x155,s_Data_Sound_mRainner1_wav_0055ccc4,2,'\x01');
      FUN_00404a10(0x156,s_Data_Sound_mRainnerAttack1_wav_0055cca4,2,'\x01');
      FUN_00404a10(0x157,s_Data_Sound_mRainnerDie_wav_0055cc88,2,'\x01');
      FUN_00509810(iVar1,0xab,0xffff,0xac,0xac,0xad);
      return;
    case 0x34:
      FUN_00529bd0(s_Monster_iui02_tga_0055cddc,0x1ed,0x2600,0x2900,0,'\x01');
      FUN_00529bd0(s_Monster_iui03_tga_0055cdc8,0x1ef,0x2600,0x2900,0,'\x01');
      *(undefined4 *)(*(int *)(iVar2 + 0x30) + 100) = 0x3e6147ae;
      FUN_00404a10(0x15e,s_Data_Sound_mMegaCrust1_wav_0055cdac,2,'\x01');
      FUN_00404a10(0x15f,s_Data_Sound_mMegaCrustAttack1_wav_0055cd88,2,'\x01');
      FUN_00404a10(0x160,s_Data_Sound_mMegaCrustDie_wav_0055cd68,2,'\x01');
      FUN_00509810(iVar1,0xb4,0xb4,0xb5,0xb5,0xb6);
      return;
    case 0x35:
      *(undefined4 *)(*(int *)(iVar2 + 0x30) + 100) = 0x3e6147ae;
      FUN_00404a10(0x152,s_Data_Sound_mPhantom1_wav_0055cc6c,2,'\x01');
      FUN_00404a10(0x153,s_Data_Sound_mPhantomAttack1_wav_0055cc4c,2,'\x01');
      FUN_00404a10(0x154,s_Data_Sound_mPhantomDie_wav_0055cc30,2,'\x01');
      FUN_00509810(iVar1,0xa8,0xa8,0xa9,0xa9,0xaa);
      return;
    case 0x36:
      *(undefined4 *)(*(int *)(iVar2 + 0x30) + 100) = 0x3e6147ae;
      FUN_00404a10(0x14f,s_Data_Sound_mDrakan1_wav_0055cc18,2,'\x01');
      FUN_00404a10(0x150,s_Data_Sound_mDrakanAttack1_wav_0055cbf8,2,'\x01');
      FUN_00404a10(0x151,s_Data_Sound_mDrakanDie_wav_0055cbdc,2,'\x01');
      FUN_00509810(iVar1,0xa5,0xa5,0xa6,0xa6,0xa7);
      return;
    case 0x37:
      FUN_00404a10(0x161,s_Data_Sound_mPhoenix1_wav_0055cbc0,2,'\x01');
      FUN_00404a10(0x162,s_Data_Sound_mPhoenix1_wav_0055cbc0,2,'\x01');
      FUN_00404a10(0x163,s_Data_Sound_mPhoenixAttack1_wav_0055cba0,2,'\x01');
      FUN_00509810(iVar1,0xb7,0xb8,0xb9,0xb9,0xffff);
    case 0x38:
      *(undefined4 *)(*(int *)(iVar2 + 0x30) + 100) = 0x3e6147ae;
      return;
    case 0x39:
      FUN_00404a10(0x16a,s_Data_Sound_mRedSkull_wav_0055caec,2,'\x01');
      FUN_00404a10(0x16b,s_Data_Sound_mRedSkullDie_wav_0055cad0,2,'\x01');
      FUN_00404a10(0x16c,s_Data_Sound_mRedSkullAttack_wav_0055cab0,2,'\x01');
      FUN_00509810(iVar1,0xc0,0xffff,0xc2,0xffff,0xc1);
      break;
    case 0x3a:
      FUN_00404a10(0x166, "Data\\Sound\\mHunter2.wav",2,'\x01');
      FUN_00404a10(0x169,s_Data_Sound_mGhaintOrgerDie_wav_0055cb08,2,'\x01');
      FUN_00509810(iVar1,0xbc,0xffff,0xbe,0xffff,0xbf);
      return;
    case 0x3b:
      FUN_00404a10(0xae,s_Data_Sound_mBullDie_wav_0055cb6c,2,'\x01');
      FUN_00404a10(0x166, "Data\\Sound\\mHunter2.wav",2,'\x01');
      FUN_00404a10(0x167,s_Data_Sound_mBlackSkullDie_wav_0055cb4c,2,'\x01');
      FUN_00404a10(0x168,s_Data_Sound_mBlackSkullAttack_wav_0055cb28,2,'\x01');
      FUN_00509810(iVar1,0xbc,0xffff,0xbe,0xffff,0xbd);
      return;
    case 0x3e:
      FUN_00404a10(0x164,s_Data_Sound_mMagicSkull_wav_0055cb84,2,'\x01');
      FUN_00404a10(0x165,s_Data_Sound_mMagicSkull_wav_0055cb84,2,'\x01');
      FUN_00509810(iVar1,0xba,0xffff,0xffff,0xffff,0xbb);
      return;
    }
  }
  return;
}
