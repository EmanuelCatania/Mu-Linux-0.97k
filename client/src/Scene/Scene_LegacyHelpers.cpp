// Scene_LegacyHelpers.cpp

#include "stdafx.h"
#include "Scene/ObjectRegistry.h"
#include "Platform/IAudio.h"
#include "globals.h"
#include "functions.h"

// WidgetB_CtorFull @ 0x0040A660 — WidgetB_CtorFull: construct two linked WidgetB objects.
// Calls WidgetB_Ctor on self, sets vtable, then constructs a local WidgetB and copies it.
void* __fastcall WidgetB_CtorFull(void *param_1)
{
    WidgetB_Ctor(param_1);
    // vtable = &PTR_FUN_00552588 — skipped

    // local_2c is a stack-local WidgetB (0x20 bytes, 8 dwords)
    int local_2c[8] = {};
    WidgetB_Ctor(local_2c);
    WidgetB_SetVtable(local_2c);
    return param_1;
}

// VTable_Release @ 0x0040A860 — VTable_Release
// Iterates object->edge_list and calls FUN_0040a8f0 on each edge.
void __cdecl VTable_Release(void *obj, float* vertices)
{
    char *this_ = (char*)obj;
    int count = *(int*)(this_ + 0x18);
    if (count <= 0) return;
    for (int i = 0; i < count; i++) {
        short *psVar1 = (short*)(*(int*)(this_ + 0x1c) + i * 10);
        int iVar2 = (int)psVar1[2] * 15000;
        float *p1 = vertices + (*psVar1 + iVar2) * 3;
        float *p2 = vertices + (psVar1[1] + iVar2) * 3;
        FUN_0040a8f0(obj, p1, p2);
    }
}

// ── Scene entity grid helpers ─────────────────────────────────────────────────
// Entity_GridUnlink @ 0x004FFCC0 — Entity_GridUnlink(node, cell_ptr)
// Unlinks node from its doubly-linked list in the 16x16 scene grid.
void __cdecl Entity_GridUnlink(void *param_1_v, void* cell) {
    OBJECT* param_1 = AsObject(param_1_v);
    SceneObjectCell* param_2 = static_cast<SceneObjectCell*>(cell);
    if (!param_1) return;
    SceneObjects::Unregister(param_1);
    OBJECT* iVar1 = param_1->SceneNext;
    OBJECT* iVar2 = param_1->ScenePrevious;
    if (iVar1 != 0) {
        if (iVar2 != 0) {
            iVar2->SceneNext = iVar1;
            iVar1->ScenePrevious = param_1->ScenePrevious;
            operator_delete(param_1);
            return;
        }
        iVar1->ScenePrevious = nullptr;
        param_2->Head = iVar1;
        operator_delete(param_1);
        return;
    }
    if (iVar2 != 0) {
        iVar2->SceneNext = nullptr;
        param_2->Tail = iVar2;
        operator_delete(param_1);
        return;
    }
    param_2->Head = nullptr;
    param_2->Tail = nullptr;
    operator_delete(param_1);
}

// Sound_ReleaseBuffer @ 0x00404AD0 (IDA: FUN_00404AD0; 5.2: ReleaseBuffer).
// Releases DirectSound buffers for the given slot (all loaded channels),
// resets slot count and 3D-anchor table.
//
// Desviaciones respecto de IDA (que indexa `g_lpDSBuffer[0][v3]` con
// v3 = 4*Buffer + channel, sin clamp):
//   1. Indexar con 2D plano: g_lpDSBuffer[Buffer][channel].
//   2. Clamp MaxBufferChannel a [0, 4] — array tiene exactamente 4 canales.
//   3. Bounds-check Buffer < 420.
//   4. Enable3DSound check usa el slot Buffer, no v2 (era bug del IDA).
HRESULT __cdecl Sound_ReleaseBuffer(int Buffer)
{
    if (!g_EnableSound || Buffer < 0 || Buffer >= 420) return 0;
    const HRESULT result = Platform::GetAudio().ReleaseBuffer(Buffer);
    MaxBufferChannel[Buffer] = 0;
    for (int channel = 0; channel < 4; ++channel) Object3DSound[Buffer][channel] = 0;
    if (SoundLoadCount > 0) --SoundLoadCount;
    return result;
}
