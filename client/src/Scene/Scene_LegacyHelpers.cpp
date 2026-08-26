// Scene_LegacyHelpers.cpp
// Extracted from stubs_misc_helpers.cpp; IDA provenance comments retained.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

// FUN_0040a660 @ 0x0040A660 — WidgetB_CtorFull: construct two linked WidgetB objects.
// Calls FUN_00409ed0 on self, sets vtable, then constructs a local WidgetB and copies it.
void* __fastcall FUN_0040a660(void *param_1)
{
    FUN_00409ed0(param_1);
    // vtable = &PTR_FUN_00552588 — skipped

    // local_2c is a stack-local WidgetB (0x20 bytes, 8 dwords)
    int local_2c[8] = {};
    FUN_00409ed0(local_2c);
    FUN_00409f10(local_2c);
    return param_1;
}

// FUN_0040a860 @ 0x0040A860 — VTable_Release
// Iterates object->edge_list and calls FUN_0040a8f0 on each edge.
void __cdecl FUN_0040a860(void *obj, int param_1)
{
    char *this_ = (char*)obj;
    int count = *(int*)(this_ + 0x18);
    if (count <= 0) return;
    for (int i = 0; i < count; i++) {
        short *psVar1 = (short*)(*(int*)(this_ + 0x1c) + i * 10);
        int iVar2 = (int)psVar1[2] * 15000;
        float *p1 = (float*)(param_1 + (*psVar1 + iVar2) * 12);
        float *p2 = (float*)(param_1 + (psVar1[1] + iVar2) * 12);
        FUN_0040a8f0(obj, p1, p2);
    }
}

// ── Scene entity grid helpers ─────────────────────────────────────────────────
// FUN_004ffcc0 @ 0x004FFCC0 — Entity_GridUnlink(node, cell_ptr)
// Unlinks node from its doubly-linked list in the 16x16 scene grid.
void __cdecl FUN_004ffcc0(void *param_1_v, int param_2) {
    char *param_1 = (char*)param_1_v;
    if (!param_1) return;
    int iVar1 = *(int*)(param_1 + 0x1b8); // next
    int iVar2 = *(int*)(param_1 + 0x1b4); // prev
    if (iVar1 != 0) {
        if (iVar2 != 0) {
            *(int*)(iVar2 + 0x1b8) = iVar1;
            *(int*)(iVar1 + 0x1b4) = *(int*)(param_1 + 0x1b4);
            operator_delete(param_1);
            return;
        }
        *(int*)(iVar1 + 0x1b4) = 0;
        *(int*)(param_2 + 4) = iVar1;
        operator_delete(param_1);
        return;
    }
    if (iVar2 != 0) {
        *(int*)(iVar2 + 0x1b8) = 0;
        *(int*)(param_2 + 8) = iVar2;
        operator_delete(param_1);
        return;
    }
    *(int*)(param_2 + 4) = 0;
    *(int*)(param_2 + 8) = 0;
    operator_delete(param_1);
}

// FUN_00404ad0 @ 0x00404AD0 — Sound_FreeChannel(Buffer)
// Releases DirectSound buffers for the given slot (all loaded channels),
// resets slot count and 3D-anchor table.
//
// BUG-FIX 2026-04-28: el IDA original usaba `g_lpDSBuffer[0][v3]` con
// v3 = 4*Buffer + channel — un acceso flatten que el compilador C++ trata
// como "fila 0, índice fuera de rango". MSVC en Release lo computa offset-
// based (funciona) pero con ITERACIONES ilimitadas (MaxBufferChannel sin
// clamp) leía mucho más allá del array, devolviendo basura tipo 0xC2A00000
// (-80.0f bit-pattern) → v4->Release() → AV.
//
// Cambios:
//   1. Indexar con 2D plano: g_lpDSBuffer[Buffer][channel].
//   2. Clamp MaxBufferChannel a [0, 4] — array tiene exactamente 4 canales.
//   3. Bounds-check Buffer < 420.
//   4. Enable3DSound check usa el slot Buffer, no v2 (era bug del IDA).
HRESULT __cdecl FUN_00404ad0(int Buffer)
{
  if (!g_EnableSound) return 0;
  if (Buffer < 0 || Buffer >= 420) return 0;

  int channels = MaxBufferChannel[Buffer];
  if (channels < 0) channels = 0;
  if (channels > 4) channels = 4;

  for (int ch = 0; ch < channels; ++ch) {
    LPDIRECTSOUNDBUFFER v4 = g_lpDSBuffer[Buffer][ch];
    if (v4) {
      v4->Release();
      g_lpDSBuffer[Buffer][ch] = nullptr;
    }
    if (Enable3DSound[Buffer]) {
      LPDIRECTSOUND3DBUFFER v5 = g_lpDS3DBuffer[Buffer][ch];
      if (v5) {
        v5->Release();
        g_lpDS3DBuffer[Buffer][ch] = nullptr;
      }
    }
  }
  MaxBufferChannel[Buffer] = 0;
  Object3DSound[Buffer][0] = 0;
  Object3DSound[Buffer][1] = 0;
  Object3DSound[Buffer][2] = 0;
  Object3DSound[Buffer][3] = 0;
  if (SoundLoadCount > 0) --SoundLoadCount;
  return 0;
}
