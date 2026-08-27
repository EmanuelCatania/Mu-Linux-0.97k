// PathFinder.cpp
// A* pathfinder and BST priority-queue helpers.
//
// FUN_0043fd30 @ 0x0043FD30  — BinTree_UpdateRange   (21 lines)
// FUN_0043fea0 @ 0x0043FEA0  — BinTree_Insert        (56 lines)
// FUN_004232f0 @ 0x004232F0  — BinTree_PopMin        (77 lines)
// FUN_004235d0 @ 0x004235D0  — BinTree_DeleteNode    (79 lines)
// FUN_0043ff60 @ 0x0043FF60  — BinTree_ClearHead     (91 lines)
// FUN_0043f500 @ 0x0043F500  — PathFinder2_Solve     (340 lines)
//
// BST node layout (20 bytes each):
//   [0] data/node-id   [1] cost(key)   [2] left*   [3] right*   [4] parent*
//
// PathFinder2 'this' layout (struct PathFinderCtx):
//   [+0x00] width        [+0x04] height      [+0x08] size (w*h)
//   [+0x0c] walkmap*     [+0x3fc] flagmap*   [+0x400] range_min   [+0x404] range_max
//   [+0x408] costmap*    [+0x40c] parentX*   [+0x410] parentY*
//   [+0x414] open_set (BST root at [2], count at [1])
//   [+0x418] —         [+0x41c] closed_set
//   [+0x10] path_len    [+0x207..] path_x    [+0x3fb..] path_y

#include "stdafx.h"
#pragma warning(disable: 4554 4302 4146)

// 8-directional movement table: pairs of (dx, dy) for grid traversal.
// In the original binary: DAT_00561578..DAT_005615BB (16 ints = 8 directions).
// We use a local array with a padding element at [-1] to match the loop pattern.
static const int s_astar_dir_pad = 0;  // unused
static const int s_astar_dirs[16] = {
  // dx,  dy
  -1, -1,
   0, -1,
   1, -1,
  -1,  0,
   1,  0,
  -1,  1,
   0,  1,
   1,  1,
};
// In the original code: local_28 = &DAT_0056157c (= &s_astar_dirs[1], the dy of dir 0)
// End condition:         (int)local_28 < 0x5615bc → local_28 < s_astar_dirs + 16
#define ASTAR_DIR_START  (s_astar_dirs + 1)
#define ASTAR_DIR_END    (s_astar_dirs + 16)


// FUN_0043fd30 — BinTree_UpdateRange
// Updates the min/max dirty-range [this+0x400..this+0x404] to include param_1.
// Returns CONCAT31((int3)(param_1>>8), 1) on success, (param_1 & ~0xFF) on out-of-range.
undefined4 __cdecl FUN_0043fd30(void *_this, int param_1)
{
  int iVar1;

  if ((-1 < param_1) && (param_1 < *(int *)((int)_this + 8))) {
    iVar1 = *(int *)((int)_this + 0x400);
    if (param_1 < *(int *)((int)_this + 0x400)) {
      iVar1 = param_1;
    }
    *(int *)((int)_this + 0x400) = iVar1;
    if (param_1 <= *(int *)((int)_this + 0x404)) {
      param_1 = *(int *)((int)_this + 0x404);
    }
    *(int *)((int)_this + 0x404) = param_1;
    return CONCAT31((int3)((uint)param_1 >> 8), 1);
  }
  return param_1 & 0xffffff00;
}


// FUN_0043fea0 — BinTree_Insert
// Inserts a (data, cost) pair into the BST keyed by cost (param_2).
// this+0x8 = root, this+0x4 = count.
void __cdecl FUN_0043fea0(void *_this, undefined4 param_1, int param_2)
{
  int iVar1;
  int iVar2;
  undefined4 *puVar3;

  iVar1 = *(int *)((int)_this + 8);
  iVar2 = iVar1;
  if (iVar1 == 0) {
    *(int *)((int)_this + 4) = *(int *)((int)_this + 4) + 1;
    puVar3 = (undefined4 *)operator_new(0x14);
    if (puVar3 != (undefined4 *)0x0) {
      puVar3[4] = 0;
      puVar3[3] = 0;
      puVar3[2] = 0;
      *puVar3 = param_1;
      puVar3[1] = param_2;
      *(undefined4 **)((int)_this + 8) = puVar3;
      return;
    }
    *(undefined4 *)((int)_this + 8) = 0;
    return;
  }
  while (iVar1 != 0) {
    iVar2 = iVar1;
    if (param_2 < *(int *)(iVar1 + 4)) {
      iVar1 = *(int *)(iVar1 + 8);
    }
    else {
      iVar1 = *(int *)(iVar1 + 0xc);
    }
  }
  *(int *)((int)_this + 4) = *(int *)((int)_this + 4) + 1;
  puVar3 = (undefined4 *)operator_new(0x14);
  if (puVar3 == (undefined4 *)0x0) {
    puVar3 = (undefined4 *)0x0;
  }
  else {
    puVar3[4] = 0;
    puVar3[3] = 0;
    puVar3[2] = 0;
    *puVar3 = param_1;
    puVar3[1] = param_2;
  }
  if (param_2 < *(int *)(iVar2 + 4)) {
    *(undefined4 **)(iVar2 + 8) = puVar3;
    puVar3[4] = iVar2;
    return;
  }
  *(undefined4 **)(iVar2 + 0xc) = puVar3;
  puVar3[4] = iVar2;
  return;
}


// FUN_004232f0 — BinTree_PopMin
// Removes and returns the minimum-key node from the BST.
// param_1 is a pointer-to-pointer to the root or a subtree root.
undefined4 __cdecl FUN_004232f0(void *_this, int *param_1)
{
  undefined4 *puVar1;
  undefined4 uVar2;
  int iVar3;
  int iVar4;
  int *piVar5;
  int *piVar6;

  piVar5 = param_1;
  puVar1 = (undefined4 *)*param_1;
  uVar2 = *puVar1;
  param_1 = (int *)0x0;
  piVar6 = (int *)puVar1[2];
  if (piVar6 == (int *)0x0) {
    piVar6 = (int *)puVar1[3];
    if (piVar6 == (int *)0x0) {
      iVar4 = puVar1[4];
      if (iVar4 == 0) {
        *(undefined4 *)((int)_this + 8) = 0;
      }
      else if (*(undefined4 **)(iVar4 + 8) == puVar1) {
        *(undefined4 *)(iVar4 + 8) = 0;
      }
      else {
        *(undefined4 *)(iVar4 + 0xc) = 0;
      }
      *(int *)((int)_this + 4) = *(int *)((int)_this + 4) + -1;
      if ((undefined *)*piVar5 != (undefined *)0x0) {
        operator_delete((undefined *)*piVar5);
      }
      *piVar5 = 0;
      return uVar2;
    }
    if (puVar1 == *(undefined4 **)((int)_this + 8)) {
      *(int **)((int)_this + 8) = piVar6;
      piVar6[4] = 0;
      goto LAB_0042337f;
    }
  }
  else {
    if (puVar1[3] != 0) {
      do {
        param_1 = piVar6;
        piVar6 = (int *)param_1[3];
      } while ((int *)param_1[3] != (int *)0x0);
      iVar4 = *param_1;
      iVar3 = param_1[1];
      FUN_004232f0(_this, (int *)&param_1);
      *(int *)*piVar5 = iVar4;
      *(int *)(*piVar5 + 4) = iVar3;
      return uVar2;
    }
    if (puVar1 == *(undefined4 **)((int)_this + 8)) {
      *(int **)((int)_this + 8) = piVar6;
      piVar6[4] = 0;
      goto LAB_0042337f;
    }
  }
  iVar4 = puVar1[4];
  if (puVar1 == *(undefined4 **)(iVar4 + 0xc)) {
    *(int **)(iVar4 + 0xc) = piVar6;
  }
  else {
    *(int **)(iVar4 + 8) = piVar6;
  }
  piVar6[4] = iVar4;
LAB_0042337f:
  if ((undefined *)*piVar5 != (undefined *)0x0) {
    operator_delete((undefined *)*piVar5);
  }
  *(int *)((int)_this + 4) = *(int *)((int)_this + 4) + -1;
  return uVar2;
}


// FUN_004235d0 — BinTree_DeleteNode
// Recursively deletes a node and all its children from the BST.
void __cdecl FUN_004235d0(void *_this, undefined4 *param_1)
{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 *puVar4;
  undefined4 *puVar5;

  puVar4 = param_1;
  if ((undefined4 *)param_1[2] != (undefined4 *)0x0) {
    FUN_004235d0(_this, (undefined4 *)param_1[2]);
  }
  if ((undefined4 *)puVar4[3] != (undefined4 *)0x0) {
    FUN_004235d0(_this, (undefined4 *)puVar4[3]);
  }
  puVar5 = (undefined4 *)puVar4[2];
  param_1 = (undefined4 *)0x0;
  if (puVar5 == (undefined4 *)0x0) {
    puVar5 = (undefined4 *)puVar4[3];
    if (puVar5 == (undefined4 *)0x0) {
      iVar1 = puVar4[4];
      if (iVar1 == 0) {
        *(undefined4 *)((int)_this + 8) = 0;
      }
      else if (*(undefined4 **)(iVar1 + 8) == puVar4) {
        *(undefined4 *)(iVar1 + 8) = 0;
      }
      else {
        *(undefined4 *)(iVar1 + 0xc) = 0;
      }
      *(int *)((int)_this + 4) = *(int *)((int)_this + 4) + -1;
      if (puVar4 != (undefined4 *)0x0) {
        operator_delete((undefined *)puVar4);
      }
      return;
    }
    if (puVar4 == *(undefined4 **)((int)_this + 8)) {
      *(undefined4 **)((int)_this + 8) = puVar5;
      puVar5[4] = 0;
      goto LAB_00423627;
    }
  }
  else {
    if (puVar4[3] != 0) {
      do {
        param_1 = puVar5;
        puVar5 = (undefined4 *)param_1[3];
      } while ((undefined4 *)param_1[3] != (undefined4 *)0x0);
      uVar2 = *param_1;
      uVar3 = param_1[1];
      FUN_004232f0(_this, (int *)&param_1);
      *puVar4 = uVar2;
      puVar4[1] = uVar3;
      return;
    }
    if (puVar4 == *(undefined4 **)((int)_this + 8)) {
      *(undefined4 **)((int)_this + 8) = puVar5;
      puVar5[4] = 0;
      goto LAB_00423627;
    }
  }
  iVar1 = puVar4[4];
  if (puVar4 == *(undefined4 **)(iVar1 + 0xc)) {
    *(undefined4 **)(iVar1 + 0xc) = puVar5;
  }
  else {
    *(undefined4 **)(iVar1 + 8) = puVar5;
  }
  puVar5[4] = iVar1;
LAB_00423627:
  if (puVar4 != (undefined4 *)0x0) {
    operator_delete((undefined *)puVar4);
  }
  *(int *)((int)_this + 4) = *(int *)((int)_this + 4) + -1;
  return;
}


// FUN_0043ff60 — BinTree_ClearHead
// Removes the BST root node, rebalancing children via FUN_004235d0/FUN_004232f0.
void __fastcall FUN_0043ff60(undefined4 *param_1)
{
  undefined4 *puVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 *puVar5;
  undefined4 *local_4;

  puVar1 = (undefined4 *)param_1[2];
  if (puVar1 == (undefined4 *)0x0) {
LAB_00440054:
    param_1[1] = 0;
    return;
  }
  local_4 = param_1;
  if ((undefined4 *)puVar1[2] != (undefined4 *)0x0) {
    FUN_004235d0(param_1, (undefined4 *)puVar1[2]);
  }
  if ((undefined4 *)puVar1[3] != (undefined4 *)0x0) {
    FUN_004235d0(param_1, (undefined4 *)puVar1[3]);
  }
  puVar5 = (undefined4 *)puVar1[2];
  local_4 = (undefined4 *)0x0;
  if (puVar5 == (undefined4 *)0x0) {
    puVar5 = (undefined4 *)puVar1[3];
    if (puVar5 == (undefined4 *)0x0) {
      iVar2 = puVar1[4];
      if (iVar2 == 0) {
        param_1[2] = 0;
      }
      else if (*(undefined4 **)(iVar2 + 8) == puVar1) {
        *(undefined4 *)(iVar2 + 8) = 0;
      }
      else {
        *(undefined4 *)(iVar2 + 0xc) = 0;
      }
      param_1[1] = param_1[1] + -1;
      if (puVar1 != (undefined4 *)0x0) {
        operator_delete((undefined *)puVar1);
      }
      param_1[2] = 0;
      goto LAB_00440054;
    }
    if (puVar1 == (undefined4 *)param_1[2]) {
      param_1[2] = (undefined4)(DWORD_PTR)puVar5;
      puVar5[4] = 0;
      goto LAB_0043ffbf;
    }
  }
  else {
    if (puVar1[3] != 0) {
      do {
        local_4 = puVar5;
        puVar5 = (undefined4 *)local_4[3];
      } while ((undefined4 *)local_4[3] != (undefined4 *)0x0);
      uVar3 = *local_4;
      uVar4 = local_4[1];
      FUN_004232f0(param_1, (int *)&local_4);
      *puVar1 = uVar3;
      puVar1[1] = uVar4;
      param_1[2] = 0;
      param_1[1] = 0;
      return;
    }
    if (puVar1 == (undefined4 *)param_1[2]) {
      param_1[2] = (undefined4)(DWORD_PTR)puVar5;
      puVar5[4] = 0;
      goto LAB_0043ffbf;
    }
  }
  iVar2 = puVar1[4];
  if (puVar1 == *(undefined4 **)(iVar2 + 0xc)) {
    *(undefined4 **)(iVar2 + 0xc) = puVar5;
  }
  else {
    *(undefined4 **)(iVar2 + 8) = puVar5;
  }
  puVar5[4] = iVar2;
LAB_0043ffbf:
  if (puVar1 != (undefined4 *)0x0) {
    operator_delete((undefined *)puVar1);
  }
  param_1[2] = 0;
  param_1[1] = param_1[1] + -1;
  param_1[1] = 0;
  return;
}


// FUN_0043f500 — PathFinder2_Solve  (__cdecl)
// A* pathfinder on a tile grid. Writes path into this->path_x/path_y arrays.
// param_1 = start tile X       param_2 = start tile Y (float)
// param_3 = target tile X      param_4 = target tile Y
// param_5 = walk-filter flag   param_6 = max walkability value to traverse
// param_7 = radius (0.0 = single target cell; else circle pre-mark)
// Returns: CONCAT31((len>>8),1) on success, 0 on failure.
uint __cdecl
FUN_0043f500(void *_this, int param_1, float param_2, int param_3, int param_4,
             int param_5, int param_6, float param_7)
{
  void *this_00;
  byte *pbVar1;
  byte bVar2;
  int iVar3;
  byte *pbVar4;
  int iVar5;
  int iVar6;
  uint uVar7;
  uint uVar8;
  int iVar9;
  undefined4 *puVar10;
  float fVar11;
  int iVar12;
  int iVar13;
  longlong lVar14;
  int *local_28;
  int local_24;
  int local_20;
  int local_1c;
  int local_18;
  float local_14;
  int local_10;
  int iStack_c;
  int iStack_8;
  int iStack_4;

  // Clear dirty range if not already sentinel
  iVar12 = *(int *)((int)_this + 0x400);
  if (iVar12 != 1950000000) {
    uVar7 = (*(int *)((int)_this + 0x404) - iVar12) + 1;
    puVar10 = (undefined4 *)(*(int *)((int)_this + 0x3fc) + iVar12);
    for (uVar8 = uVar7 >> 2; uVar8 != 0; uVar8 = uVar8 - 1) {
      *puVar10 = 0;
      puVar10 = puVar10 + 1;
    }
    for (uVar7 = uVar7 & 3; uVar7 != 0; uVar7 = uVar7 - 1) {
      *(undefined1 *)puVar10 = 0;
      puVar10 = (undefined4 *)((int)puVar10 + 1);
    }
    *(undefined4 *)((int)_this + 0x400) = 1950000000;
    *(undefined4 *)((int)_this + 0x404) = 0xffffffff;
  }

  pbVar4 = (byte *)0x0;
  if ((param_1 != 0) && (pbVar4 = (byte *)0x0, param_2 != 0.0f)) {
    if (_DAT_00552580 == param_7) {
      // Single-target mode: mark target cell directly
      pbVar4 = (byte *)(*(int *)_this * param_4);
      pbVar1 = pbVar4 + param_3;
      if (((((char)param_5 != '\0') && (param_6 <= (int)(uint)pbVar1[*(int *)((int)_this + 0xc)])) &&
          (pbVar4 = (byte *)(CONCAT31((int3)((uint)pbVar4 >> 8),
                                      pbVar4[param_3 + *(int *)((int)_this + 0xc)]) & 0xffffff20),
           (char)pbVar4 != ' ')) ||
         ((pbVar1[*(int *)((int)_this + 0x3fc)] = 4, (int)pbVar1 < 0 ||
          (*(int *)((int)_this + 8) <= (int)pbVar1)))) goto LAB_0043fd21;
      pbVar4 = *(byte **)((int)_this + 0x400);
      if ((int)pbVar1 < (int)*(byte **)((int)_this + 0x400)) {
        pbVar4 = pbVar1;
      }
      *(byte **)((int)_this + 0x400) = pbVar4;
      pbVar4 = *(byte **)((int)_this + 0x404);
      if ((int)*(byte **)((int)_this + 0x404) < (int)pbVar1) {
        pbVar4 = pbVar1;
      }
      *(byte **)((int)_this + 0x404) = pbVar4;
    }
    else {
      // Circle pre-mark mode
      lVar14 = (longlong)param_7;   // IDA sub_43F500: v15 = (__int64)a8  (radio del circle pre-mark)
      local_24 = (int)lVar14;
      uVar8 = -local_24;
      if (-local_24 == local_24 || (int)uVar8 < local_24) {
        iVar12 = uVar8 + param_4;
        local_14 = (float)uVar8;
        do {
          iVar13 = -(local_24 - ((uVar8 ^ (int)uVar8 >> 0x1f) - ((int)uVar8 >> 0x1f)));
          fVar11 = local_14;
          for (iVar5 = iVar13; local_14 = fVar11, iVar5 < 1; iVar5 = iVar5 + 1) {
            iVar6 = iVar12 * *(int *)_this + iVar5 + param_3;
            if ((iVar6 < 0) || (*(int *)((int)_this + 8) <= iVar6)) {
              FUN_00405540(&DAT_055c9bf0, &DAT_005597a0);
            }
            else if (((char)param_5 == '\0') ||
                    ((int)(uint)*(byte *)(*(int *)((int)_this + 0xc) + iVar6) < param_6)) {
              *(undefined1 *)(*(int *)((int)_this + 0x3fc) + iVar6) = 4;
              FUN_0043fd30(_this, iVar6);
            }
            iVar6 = (iVar12 * *(int *)_this - iVar5) + param_3;
            if ((iVar6 < 0) || (*(int *)((int)_this + 8) <= iVar6)) {
              FUN_00405540(&DAT_055c9bf0, &DAT_005597a0);
            }
            else if (((char)param_5 == '\0') ||
                    ((int)(uint)*(byte *)(*(int *)((int)_this + 0xc) + iVar6) < param_6)) {
              *(undefined1 *)(*(int *)((int)_this + 0x3fc) + iVar6) = 4;
              FUN_0043fd30(_this, iVar6);
            }
            fVar11 = local_14;
          }
          if ((int)fVar11 < iVar13) {
            local_18 = uVar8 * uVar8;
            do {
              local_1c = (int)fVar11 * (int)fVar11 + local_18;
              if (SQRT((float)local_1c) < param_7) {
                iVar5 = iVar12 * *(int *)_this + (int)fVar11 + param_3;
                if ((iVar5 < 0) || (*(int *)((int)_this + 8) <= iVar5)) {
                  FUN_00405540(&DAT_055c9bf0, &DAT_005597a0);
                }
                else if (((char)param_5 == '\0') ||
                        ((int)(uint)*(byte *)(*(int *)((int)_this + 0xc) + iVar5) < param_6)) {
                  *(undefined1 *)(*(int *)((int)_this + 0x3fc) + iVar5) = 4;
                  FUN_0043fd30(_this, iVar5);
                }
                iVar5 = (iVar12 * *(int *)_this - (int)fVar11) + param_3;
                if ((iVar5 < 0) || (*(int *)((int)_this + 8) <= iVar5)) {
                  FUN_00405540(&DAT_055c9bf0, &DAT_005597a0);
                }
                else if (((char)param_5 == '\0') ||
                        ((int)(uint)*(byte *)(*(int *)((int)_this + 0xc) + iVar5) < param_6)) {
                  *(undefined1 *)(*(int *)((int)_this + 0x3fc) + iVar5) = 4;
                  FUN_0043fd30(_this, iVar5);
                }
              }
              fVar11 = (float)((int)fVar11 + 1);
            } while ((int)fVar11 < iVar13);
          }
          uVar8 = uVar8 + 1;
          iVar12 = iVar12 + 1;
        } while ((int)uVar8 <= local_24);
      }
    }

    // A* main loop setup
    iVar12 = *(int *)_this * (int)param_2 + param_1;
    local_18 = param_1;
    local_20 = 1950000000;
    local_14 = param_2;
    FUN_0043fea0((undefined4 *)((int)_this + 0x414), iVar12, 0);
    pbVar4 = (byte *)(*(int *)((int)_this + 0x3fc) + iVar12);
    *pbVar4 = *(byte *)(*(int *)((int)_this + 0x3fc) + iVar12) | 1;
    if ((-1 < iVar12) && (iVar12 < *(int *)((int)_this + 8))) {
      iVar5 = *(int *)((int)_this + 0x400);
      if (iVar12 < *(int *)((int)_this + 0x400)) {
        iVar5 = iVar12;
      }
      *(int *)((int)_this + 0x400) = iVar5;
      if (iVar12 <= *(int *)((int)_this + 0x404)) {
        iVar12 = *(int *)((int)_this + 0x404);
      }
      *(int *)((int)_this + 0x404) = iVar12;

      iVar12 = (-(uint)((char)param_5 != '\0') & 0x1c2) + 0x32;
      local_1c = iVar12;
      local_10 = iVar12;
      iVar5 = (*(code *)**(undefined4 **)((int)_this + 0x414))();

      // A* search loop
      while ((0 < iVar5 && (0 < iVar12))) {
        local_24 = 0;
        iVar5 = *(int *)((int)_this + 0x41c);
        while (iVar13 = iVar5, iVar13 != 0) {
          local_24 = iVar13;
          iVar5 = *(int *)(iVar13 + 8);
        }
        iStack_4 = FUN_004232f0((void *)((int)_this + 0x414), &local_24);
        iVar5 = iStack_4 % *(int *)_this;
        param_7 = (float)(iStack_4 / *(int *)_this);
        *(uint *)(*(int *)((int)_this + 0x408) + iStack_4 * 4) =
             -(uint)(iVar12 != local_10) & 1950000000;

        // Check 8 directions
        local_28 = (int *)ASTAR_DIR_START;
        do {
          iStack_c = local_28[-1];  // dx
          iStack_8 = *local_28;    // dy
          iVar13 = iStack_c + iVar5;
          iVar12 = iStack_8 + (int)param_7;
          if ((((-1 < iVar13) && (-1 < iVar12)) && (iVar13 < *(int *)_this)) &&
             ((iVar12 < *(int *)((int)_this + 4) &&
              (iVar6 = *(int *)_this * iVar12 + iVar13,
              (*(byte *)(*(int *)((int)_this + 0x3fc) + iVar6) & 2) != 0)))) {
            if ((iStack_c == 0) || (iVar9 = 7, iStack_8 == 0)) {
              iVar9 = 5;
            }
            iVar3 = *(int *)((int)_this + 0x408);
            iVar9 = *(int *)(iVar3 + iVar6 * 4) + iVar9;
            if (iVar9 < *(int *)(iVar3 + iStack_4 * 4)) {
              *(int *)(iVar3 + iStack_4 * 4) = iVar9;
              *(int *)(*(int *)((int)_this + 0x40c) + iStack_4 * 4) = iVar13;
              *(int *)(*(int *)((int)_this + 0x410) + iStack_4 * 4) = iVar12;
            }
          }
          local_28 = local_28 + 2;
        } while (local_28 < ASTAR_DIR_END);

        *(byte *)(*(int *)((int)_this + 0x3fc) + iStack_4) =
             *(byte *)(*(int *)((int)_this + 0x3fc) + iStack_4) | 2;

        if ((*(byte *)(*(int *)((int)_this + 0x3fc) + iStack_4) & 4) != 0) {
          // Found target — trace back path
          param_5 = *(int *)((int)_this + 0x41c);
          this_00 = (void *)((int)_this + 0x414);
          if (param_5 != 0) {
            if (*(undefined4 **)(param_5 + 8) != (undefined4 *)0x0) {
              FUN_004235d0(this_00, *(undefined4 **)(param_5 + 8));
            }
            if (*(undefined4 **)(param_5 + 0xc) != (undefined4 *)0x0) {
              FUN_004235d0(this_00, *(undefined4 **)(param_5 + 0xc));
            }
            FUN_004232f0(this_00, &param_5);
            *(undefined4 *)((int)_this + 0x41c) = 0;
          }
          *(undefined4 *)((int)_this + 0x418) = 0;
          *(undefined4 *)((int)_this + 0x10) = 0;
          while (true) {
            *(char *)((int)_this + (0x207 - *(int *)((int)_this + 0x10))) = (char)iVar5;
            *(char *)((int)_this + (0x3fb - *(int *)((int)_this + 0x10))) = (char)(int)param_7;
            if ((iVar5 == param_1) && (param_7 == param_2)) break;
            iVar12 = (int)param_7 * *(int *)_this + iVar5;
            iVar13 = *(int *)((int)_this + 0x10) + 1;
            iVar5 = *(int *)(*(int *)((int)_this + 0x40c) + iVar12 * 4);
            param_7 = *(float *)(*(int *)((int)_this + 0x410) + iVar12 * 4);
            *(int *)((int)_this + 0x10) = iVar13;
            if (499 < iVar13) {
              return (uint)param_7 & 0xffffff00;
            }
          }
          goto LAB_0043fc4d;
        }

        // Expand neighbours
        local_28 = (int *)ASTAR_DIR_START;
        do {
          iVar12 = local_28[-1] + iVar5;
          iVar13 = *local_28 + (int)param_7;
          if ((((-1 < iVar12) && (-1 < iVar13)) && (iVar12 < *(int *)_this)) &&
             (iVar13 < *(int *)((int)_this + 4))) {
            iVar6 = *(int *)_this * iVar13 + iVar12;
            bVar2 = *(byte *)(*(int *)((int)_this + 0xc) + iVar6);
            uVar8 = (uint)bVar2;
            if ((bVar2 & 0x20) == 0x20) {
              uVar8 = uVar8 - 0x20;
            }
            if (((*(byte *)(*(int *)((int)_this + 0x3fc) + iVar6) & 1) == 0) &&
               ((int)uVar8 < param_6)) {
              uVar8 = iVar12 - param_3 >> 0x1f;
              iVar12 = (iVar12 - param_3 ^ uVar8) - uVar8;
              uVar8 = iVar13 - param_4 >> 0x1f;
              iVar13 = (iVar13 - param_4 ^ uVar8) - uVar8;

              if ((iVar12 == 1) && (iVar13 == 1)) {
                iVar13 = 0;
LAB_0043fa38:
                iVar9 = iVar13;
              }
              else {
                iVar9 = iVar12;
                if (iVar13 <= iVar12) goto LAB_0043fa38;
              }
              uVar8 = iVar12 - iVar13 >> 0x1f;
              iVar12 = ((iVar12 - iVar13 ^ uVar8) - uVar8) * 0xf + 3 + iVar9 * 0x15;
              FUN_0043fea0((void *)((int)_this + 0x414), iVar6,
                           ((int)(iVar12 + (iVar12 >> 0x1f & 3U)) >> 2) +
                           *(int *)(*(int *)((int)_this + 0x408) + iStack_4 * 4));
              pbVar4 = (byte *)(*(int *)((int)_this + 0x3fc) + iVar6);
              *pbVar4 = *pbVar4 | 1;
              if ((iVar6 < 0) || (*(int *)((int)_this + 8) <= iVar6)) goto LAB_0043fd21;
              iVar12 = *(int *)((int)_this + 0x400);
              if (iVar6 < *(int *)((int)_this + 0x400)) {
                iVar12 = iVar6;
              }
              *(int *)((int)_this + 0x400) = iVar12;
              iVar12 = *(int *)((int)_this + 0x404);
              if (*(int *)((int)_this + 0x404) < iVar6) {
                iVar12 = iVar6;
              }
              *(int *)((int)_this + 0x404) = iVar12;
              *(int *)(*(int *)((int)_this + 0x40c) + iVar6 * 4) = iVar5;
              *(float *)(*(int *)((int)_this + 0x410) + iVar6 * 4) = param_7;
            }
          }
          local_28 = local_28 + 2;
        } while (local_28 < ASTAR_DIR_END);

        if ((char)param_5 == '\0') {
          uVar8 = iVar5 - param_3 >> 0x1f;
          iVar12 = (iVar5 - param_3 ^ uVar8) - uVar8;
          uVar8 = (int)param_7 - param_4 >> 0x1f;
          iVar13 = ((int)param_7 - param_4 ^ uVar8) - uVar8;
          if ((iVar12 == 1) && (iVar13 == 1)) {
            iVar13 = 0;
LAB_0043fb32:
            iVar6 = iVar13;
          }
          else {
            iVar6 = iVar12;
            if (iVar13 <= iVar12) goto LAB_0043fb32;
          }
          uVar8 = iVar12 - iVar13 >> 0x1f;
          iVar12 = ((iVar12 - iVar13 ^ uVar8) - uVar8) * 0xf + 3 + iVar6 * 0x15;
          iVar12 = (int)(iVar12 + (iVar12 >> 0x1f & 3U)) >> 2;
          if (iVar12 < local_20) {
            local_14 = param_7;
            local_20 = iVar12;
            local_18 = iVar5;
          }
        }
        iVar12 = local_1c + -1;
        local_1c = iVar12;
        iVar5 = (*(code *)**(undefined4 **)((int)_this + 0x414))();
      }

      puVar10 = (undefined4 *)((int)_this + 0x414);
      if ((char)param_5 == '\0') {
        param_5 = *(int *)((int)_this + 0x41c);
        if (param_5 != 0) {
          if (*(undefined4 **)(param_5 + 8) != (undefined4 *)0x0) {
            FUN_004235d0(puVar10, *(undefined4 **)(param_5 + 8));
          }
          if (*(undefined4 **)(param_5 + 0xc) != (undefined4 *)0x0) {
            FUN_004235d0(puVar10, *(undefined4 **)(param_5 + 0xc));
          }
          FUN_004232f0(puVar10, &param_5);
          *(undefined4 *)((int)_this + 0x41c) = 0;
        }
        *(undefined4 *)((int)_this + 0x418) = 0;
        *(undefined4 *)((int)_this + 0x10) = 0;
        while (true) {
          *(char *)((int)_this + (0x207 - *(int *)((int)_this + 0x10))) = (char)local_18;
          *(char *)((int)_this + (0x3fb - *(int *)((int)_this + 0x10))) = (char)(int)local_14;
          if ((local_18 == param_1) && (local_14 == param_2)) break;
          iVar12 = (int)local_14 * *(int *)_this + local_18;
          iVar5 = *(int *)((int)_this + 0x10) + 1;
          local_18 = *(int *)(*(int *)((int)_this + 0x40c) + iVar12 * 4);
          local_14 = *(float *)(*(int *)((int)_this + 0x410) + iVar12 * 4);
          *(int *)((int)_this + 0x10) = iVar5;
          if (499 < iVar5) {
            return (uint)local_14 & 0xffffff00;
          }
        }
LAB_0043fc4d:
        iVar12 = *(int *)((int)_this + 0x10) + 1;
        *(int *)((int)_this + 0x10) = iVar12;
        return CONCAT31((int3)((uint)iVar12 >> 8), 1);
      }
      FUN_0043ff60((undefined4 *)puVar10);
      pbVar4 = nullptr;
    }
  }
LAB_0043fd21:
  return (uint)pbVar4 & 0xffffff00;
}

// ─────────────────────────────────────────────────────────────────────────────
// CONSTRUCCION E INICIALIZACION DEL CONTEXTO PATH  (2026-08-17)
//
// Hasta ahora el contexto (DAT_05826df4) se reservaba en WinMain con
// `malloc(0x420)` + memset, y por eso el vtable de la cola de prioridad en
// +0x414 quedaba NULL: FUN_0043f500 (PATH::FindPath) crashea al llamarlo, y de
// ahi venia el `pfReady = false` forzado en stubs_externs.cpp, que obliga a usar
// el A* sustituto. Estas dos funciones portan lo que faltaba.
//
// ── Layout del contexto, leido del binario ───────────────────────────────────
//   indice DWORD (byte)      contenido
//   [0]     +0x000           ancho del grid   = 0x100
//   [1]     +0x004           alto del grid    = 0x100
//   [2]     +0x008           celdas totales   = 0x10000
//   [3]     +0x00c           puntero a TerrainWall
//   [4]     +0x010           largo del camino resuelto
//   [0xff]  +0x3fc           mapa de flags (0x10000 bytes)
//   [0x100] +0x400           rango sucio: min (centinela 1950000000)
//   [0x101] +0x404           rango sucio: max (centinela -1)
//   [0x102] +0x408           mapa de costes   (0x10000 DWORDs)
//   [0x103] +0x40c           padre X          (0x10000 DWORDs)
//   [0x104] +0x410           padre Y          (0x10000 DWORDs)
//   [0x105] +0x414           vtable de la cola de prioridad
//   [0x106] +0x418           cantidad de nodos en la cola
//   [0x107] +0x41c           raiz del arbol de la cola
//   ademas: [+0x207..] path_x  y  [+0x3fb..] path_y
//
// ── El vtable ────────────────────────────────────────────────────────────────
// El ctor escribe `MOV dword ptr [EAX + 0x414], 0x552840`, y en 0x00552840 hay
// un unico puntero: 0x00422DE0. Esa direccion no esta definida como funcion en
// el Ghidra, pero sus bytes son `8B 41 04 C3`:
//     MOV EAX, [ECX+4]
//     RET
// o sea un `__thiscall int GetCount()` que devuelve el campo en this+4. Con
// this = contexto+0x414, ese campo es +0x418: la cantidad de nodos de la cola.
// Encaja con el uso en FindPath: `iVar2 = (**(code **)in_ECX[0x105])();` seguido
// de `while ((0 < iVar2 && (0 < iVar10)))`.
extern "C" int __fastcall PathPQueue_GetCount(void *_this, void * /*edx*/)
{
    return *(int *)((char *)_this + 4);
}

// El vtable de una sola entrada que vive en 0x00552840 en el binario.
static void *s_PathPQueueVtbl[1] = { (void *)&PathPQueue_GetCount };

// Ctor del contexto — binario 0x0043F280..0x0043F2C7 (no esta definido como
// funcion en el Ghidra, va inline antes de InitPath):
//     PUSH 0x424 ; CALL operator_new ; XOR ECX,ECX ; CMP EAX,ECX ; JZ fail
//     MOV [EAX+0x414], 0x552840
//     MOV [EAX+0x418], ECX     MOV [EAX+0x41c], ECX
//     MOV [EAX+0x3fc], ECX     MOV [EAX+0x408], ECX
//     MOV [EAX+0x40c], ECX     MOV [EAX+0x410], ECX
//     MOV [0x05826df4], EAX
// Ojo con el tamano: son 0x424 bytes, no 0x420. Con 0x420 el ultimo campo del
// contexto (+0x41c, la raiz del arbol) cae fuera de la reserva.
void __cdecl PathContext_Create(void)
{
    DWORD *p = (DWORD *)operator_new(0x424);
    if (p == nullptr) {
        DAT_05826df4 = 0;
        return;
    }
    p[0x105] = (DWORD)(uintptr_t)s_PathPQueueVtbl;   // +0x414 vtable
    p[0x106] = 0;                                    // +0x418 count
    p[0x107] = 0;                                    // +0x41c root
    p[0xff]  = 0;                                    // +0x3fc flagmap
    p[0x102] = 0;                                    // +0x408 costmap
    p[0x103] = 0;                                    // +0x40c parentX
    p[0x104] = 0;                                    // +0x410 parentY
    DAT_05826df4 = (DWORD)(uintptr_t)p;
}

// ZzzAI::InitPath (0x0043F2D0) NO va aca: ya estaba portada, y correctamente,
// en stubs_externs.cpp (PathFinder_ResetContext). La llama FUN_0050f690 (World_Init) desde
// Scene_Intro, igual que en el binario. Definirla de nuevo aca daba LNK2005.
