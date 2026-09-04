// Scene_ObjectInteraction.cpp
//
// Extracted from stubs_game.cpp.  Owns special scene-object updates and
// mouse picking.  Entry-point comments retain their IDA symbols/addresses.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

extern void FUN_004fa5a0(void);
// MoveObject_Special @ 0x004FA5F0 (~93 lines) — castle gate destruction animation
// Gate objects in castle siege world. Decrements counter, plays sound 0x6c,
// spawns 10 dust particles at height 80, clears terrain on completion.
void __cdecl MoveObject_Special_stub(int param_1) {
    // Retain the old exported name, but execute the exact IDA port.
    FUN_004fa5f0(param_1);
    return;
    // 0x004FA5F0 — Castle gate destruction animation.
    // param_1 = pointer to scene object struct.
    // DAT_0055a7b4 = target world ID, DAT_0055a7b0 = target object type,
    // DAT_0055a7b8 = destruction frame counter, _DAT_0055a7bc = velocity accumulator.
    // World = g_GameSubState (current map ID).

    if ((int)DAT_0055a7b4 < 0) return;
    if ((int)DAT_0055a7b0 < 0) return;
    if ((int)DAT_0055a7b8 < 0) return;
    if ((DWORD)World != DAT_0055a7b4) return;

    short objType = *(short*)(param_1 + 2);

    if (objType != (short)DAT_0055a7b0) {
        if (objType == 9) goto label_gate_side;
        if (objType != 10) return;
    }

    if ((objType != 9) && (objType != 10)) {
        // Main gate object
        if (objType != (short)DAT_0055a7b0) return;

        if (DAT_0055a7b8 == 0x14) {
            // Start destruction: set initial height offset and mark for removal
            *(DWORD*)(param_1 + 0x1c) = 0x420c0000;  // 35.0f
            *(DWORD*)(param_1 + 0x58) = 0xFFFFFFFF;   // -1 (destroy marker)
            PlayBuffer(0x6c, 0, 0);  // gate destruction sound
        }

        if ((int)DAT_0055a7b8 < 0) return;

        float* pHeight = (float*)(param_1 + 0x1c);
        *pHeight = _DAT_0055a7bc + *pHeight;
        _DAT_0055a7bc = _DAT_0055a7bc + _DAT_005528f0;  // accelerate

        if (_DAT_00552848 <= *pHeight) {
            *pHeight = *pHeight - (float)(int)DAT_0055a7b8;
            _DAT_0055a7bc = 2.0f;

            if (*pHeight == 80.0f) {
                // Spawn 10 dust particles at ground level
                for (int i = 10; i != 0; i--) {
                    float posX = (float)(rand() % 300 - (int)_DAT_0055297c) + *(float*)(param_1 + 0x10);
                    float posZ = *(float*)(param_1 + 0x14) - (float)(rand() % 0x14 + (int)_DAT_00552ab4);
                    // CreateParticle — Ghidra shows phantom regs (unaff_EBX/EBP/ESI/EDI);
                    // actual call signature: Particle_Spawn(type, pos, light, size, flag, alpha, mode)
                    // The particle spawn at height 80 with dust effect 0x4c5 is the key logic.
                    // Due to phantom stack params, exact arg mapping is approximate.
                    float pos[3] = { posX, *pHeight, *(float*)(param_1 + 0xe8) };
                    float light[3] = { 1.0f, 1.0f, 1.0f };
                    Particle_Spawn(0x4C5, pos, light, NULL, 0, posZ, 0);
                }
            }
        }

        if (DAT_0055a7b8 == 0) {
            *(DWORD*)(param_1 + 0x58) = 0xFFFFFFFE;  // -2 (fully destroyed)
            *pHeight = 90.0f;
            FUN_004fa5a0();  // GuildMark_ResetTarget
            // AddTerrainAttributeRange(0xd, 0x46, 3, 6, 8, 0) — clear walkable zone
            Terrain_UpdateTileAttributeRect(0x0d, 0x46, 3, 6, 0x08, 0x00);
        }

        DAT_0055a7b8 = DAT_0055a7b8 - 1;
        return;
    }

label_gate_side:
    // Side gate objects (types 9 and 10): hide when counter reaches 0
    if (DAT_0055a7b8 != 0) return;
    *(DWORD*)(param_1 + 0x58) = 0xFFFFFFFF;  // -1
    *(unsigned short*)(param_1 + 0x86) = 4;
}

// PickObject_Mouse @ 0x004FA7C0 (~90 lines) — mouse-picking scene objects
// Iterates 0x10 * 0x10 object lists. Per visible object:
// BMD::Animation + Transform, then CollisionDetectLineToMesh vs MousePosition/Target.
char* __stdcall PickObject_Mouse_stub(void) {
    // 0x004FA7C0 — Mouse-picking scene objects.
    // Iterates 0x10 * 0x10 object linked lists rooted at DAT_083a021c (stride 4 DWORDs per cell).
    // Per visible object: sets up BMD model data, calls BMD::Animation + Transform,
    // copies OBB data (48 bytes), then CollisionDetectLineToMesh vs MousePosition/MouseTarget.
    // Returns pointer to picked object, or NULL.
    //
    // NOTE: Ghidra shows phantom registers (unaff_EBX/EBP/ESI/EDI) in BMD::Animation
    // and BMD::CollisionDetectLineToMesh calls — these are anti-tamper artifacts.
    // The actual logic is faithful but some inner call args are approximate.

    char* result = NULL;

    FUN_00512d30();  // Map_InitRayCast — sets up MousePosition/MouseTarget ray

    DWORD* pCell = (DWORD*)&DAT_083a021c;
    do {
        int cellCount = 0x10;
        do {
            char* obj = (char*)(DWORD)*pCell;
            // 2026-05-07: SEH-wrap to survive corrupt next pointers in the
            // bucket linked list (same root cause as the MoveObjects guard).
            __try {
            int pickIter = 0;
            while (obj != NULL && pickIter++ < 4096 &&
                   (uintptr_t)obj >= 0x10000u && (uintptr_t)obj < 0x80000000u) {
                // Check: object alive, visible, and alpha above threshold
                if ((obj[0] != '\0') && (obj[0x160] != '\0') &&
                    (_DAT_005524f8 <= *(float*)(obj + 0x168)))
                {
                    // Get model from entity type
                    short modelType = *(short*)(obj + 2);
                    char* modelBase = (char*)DAT_05828d58;
                    // BMD stride is 0xBC; Models = DAT_05828d58
                    char* model = modelBase + modelType * 0xBC;

                    // Set model data fields from object
                    *(DWORD*)(model + 0x68) = *(DWORD*)(obj + 0x0C);
                    *(char*)(model + 0xA0) = obj[0x105];
                    *(DWORD*)(model + 0x6C) = *(DWORD*)(obj + 0x10);
                    *(DWORD*)(model + 0x70) = *(DWORD*)(obj + 0x14);
                    *(DWORD*)(model + 0x74) = *(DWORD*)(obj + 0x18);

                    // BMD::Animation — set up bone matrices
                    // FUN_00440060(model, BoneTransform, frame, priorFrame, priorAction, angles, headAngles, parent, translate)
                    // Phantom regs make exact mapping impractical; the key effect is populating BoneTransform.
                    float angles[3] = { 0.0f, 0.0f, *(float*)(obj + 0x28) };
                    float headAngles[3] = { 0.0f, 0.0f, 0.0f };
                    FUN_00440060(model, 0, *(float*)(obj + 0x10C),
                                 (unsigned int)(unsigned char)obj[0x106],
                                 (unsigned char)obj[0x1C], (unsigned int*)angles, headAngles,
                                 0, 0);

                    // BMD::Transform — compute world-space OBB
                    float bbMin[3], bbMax[3];
                    // obj+0x118 = BoundingBoxMin, obj+0x124 = mid, obj+0x130 = BoundingBoxMax
                    BMD_TransformPosition(model, (float*)(obj + 0x118), (float*)(obj + 0x130), bbMin, 0);

                    // Copy 48 bytes of OBB data from obj+0x130
                    float obb[12];
                    memcpy(obb, obj + 0x130, 48);

                    // Check ray intersection with bounding volume
                    bool hit = FUN_00513260((float*)&CameraRayOriginX, (float*)&DAT_083a4110, obb);
                    if ((char)hit != '\0') {
                        // CollisionDetectLineToMesh — precise line-mesh test
                        // Phantom regs in Ghidra; approximate call:
                        if (BMD__CollisionDetectLineToMesh_stub(
                                (DWORD)model, (float*)&CameraRayOriginX, (float*)&DAT_083a4110,
                                false, 0, 0)) {
                            result = obj;
                        }
                    }
                }
                obj = *(char**)(obj + 0x1B8);  // next in linked list
            }
            } __except (EXCEPTION_EXECUTE_HANDLER) {
                // Corrupt linked-list node — abandon this cell.
            }
            pCell += 4;  // 4 DWORDs per cell
            cellCount--;
        } while (cellCount != 0);
    } while (pCell < &DAT_083a021c + 0x400);  // 0x10*0x10 grid, 4 bytes each = 0x1000 bytes

    return result;
}
