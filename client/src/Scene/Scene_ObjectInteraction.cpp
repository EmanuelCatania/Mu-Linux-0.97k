// Scene_ObjectInteraction.cpp
//
// Updates de objetos especiales de la escena y mouse picking.

#include "stdafx.h"
#include "Render/ResourceTables.h"
#include "Scene/ObjectRegistry.h"
#include "Platform/GuardedOperation.h"
#include "Item/ItemDefines.h"
#include "globals.h"
#include "functions.h"

extern void ClearActionObject(void);
// MoveObject_Special @ 0x004FA5F0 (~93 lines) — castle gate destruction animation
// Gate objects in castle siege world. Decrements counter, plays sound 0x6c,
// spawns 10 dust particles at height 80, clears terrain on completion.
// MoveObject_Special (0x004FA5F0) vive en Render/Render_LegacyLinker.cpp.

// PickObject_Mouse @ 0x004FA7C0 (~90 lines) — mouse-picking scene objects
// Iterates 0x10 * 0x10 object lists. Per visible object:
// BMD::Animation + Transform, then CollisionDetectLineToMesh vs MousePosition/Target.
char* __stdcall PickObject_Mouse(void) {
    // 0x004FA7C0 — Mouse-picking scene objects.
    // Recorre las listas de las 256 celdas en el orden original.
    // Per visible object: sets up BMD model data, calls BMD::Animation + Transform,
    // copies OBB data (48 bytes), then CollisionDetectLineToMesh vs MousePosition/MouseTarget.
    // Returns pointer to picked object, or NULL.
    //
    // NOTE: Ghidra shows phantom registers (unaff_EBX/EBP/ESI/EDI) in BMD::Animation
    // and BMD::CollisionDetectLineToMesh calls — these are anti-tamper artifacts.
    // The actual logic is faithful but some inner call args are approximate.

    char* result = NULL;

    Map_InitRayCast();  // Map_InitRayCast — sets up MousePosition/MouseTarget ray

    SceneObjectCell* pCell = g_ObjectBucketGrid;
    do {
        int cellCount = 0x10;
        do {
            char* obj = static_cast<char*>(pCell->Head);
            // SEH alrededor del recorrido de la lista de buckets (punteros next corruptos),
            // igual que el guard de MoveObjects.
            Platform::RunValidatedOperation([&]() {
            int pickIter = 0;
            while (obj != NULL && pickIter++ < 4096 &&
                   SceneObjects::Contains(obj)) {
                // Check: object alive, visible, and alpha above threshold
                if ((*(char *)&AsObject(obj)->Live != '\0') && (*(char *)&AsObject(obj)->Visible != '\0') &&
                    (_DAT_005524f8 <= AsObject(obj)->Alpha))
                {
                    // Get model from entity type
                    short modelType = AsObject(obj)->Type;
                    if (modelType < 0 || modelType >= ResourceTables::ModelCount() || !DAT_05828d58) {
                        obj = reinterpret_cast<char*>(AsObject(obj)->SceneNext);
                        continue;
                    }
                    BMD* model = AsModel(DAT_05828d58) + modelType;

                    // Set model data fields from object
                    *(DWORD *)&AsModel(model)->BodyScale = *(DWORD *)&AsObject(obj)->Scale;
                    *(char *)&AsModel(model)->CurrentAction = *(char *)&AsObject(obj)->CurrentAction;
                    *(DWORD *)&AsModel(model)->BodyOrigin[0] = *(DWORD *)&AsObject(obj)->Position[0];
                    *(DWORD *)&AsModel(model)->BodyOrigin[1] = *(DWORD *)&AsObject(obj)->Position[1];
                    *(DWORD *)&AsModel(model)->BodyOrigin[2] = *(DWORD *)&AsObject(obj)->Position[2];

                    // BMD::Animation — set up bone matrices
                    // BMD_Animation(model, BoneTransform, frame, priorFrame, priorAction, angles, headAngles, parent, translate)
                    // Phantom regs make exact mapping impractical; the key effect is populating BoneTransform.
                    float angles[3] = { 0.0f, 0.0f, AsObject(obj)->HeadAngle[0] };
                    float headAngles[3] = { 0.0f, 0.0f, 0.0f };
                    BMD_Animation(model, 0, AsObject(obj)->PriorAnimationFrame,
                                 (unsigned int)(unsigned char)*(char *)&AsObject(obj)->PriorAction,
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
                        // IDA 0x004FA7C0: actualiza el hit más cercano sin excluir caras.
                        if (BMD__CollisionDetectLineToMesh(
                                model, (float*)&CameraRayOriginX, (float*)&DAT_083a4110,
                                true, -1, -1)) {
                            result = obj;
                        }
                    }
                }
                obj = reinterpret_cast<char*>(AsObject(obj)->SceneNext);
            }
            }); // Windows conserva SEH; POSIX valida cada nodo antes de leerlo.
            ++pCell;
            cellCount--;
        } while (cellCount != 0);
    } while (pCell < g_ObjectBucketGrid + 256);

    return result;
}
