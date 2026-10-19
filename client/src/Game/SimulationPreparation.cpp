#include "stdafx.h"
#include "Game/SimulationPreparation.h"
#include "Entity/ObjectPose.h"
#include "Entity/ObjectBounds.h"
#include "Item/ContentCatalog.h"

void DivineSkirt_Tick(int entity, int modelType, int part, void *model);

namespace {
bool SceneReady() {
    if (SceneFlag == 2) return DAT_083a7c49 != 0;
    if (SceneFlag == 4) return CharSelectSceneInitialized != 0 && DAT_05826cb0 > 50;
    return SceneFlag == 5 && DAT_083a7c10 != 0 && DAT_07e11d1c <= 30;
}
}

void Simulation_PrepareStep()
{
    if (!SceneReady() || !DAT_07abf5d0 || !DAT_05828d58) return;
    // BMD_Animation usa estos temporales compartidos; la publicación SIM sólo
    // conserva sus matrices de salida, sin filtrar contexto a otros callers.
    char quaternions[sizeof(DAT_05826e18)];
    float root[12];
    memcpy(quaternions, DAT_05826e18, sizeof(quaternions));
    memcpy(root, &DAT_06989c9c, sizeof(root));
    for (int slot = 0; slot < 400; ++slot) {
        CHARACTER &c = ((CHARACTER *)DAT_07abf5d0)[slot];
        OBJECT &o = c.Object;
        if (!o.Live || (!o.Visible && !((void *)&c == (void *)DAT_07abf5d8 && SceneFlag == 5))) continue;
        void *model = (void *)(DAT_05828d58 + o.Type * 0xbc);
        if (*(short *)((char *)model + 0x26) == 0) continue;
        char context[0xbc];
        memcpy(context, model, sizeof(context));
        if (o.Alpha >= _DAT_005524f8) {
            *(float *)((char *)model + 0x84) = 0.0f;
            float *bones = o.EnableBoneMatrix ? (float *)o.BoneMatrices : (float *)&DAT_06970a9c;
            ObjectPose_PrepareBones((int)&c, model, bones, 1);
            ObjectBounds_Build(o.BoundingBoxMin, o.BoundingBoxMax,
                               (float *)((char *)model + 0x6c), o.OBB);
        }
        memcpy(model, context, sizeof(context));
        const bool subtypeNpc = o.Type == 390 && o.SubType >= 206 && o.SubType <= 208;
        if (!subtypeNpc && !c.Change) {
            for (int part = 0; part < 6; ++part) {
                PART_t &piece = c.BodyPart[part];
                if (piece.Type == -1) continue;
                const int type = gContentCatalog.EntityDrawModel(&c, piece.Type);
                if (type != 707 || o.Alpha <= _DAT_005524f8) continue;
                void *pants = (void *)(DAT_05828d58 + type * 0xbc);
                memcpy(context, pants, sizeof(context));
                *(float *)((char *)pants + 0x68) = o.Scale;
                memcpy((char *)pants + 0x6c, o.Position, sizeof(o.Position));
                DivineSkirt_Tick((int)&c, type, (int)&piece, pants);
                memcpy(pants, context, sizeof(context));
            }
        }
    }
    memcpy(DAT_05826e18, quaternions, sizeof(quaternions));
    memcpy(&DAT_06989c9c, root, sizeof(root));
}
