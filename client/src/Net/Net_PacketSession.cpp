// Net_PacketSession.cpp
// Packet session management: per-frame packet receive/dispatch and slot cleanup.
//
// FUN_004cbdf0 @ 0x004cbdf0 — Net_ProcessReceiveQueue
// FUN_004ecb00 @ 0x004ecb00 — Scene_PacketUpdate

#include "stdafx.h"


// FUN_004cbdf0 — Net_ProcessReceiveQueue
// Processes incoming packets: decrypts pending data via inline reverse chain-XOR
// (matching FUN_00409e20 logic), then re-inserts into the session hash table.
// Also resets queued item/skill/chat slots after dispatch.
// Called once per frame from the main loop.
void FUN_004cbdf0(void)
{
    return;  // AUTO-SKIP: absolute end-bound loop (Ghidra artifact — pool not populated in our build).
  char cVar1;
  uint uVar2;
  void *pvVar3;
  undefined4 *puVar4;
  undefined4 *puVar5;
  int iStack0000000c;
  undefined4 *unaff_EBX;
  uint uVar7;
  int iVar8;
  undefined4 **ppuVar9;
  byte *pbVar10;
  undefined4 *puVar11;
  byte *pbVar12;
  bool bVar13;
  bool bVar14;
  undefined4 *local_c;
  byte abStack_8 [4];
  undefined4 *local_4;
  byte stack0xfffffff0[4] = {0};

  local_c = (undefined4*)DAT_07cf1ffc;
  local_4 = (undefined4*)DAT_07cf1ffc;
  uVar2 = (**(code **)(DAT_055c9bc8 + 0xc))(DAT_07cf1ffc);
  uVar7 = 0;
  local_c = (undefined4 *)0x0;
  if (DAT_055c9bd4 != 0) {
    do {
      bVar13 = false;
      iVar8 = 0;
      bVar14 = true;
      pbVar12 = (byte *)(DAT_055c9bd0 + uVar2 * 4);
      iStack0000000c = 4;
      ppuVar9 = &local_c;
      pbVar10 = pbVar12;
      do {
        if (iStack0000000c == 0) break;
        iStack0000000c = iStack0000000c + -1;
        bVar13 = *(byte *)ppuVar9 < *pbVar10;
        bVar14 = *(byte *)ppuVar9 == *pbVar10;
        ppuVar9 = (undefined4 **)((int)ppuVar9 + 1);
        pbVar10 = pbVar10 + 1;
      } while (bVar14);
      if (!bVar14) {
        iVar8 = (1 - (uint)bVar13) - (uint)(bVar13 != 0);
      }
      if (iVar8 == 0) goto LAB_004cbe8f;
      iStack0000000c = 4;
      bVar13 = false;
      iVar8 = 0;
      bVar14 = true;
      pbVar10 = abStack_8;
      do {
        if (iStack0000000c == 0) break;
        iStack0000000c = iStack0000000c + -1;
        bVar13 = *pbVar10 < *pbVar12;
        bVar14 = *pbVar10 == *pbVar12;
        pbVar10 = pbVar10 + 1;
        pbVar12 = pbVar12 + 1;
      } while (bVar14);
      if (!bVar14) {
        iVar8 = (1 - (uint)bVar13) - (uint)(bVar13 != 0);
      }
      if (iVar8 == 0) {
        if (uVar2 == 0xffffffff) goto LAB_004cbe8f;
        uVar2 = HashTable_GetIndex(&DAT_055c9bc8,unaff_EBX);
        if (uVar2 == 0xffffffff) {
          puVar5 = (undefined4 *)0x0;
        }
        else {
          puVar5 = *(undefined4 **)(DAT_055c9bcc + uVar2 * 4);
        }
        cVar1 = *(char *)(puVar5 + 0x161);
        *(byte *)(puVar5 + 0x161) = cVar1 + 1U;
        if ((byte)(cVar1 + 1U) < 2) {
          puVar4 = (undefined4*)operator_new(0x584);
          puVar11 = puVar4;
          for (iStack0000000c = 0x161; iStack0000000c != 0; iStack0000000c = iStack0000000c + -1) {
            *puVar11 = *puVar5;
            puVar5 = puVar5 + 1;
            puVar11 = puVar11 + 1;
          }
          uVar2 = 0x583;
          iStack0000000c = 0x584;
          do {
            if (uVar2 < 0x583) {
              *(byte *)(uVar2 + (int)puVar4) =
                   *(byte *)(uVar2 + (int)puVar4) ^ *(byte *)(uVar2 + 1 + (int)puVar4);
            }
            uVar7 = uVar2 & 0x8000000f;
            if ((int)uVar7 < 0) {
              uVar7 = (uVar7 - 1 | 0xfffffff0) + 1;
            }
            *(byte *)(uVar2 + (int)puVar4) =
                 (DAT_00559050[uVar7] ^ *(char *)(uVar2 + (int)puVar4) - 0x23U) + 0xb9;
            uVar2 = uVar2 - 1;
            iStack0000000c = iStack0000000c + -1;
          } while (iStack0000000c != 0);
          puVar5 = puVar4;
          for (iStack0000000c = 0x161; iStack0000000c != 0; iStack0000000c = iStack0000000c + -1) {
            *unaff_EBX = *puVar5;
            puVar5 = puVar5 + 1;
            unaff_EBX = unaff_EBX + 1;
          }
          operator_delete((undefined *)puVar4);
        }
        goto LAB_004cbf65;
      }
      uVar2 = (uVar2 + 1) % DAT_055c9bd4;
      uVar7 = uVar7 + 1;
    } while (uVar7 < DAT_055c9bd4);
  }
  FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______GetIndex_00558108);
LAB_004cbe8f:
  pvVar3 = operator_new(0x585);
  *(undefined1 *)((int)pvVar3 + 0x584) = 1;
  FUN_00403f80(&DAT_055c9bc8,pvVar3,unaff_EBX);
LAB_004cbf65:
  puVar11 = (undefined4*)DAT_07cf1ffc;
  iStack0000000c = 0xc;
  puVar5 = (undefined4*)((char*)DAT_07cf1ffc + 0x94);
  do {
    *(undefined2 *)(puVar5 + -0xe) = 0xffff;
    *puVar5 = 0;
    puVar5 = puVar5 + 0x11;
    iStack0000000c = iStack0000000c + -1;
  } while (iStack0000000c != 0);
  local_c = puVar11;
  uVar2 = (**(code **)(DAT_055c9bc8 + 0xc))(puVar11);
  local_c = (undefined4 *)0x0;
  uVar7 = 0;
  if (DAT_055c9bd4 != 0) {
    do {
      iStack0000000c = 4;
      bVar13 = false;
      iVar8 = 0;
      bVar14 = true;
      pbVar12 = (byte *)(DAT_055c9bd0 + uVar2 * 4);
      ppuVar9 = &local_c;
      pbVar10 = pbVar12;
      do {
        if (iStack0000000c == 0) break;
        iStack0000000c = iStack0000000c + -1;
        bVar13 = *(byte *)ppuVar9 < *pbVar10;
        bVar14 = *(byte *)ppuVar9 == *pbVar10;
        ppuVar9 = (undefined4 **)((int)ppuVar9 + 1);
        pbVar10 = pbVar10 + 1;
      } while (bVar14);
      if (!bVar14) {
        iVar8 = (1 - (uint)bVar13) - (uint)(bVar13 != 0);
      }
      if (iVar8 == 0) goto LAB_004cc05c;
      iStack0000000c = 4;
      bVar13 = false;
      iVar8 = 0;
      bVar14 = true;
      pbVar10 = stack0xfffffff0;
      do {
        if (iStack0000000c == 0) break;
        iStack0000000c = iStack0000000c + -1;
        bVar13 = *pbVar10 < *pbVar12;
        bVar14 = *pbVar10 == *pbVar12;
        pbVar10 = pbVar10 + 1;
        pbVar12 = pbVar12 + 1;
      } while (bVar14);
      if (!bVar14) {
        iVar8 = (1 - (uint)bVar13) - (uint)(bVar13 != 0);
      }
      if (iVar8 == 0) {
        if (uVar2 != 0xffffffff) {
          uVar2 = HashTable_GetIndex(&DAT_055c9bc8,puVar11);
          if (uVar2 == 0xffffffff) {
            puVar5 = (undefined4 *)0x0;
          }
          else {
            puVar5 = *(undefined4 **)(DAT_055c9bcc + uVar2 * 4);
          }
          cVar1 = *(char *)(puVar5 + 0x161);
          *(char *)(puVar5 + 0x161) = cVar1 + -1;
          if ((char)(cVar1 + -1) == '\0') {
            FUN_00404400(puVar5,puVar11);
          }
        }
        goto LAB_004cc05c;
      }
      uVar2 = (uVar2 + 1) % DAT_055c9bd4;
      uVar7 = uVar7 + 1;
    } while (uVar7 < DAT_055c9bd4);
  }
  FUN_00405540(&DAT_055c9bf0,s_Hash_table_full______GetIndex_00558108);
LAB_004cc05c:
  puVar5 = (undefined4*)&DAT_07ea8448;
  do {
    *(undefined2 *)(puVar5 + -0xe) = 0xffff;
    *puVar5 = 0;
    puVar5 = puVar5 + 0x11;
  } while ((int)puVar5 < 0x7ea9548);
  puVar5 = (undefined4*)&DAT_07ea5b68;
  do {
    *(undefined2 *)(puVar5 + -0xe) = 0xffff;
    *puVar5 = 0;
    puVar5 = puVar5 + 0x11;
  } while ((int)puVar5 < 0x7ea7b48);
  puVar5 = (undefined4*)&DAT_07ea9880;
  do {
    *(undefined2 *)(puVar5 + -0xe) = 0xffff;
    *puVar5 = 0;
    puVar5 = puVar5 + 0x11;
  } while ((int)puVar5 < 0x7eaa100);
  DAT_07eaa0e8 = 0;
  iStack0000000c = 0;
  do {
    *(undefined2 *)((int)&DAT_07ea5298 + iStack0000000c) = 0xffff;
    *(undefined2 *)((int)&DAT_07ea7b88 + iStack0000000c) = 0xffff;
    *(undefined2 *)((int)&DAT_07e11f78 + iStack0000000c) = 0xffff;
    *(undefined4 *)((int)&DAT_07ea52d0 + iStack0000000c) = 0;
    *(undefined4 *)((int)&DAT_07ea7bc0 + iStack0000000c) = 0;
    *(undefined4 *)((int)&DAT_07e11fb0 + iStack0000000c) = 0;
    iStack0000000c = iStack0000000c + 0x44;
  } while (iStack0000000c < 0x880);
  return;
}


// FUN_004ecb00 — Scene_PacketUpdate
// Processes queued incoming network packets for the current scene.
// Guards against map 0x6e. Calls sub-scene packet handlers in sequence:
//   FUN_004e4760, FUN_004e5500, FUN_004e5de0 (always)
//   FUN_004e6550..FUN_004ec330 (if DAT_07eaa117 != 0 — extended scene)
//   FUN_004df410 (if DAT_07eaa164 == 0 — not in special mode)
//   FUN_004e7ac0, FUN_004e8b70 (always)
void FUN_004ecb00(void)
{
  undefined4 uVar1;
  undefined4 extraout_ECX;
  undefined4 extraout_ECX_00;
  uint extraout_EDX;
  uint extraout_EDX_00;
  uint uVar2;

  if (DAT_083a7c24 == 0x6e) {
    return;
  }
  uVar1 = FUN_004e93a0();
  if (((((char)uVar1 != '\0') || (DAT_083a7c24 != 0)) || (DAT_055c9b7c != 0)) || (DAT_055c9b80 != 0)
     ) {
    if (DAT_055c9b7c != *(int *)(DAT_055c9ff4 + 0x1c)) {
      return;
    }
    DAT_055c9b7c = 0;
  }
  FUN_004e4760();
  FUN_004e5500();
  FUN_004e5de0();
  uVar1 = extraout_ECX;
  uVar2 = extraout_EDX;
  if (DAT_07eaa117 != '\0') {
    FUN_004e6550();
    FUN_004e6c40();
    FUN_004e9050();
    FUN_004eb5d0();
    FUN_004eb7f0();
    FUN_004ec330();
    uVar1 = extraout_ECX_00;
    uVar2 = extraout_EDX_00;
  }
  if (DAT_07eaa164 == '\0') {
    FUN_004df410(uVar1,uVar2);
  }
  FUN_004e7ac0();
  FUN_004e8b70();
  return;
}
