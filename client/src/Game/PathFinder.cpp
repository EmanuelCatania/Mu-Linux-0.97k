#include <cstdint>
// PathFinder.cpp
// A* pathfinder and BST priority-queue helpers.
//
// IDA: PATH_AddClearPos (0x0043FD30)
// IDA: CBTree_Add (0x0043FEA0)
// IDA: CBTree_RemoveNode (0x004232F0)
// IDA: CBTree_RemoveFrom (0x004235D0)
// IDA: CBTree_RemoveAll (0x0043FF60)
// PATH_FindPath @ 0x0043F500  — PathFinder2_Solve     (340 lines)
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
#include "Game/PathContext.h"
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


// PATH::AddClearPos de ZzzPath.h.
// Amplía el rango sucio [DirtyMin..DirtyMax] para incluir param_1.
undefined4 __cdecl PATH_AddClearPos(PathContext *ctx, int param_1)
{
  if ((-1 < param_1) && (param_1 < ctx->Size)) {
    if (param_1 < ctx->DirtyMin) ctx->DirtyMin = param_1;
    if (param_1 > ctx->DirtyMax) ctx->DirtyMax = param_1;
    return CONCAT31((int3)((uint)ctx->DirtyMax >> 8), 1);
  }
  return param_1 & 0xffffff00;
}


static PathNode *NewPathNode(int data, int cost)
{
  auto *node = static_cast<PathNode *>(operator_new(sizeof(PathNode)));
  if (node != nullptr) {
    node->Parent = nullptr;
    node->Right = nullptr;
    node->Left = nullptr;
    node->Data = data;
    node->Cost = cost;
  }
  return node;
}


// Add de CBTree, usado por PATH::FindPath. Inserta (data, cost) ordenado por cost.
void __cdecl CBTree_Add(PathQueue *queue, int data, int cost)
{
  PathNode *parent = queue->Root;
  if (parent == nullptr) {
    queue->Count++;
    queue->Root = NewPathNode(data, cost);
    return;
  }
  for (PathNode *node = parent; node != nullptr;) {
    parent = node;
    node = (cost < node->Cost) ? node->Left : node->Right;
  }
  queue->Count++;
  PathNode *node = NewPathNode(data, cost);
  if (cost < parent->Cost) parent->Left = node;
  else                     parent->Right = node;
  node->Parent = parent;
}


// Saca `victim` del árbol enganchando su único hijo al padre.
static void ReplaceWithChild(PathQueue *queue, PathNode *victim, PathNode *child)
{
  if (victim == queue->Root) {
    queue->Root = child;
    child->Parent = nullptr;
    return;
  }
  PathNode *parent = victim->Parent;
  if (victim == parent->Right) parent->Right = child;
  else                         parent->Left = child;
  child->Parent = parent;
}


static void DetachLeaf(PathQueue *queue, PathNode *leaf)
{
  PathNode *parent = leaf->Parent;
  if (parent == nullptr)          queue->Root = nullptr;
  else if (parent->Left == leaf)  parent->Left = nullptr;
  else                            parent->Right = nullptr;
}


static PathNode *RightmostOf(PathNode *node)
{
  while (node->Right != nullptr) node = node->Right;
  return node;
}


// RemoveNode de CBTree, usado por PATH::FindPath. Devuelve el dato del nodo.
int __cdecl CBTree_RemoveNode(PathQueue *queue, PathNode **slot)
{
  PathNode *node = *slot;
  const int data = node->Data;
  PathNode *child = node->Left;
  if (child == nullptr) {
    child = node->Right;
    if (child == nullptr) {
      DetachLeaf(queue, node);
      queue->Count--;
      if (*slot != nullptr) operator_delete(*slot);
      *slot = nullptr;
      return data;
    }
  } else if (node->Right != nullptr) {
    PathNode *replacement = RightmostOf(child);
    const int replacementData = replacement->Data;
    const int replacementCost = replacement->Cost;
    CBTree_RemoveNode(queue, &replacement);
    (*slot)->Data = replacementData;
    (*slot)->Cost = replacementCost;
    return data;
  }
  ReplaceWithChild(queue, node, child);
  if (*slot != nullptr) operator_delete(*slot);
  queue->Count--;
  return data;
}


// CBTree::RemoveFrom de BaseCls.h. Borra un nodo y todos sus hijos.
void __cdecl CBTree_RemoveFrom(PathQueue *queue, PathNode *node)
{
  if (node->Left != nullptr) CBTree_RemoveFrom(queue, node->Left);
  if (node->Right != nullptr) CBTree_RemoveFrom(queue, node->Right);
  PathNode *self = node;
  CBTree_RemoveNode(queue, &self);
}


// RemoveAll de CBTree, usado por PATH::FindPath.
void __cdecl CBTree_RemoveAll(PathQueue *queue)
{
  if (queue->Root != nullptr) {
    CBTree_RemoveFrom(queue, queue->Root);
    queue->Root = nullptr;
  }
  queue->Count = 0;
}


// IDA: PATH_FindPath (0x0043F500)
// A* pathfinder on a tile grid. Writes path into this->path_x/path_y arrays.
// param_1 = start tile X       param_2 = start tile Y (float)
// param_3 = target tile X      param_4 = target tile Y
// param_5 = walk-filter flag   param_6 = max walkability value to traverse
// param_7 = radius (0.0 = single target cell; else circle pre-mark)
// Returns: CONCAT31((len>>8),1) on success, 0 on failure.
uint __cdecl
PATH_FindPath(PathContext *ctx, int param_1, float param_2, int param_3, int param_4,
             int param_5, int param_6, float param_7)
{
  PathQueue *this_00;
  int pbVar1;
  byte bVar2;
  uint failValue;
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
  PathNode *root;
  PathNode *subtree;
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
  iVar12 = ctx->DirtyMin;
  if (iVar12 != 1950000000) {
    uVar7 = (ctx->DirtyMax - iVar12) + 1;
    puVar10 = reinterpret_cast<undefined4 *>(ctx->Flags + iVar12);
    for (uVar8 = uVar7 >> 2; uVar8 != 0; uVar8 = uVar8 - 1) {
      *puVar10 = 0;
      puVar10 = puVar10 + 1;
    }
    for (uVar7 = uVar7 & 3; uVar7 != 0; uVar7 = uVar7 - 1) {
      *(undefined1 *)puVar10 = 0;
      puVar10 = reinterpret_cast<undefined4 *>(reinterpret_cast<char *>(puVar10) + 1);
    }
    ctx->DirtyMin = 1950000000;
    ctx->DirtyMax = -1;
  }

  failValue = 0;
  if ((param_1 != 0) && (param_2 != 0.0f)) {
    if (_DAT_00552580 == param_7) {
      // Single-target mode: mark target cell directly
      failValue = (uint)(ctx->Width * param_4);
      pbVar1 = (int)failValue + param_3;
      if (((char)param_5 != '\0') && (param_6 <= (int)(uint)ctx->Walk[pbVar1])) {
        failValue = CONCAT31((int3)(failValue >> 8), ctx->Walk[pbVar1]) & 0xffffff20;
        if ((char)failValue != ' ') goto LAB_0043fd21;
      }
      ctx->Flags[pbVar1] = 4;
      if ((pbVar1 < 0) || (ctx->Size <= pbVar1)) goto LAB_0043fd21;
      failValue = (uint)ctx->DirtyMin;
      if (pbVar1 < ctx->DirtyMin) failValue = (uint)pbVar1;
      ctx->DirtyMin = (int)failValue;
      failValue = (uint)ctx->DirtyMax;
      if (ctx->DirtyMax < pbVar1) failValue = (uint)pbVar1;
      ctx->DirtyMax = (int)failValue;
    }
    else {
      // Circle pre-mark mode
      lVar14 = (longlong)param_7;   // IDA sub_43F500: v15 = (std::int64_t)a8  (radio del circle pre-mark)
      local_24 = (int)lVar14;
      uVar8 = -local_24;
      if (-local_24 == local_24 || (int)uVar8 < local_24) {
        iVar12 = uVar8 + param_4;
        local_14 = (float)uVar8;
        do {
          iVar13 = -(local_24 - ((uVar8 ^ (int)uVar8 >> 0x1f) - ((int)uVar8 >> 0x1f)));
          fVar11 = local_14;
          for (iVar5 = iVar13; local_14 = fVar11, iVar5 < 1; iVar5 = iVar5 + 1) {
            iVar6 = iVar12 * ctx->Width + iVar5 + param_3;
            if ((iVar6 < 0) || (ctx->Size <= iVar6)) {
              CErrorReport_Write(&DAT_055c9bf0, &DAT_005597a0);
            }
            else if (((char)param_5 == '\0') ||
                    ((int)(uint)ctx->Walk[iVar6] < param_6)) {
              ctx->Flags[iVar6] = 4;
              PATH_AddClearPos(ctx, iVar6);
            }
            iVar6 = (iVar12 * ctx->Width - iVar5) + param_3;
            if ((iVar6 < 0) || (ctx->Size <= iVar6)) {
              CErrorReport_Write(&DAT_055c9bf0, &DAT_005597a0);
            }
            else if (((char)param_5 == '\0') ||
                    ((int)(uint)ctx->Walk[iVar6] < param_6)) {
              ctx->Flags[iVar6] = 4;
              PATH_AddClearPos(ctx, iVar6);
            }
            fVar11 = local_14;
          }
          if ((int)fVar11 < iVar13) {
            local_18 = uVar8 * uVar8;
            do {
              local_1c = (int)fVar11 * (int)fVar11 + local_18;
              if (SQRT((float)local_1c) < param_7) {
                iVar5 = iVar12 * ctx->Width + (int)fVar11 + param_3;
                if ((iVar5 < 0) || (ctx->Size <= iVar5)) {
                  CErrorReport_Write(&DAT_055c9bf0, &DAT_005597a0);
                }
                else if (((char)param_5 == '\0') ||
                        ((int)(uint)ctx->Walk[iVar5] < param_6)) {
                  ctx->Flags[iVar5] = 4;
                  PATH_AddClearPos(ctx, iVar5);
                }
                iVar5 = (iVar12 * ctx->Width - (int)fVar11) + param_3;
                if ((iVar5 < 0) || (ctx->Size <= iVar5)) {
                  CErrorReport_Write(&DAT_055c9bf0, &DAT_005597a0);
                }
                else if (((char)param_5 == '\0') ||
                        ((int)(uint)ctx->Walk[iVar5] < param_6)) {
                  ctx->Flags[iVar5] = 4;
                  PATH_AddClearPos(ctx, iVar5);
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
    iVar12 = ctx->Width * (int)param_2 + param_1;
    local_18 = param_1;
    local_20 = 1950000000;
    local_14 = param_2;
    CBTree_Add(&ctx->Queue, iVar12, 0);
    ctx->Flags[iVar12] |= 1;
    failValue = (uint)(uintptr_t)(ctx->Flags + iVar12);
    if ((-1 < iVar12) && (iVar12 < ctx->Size)) {
      iVar5 = ctx->DirtyMin;
      if (iVar12 < ctx->DirtyMin) {
        iVar5 = iVar12;
      }
      ctx->DirtyMin = iVar5;
      if (iVar12 <= ctx->DirtyMax) {
        iVar12 = ctx->DirtyMax;
      }
      ctx->DirtyMax = iVar12;

      iVar12 = (-(uint)((char)param_5 != '\0') & 0x1c2) + 0x32;
      local_1c = iVar12;
      local_10 = iVar12;
      iVar5 = ctx->Queue.Count;

      // A* search loop
      while ((0 < iVar5 && (0 < iVar12))) {
        root = nullptr;
        for (PathNode *node = ctx->Queue.Root; node != nullptr; node = node->Left) root = node;
        iStack_4 = CBTree_RemoveNode(&ctx->Queue, &root);
        iVar5 = iStack_4 % ctx->Width;
        param_7 = (float)(iStack_4 / ctx->Width);
        ctx->Cost[iStack_4] = (int)(-(uint)(iVar12 != local_10) & 1950000000);

        // Check 8 directions
        local_28 = (int *)ASTAR_DIR_START;
        do {
          iStack_c = local_28[-1];  // dx
          iStack_8 = *local_28;    // dy
          iVar13 = iStack_c + iVar5;
          iVar12 = iStack_8 + (int)param_7;
          if ((((-1 < iVar13) && (-1 < iVar12)) && (iVar13 < ctx->Width)) &&
             ((iVar12 < ctx->Height &&
              (iVar6 = ctx->Width * iVar12 + iVar13,
              (ctx->Flags[iVar6] & 2) != 0)))) {
            if ((iStack_c == 0) || (iVar9 = 7, iStack_8 == 0)) {
              iVar9 = 5;
            }
            iVar9 = ctx->Cost[iVar6] + iVar9;
            if (iVar9 < ctx->Cost[iStack_4]) {
              ctx->Cost[iStack_4] = iVar9;
              ctx->ParentX[iStack_4] = iVar13;
              ctx->ParentY[iStack_4] = iVar12;
            }
          }
          local_28 = local_28 + 2;
        } while (local_28 < ASTAR_DIR_END);

        ctx->Flags[iStack_4] =
             ctx->Flags[iStack_4] | 2;

        if ((ctx->Flags[iStack_4] & 4) != 0) {
          // Found target — trace back path
          // IDA reusa param_5 como raíz; el valor no vuelve a leerse en esta rama.
          this_00 = &ctx->Queue;
          subtree = ctx->Queue.Root;
          if (subtree != nullptr) {
            if (subtree->Left != nullptr) CBTree_RemoveFrom(this_00, subtree->Left);
            if (subtree->Right != nullptr) CBTree_RemoveFrom(this_00, subtree->Right);
            CBTree_RemoveNode(this_00, &subtree);
            ctx->Queue.Root = nullptr;
          }
          ctx->Queue.Count = 0;
          ctx->Length = 0;
          while (true) {
            ctx->PathX[499 - ctx->Length] = (char)iVar5;
            ctx->PathY[499 - ctx->Length] = (char)(int)param_7;
            if ((iVar5 == param_1) && (param_7 == param_2)) break;
            iVar12 = (int)param_7 * ctx->Width + iVar5;
            iVar13 = ctx->Length + 1;
            iVar5 = ctx->ParentX[iVar12];
            param_7 = *reinterpret_cast<float *>(&ctx->ParentY[iVar12]);
            ctx->Length = iVar13;
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
          if ((((-1 < iVar12) && (-1 < iVar13)) && (iVar12 < ctx->Width)) &&
             (iVar13 < ctx->Height)) {
            iVar6 = ctx->Width * iVar13 + iVar12;
            bVar2 = ctx->Walk[iVar6];
            uVar8 = (uint)bVar2;
            if ((bVar2 & 0x20) == 0x20) {
              uVar8 = uVar8 - 0x20;
            }
            if (((ctx->Flags[iVar6] & 1) == 0) &&
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
              CBTree_Add(&ctx->Queue, iVar6,
                           ((int)(iVar12 + (iVar12 >> 0x1f & 3U)) >> 2) +
                           ctx->Cost[iStack_4]);
              ctx->Flags[iVar6] |= 1;
              failValue = (uint)(uintptr_t)(ctx->Flags + iVar6);
              if ((iVar6 < 0) || (ctx->Size <= iVar6)) goto LAB_0043fd21;
              iVar12 = ctx->DirtyMin;
              if (iVar6 < ctx->DirtyMin) {
                iVar12 = iVar6;
              }
              ctx->DirtyMin = iVar12;
              iVar12 = ctx->DirtyMax;
              if (ctx->DirtyMax < iVar6) {
                iVar12 = iVar6;
              }
              ctx->DirtyMax = iVar12;
              ctx->ParentX[iVar6] = iVar5;
              *reinterpret_cast<float *>(&ctx->ParentY[iVar6]) = param_7;
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
        iVar5 = ctx->Queue.Count;
      }

      if ((char)param_5 == '\0') {
        this_00 = &ctx->Queue;
        subtree = ctx->Queue.Root;
        if (subtree != nullptr) {
          if (subtree->Left != nullptr) CBTree_RemoveFrom(this_00, subtree->Left);
          if (subtree->Right != nullptr) CBTree_RemoveFrom(this_00, subtree->Right);
          CBTree_RemoveNode(this_00, &subtree);
          ctx->Queue.Root = nullptr;
        }
        ctx->Queue.Count = 0;
        ctx->Length = 0;
        while (true) {
          ctx->PathX[499 - ctx->Length] = (char)local_18;
          ctx->PathY[499 - ctx->Length] = (char)(int)local_14;
          if ((local_18 == param_1) && (local_14 == param_2)) break;
          iVar12 = (int)local_14 * ctx->Width + local_18;
          iVar5 = ctx->Length + 1;
          local_18 = ctx->ParentX[iVar12];
          local_14 = *reinterpret_cast<float *>(&ctx->ParentY[iVar12]);
          ctx->Length = iVar5;
          if (499 < iVar5) {
            return (uint)local_14 & 0xffffff00;
          }
        }
LAB_0043fc4d:
        iVar12 = ctx->Length + 1;
        ctx->Length = iVar12;
        return CONCAT31((int3)((uint)iVar12 >> 8), 1);
      }
      CBTree_RemoveAll(&ctx->Queue);
      failValue = 0;
    }
  }
LAB_0043fd21:
  return failValue & 0xffffff00;
}

// ─────────────────────────────────────────────────────────────────────────────
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
// Ctor del contexto — binario 0x0043F280..0x0043F2C7.
// La vtable de IDA sólo expone GetCount; acá la cuenta se lee del campo.
void __cdecl PathContext_Create(void)
{
    auto *ctx = static_cast<PathContext *>(operator_new(sizeof(PathContext)));
    if (ctx == nullptr) {
        DAT_05826df4 = nullptr;
        return;
    }
    ctx->Queue.Count = 0;
    ctx->Queue.Root = nullptr;
    ctx->Flags = nullptr;
    ctx->Cost = nullptr;
    ctx->ParentX = nullptr;
    ctx->ParentY = nullptr;
    DAT_05826df4 = ctx;
}

// ZzzAI::InitPath (0x0043F2D0) no va acá: es PathFinder_ResetContext, que la
// llama OpenFont (World_Init) desde Scene_Intro, igual que en el binario.
// Definirla de nuevo daría LNK2005.
