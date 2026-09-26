// Scene_ObjectInteraction.cpp
//
// Extracted from stubs_game.cpp.  Owns special scene-object updates and
// mouse picking.  Entry-point comments retain their IDA symbols/addresses.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

extern void ClearActionObject(void);
// MoveObject_Special @ 0x004FA5F0 (~93 lines) — castle gate destruction animation
// Gate objects in castle siege world. Decrements counter, plays sound 0x6c,
// spawns 10 dust particles at height 80, clears terrain on completion.
// MoveObject_Special (0x004FA5F0) vive en Render/Render_LegacyLinker.cpp.
//
// 2026-09-26: aca habia un puente con ese nombre cuyo cuerpo era
//     MoveObject_Special(param_1); return;   + 80 lineas despues del return
// o sea delegaba y dejaba la version vieja como codigo inalcanzable.  Nadie
// lo llamaba: los cuatro call sites van al FUN_ directo.  Eliminado.

// PickObject_Mouse @ 0x004FA7C0 (~90 lines) — mouse-picking scene objects
// Iterates 0x10 * 0x10 object lists. Per visible object:
// BMD::Animation + Transform, then CollisionDetectLineToMesh vs MousePosition/Target.
char* __stdcall PickObject_Mouse(void) {
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

    Map_InitRayCast();  // Map_InitRayCast — sets up MousePosition/MouseTarget ray

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
                    // BMD_Animation(model, BoneTransform, frame, priorFrame, priorAction, angles, headAngles, parent, translate)
                    // Phantom regs make exact mapping impractical; the key effect is populating BoneTransform.
                    float angles[3] = { 0.0f, 0.0f, *(float*)(obj + 0x28) };
                    float headAngles[3] = { 0.0f, 0.0f, 0.0f };
                    BMD_Animation(model, 0, *(float*)(obj + 0x10C),
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
                    bool hit = Collision_SegmentToOBB((float*)&CameraRayOriginX, (float*)&DAT_083a4110, obb);
                    if ((char)hit != '\0') {
                        // CollisionDetectLineToMesh — precise line-mesh test
                        // Phantom regs in Ghidra; approximate call:
                        if (BMD__CollisionDetectLineToMesh(
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
