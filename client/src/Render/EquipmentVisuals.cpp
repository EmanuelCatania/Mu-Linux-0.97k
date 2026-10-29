#include "stdafx.h"
#include "Render/MaterialVisuals.h"
#include "Game/RenderInterpolation.h"
#include "Game/RenderInterpolationMath.h"
#include "Game/RenderSnapshots.h"
#include "Render/EquipmentVisuals.h"
#include "Game/MapManager.h"
#include <vector>

namespace {
struct Sprite {
    unsigned short type;
    float position[3], scale, light[3], rotation;
    int owner, mode, stage;
};
struct Mesh {
    MaterialVisualSample material;
    int model, part=0, stage=0;
    PART_t partState{};
    float root[12]{}, relative[12]{}, offset[3]{}, extraOrigin[3]{};
    bool linked=false, relativeValid=false;
    char context[0xbc], object[0x1c0], bones[sizeof(g_BoneScratch)];
    float light[3], alpha, lod;
    unsigned int level, flags;
    unsigned char option;
    char translate;
};
struct Command { bool mesh; unsigned int index; int stage; };
struct EquipmentFrame {
    std::vector<Mesh> meshes;
    std::vector<Sprite> sprites;
    std::vector<Command> commands;
    const CHARACTER* entity = nullptr;
    int spawn = 0, type = 0, key = 0;
    bool valid = false;
};
EquipmentFrame frames[401], previousFrames[401];
EquipmentFrame *recording = nullptr;
int currentStage = 0, scene = -1, world = -1;
bool drawing = false;

int Slot(const void *entity) {
    if (entity == DAT_07abf050) return 400;
    const uintptr_t address = (uintptr_t)entity, base = (uintptr_t)DAT_07abf5d0;
    if (!base || address < base || address - base >= 400 * sizeof(CHARACTER)) return -1;
    const uintptr_t offset = address - base;
    return offset % sizeof(CHARACTER) ? -1 : (int)(offset / sizeof(CHARACTER));
}
}

EquipmentModelContext::EquipmentModelContext(void *value) : model(value) {
    memcpy(saved, model, sizeof(saved));
}
EquipmentModelContext::~EquipmentModelContext() { memcpy(model, saved, sizeof(saved)); }

void Equipment_BeginStep() {
    recording = nullptr;
    const bool sameScene=scene==SceneFlag && world==gMapManager.GetCurrentMap();
    scene = SceneFlag; world = gMapManager.GetCurrentMap();
    for (int slot=0;slot<401;++slot) {
        std::swap(frames[slot],previousFrames[slot]);
        if (!sameScene) previousFrames[slot].valid=false;
        EquipmentFrame &frame=frames[slot];
        frame.valid = false;
        frame.meshes.clear(); frame.sprites.clear(); frame.commands.clear();
    }
}
void Equipment_Invalidate(const void *entity) {
    const int slot = Slot(entity);
    if (slot >= 0) frames[slot].valid = previousFrames[slot].valid = false;
}
void Equipment_BeginCharacter(const CHARACTER* entity) {
    const int slot = Slot(entity);
    recording = slot >= 0 ? &frames[slot] : nullptr;
    if (!recording) return;
    recording->entity = entity;
    recording->spawn = AsObject(entity)->InitialSceneFrame;
    recording->type = AsObject(entity)->Type;
    recording->key = AsCharacter(entity)->Key;
    recording->valid = true;
}
void Equipment_EndCharacter() { recording = nullptr; }
void Equipment_SetStage(int stage) { currentStage = stage; }
bool Equipment_IsDrawing() { return drawing; }

void Equipment_AddSprite(unsigned short type, float *position, float scale,
    float *light, int owner, float rotation, int mode) {
    if (!recording) return;
    Sprite sprite;
    sprite.type = type; sprite.scale = scale; sprite.owner = owner;
    sprite.rotation = rotation; sprite.mode = mode; sprite.stage=currentStage;
    memcpy(sprite.position, position, sizeof(sprite.position));
    memcpy(sprite.light, light, sizeof(sprite.light));
    recording->commands.push_back({false, (unsigned int)recording->sprites.size(), currentStage});
    recording->sprites.push_back(sprite);
}
void Equipment_WingSprite(unsigned short type, float *position, float scale,
    float *light, int owner, float rotation, int mode) {
    if (recording) Equipment_AddSprite(type, position, scale, light, owner, rotation, mode);
    else CreateSprite(type, position, scale, light, owner, rotation, mode);
}
void Equipment_AddMesh(int model, const void *object, const float *light,
    float alpha, unsigned int level, unsigned char option, unsigned int flags, char translate, int part, int linked, const float *offset) {
    if (!recording) return;
    recording->commands.push_back({true, (unsigned int)recording->meshes.size(), currentStage});
    recording->meshes.emplace_back();
    Mesh &mesh = recording->meshes.back();
    MaterialVisuals_Prepare(mesh.material,model,false,alpha>_DAT_005524f8 && !*((const BYTE *)object+140));
    mesh.model = model; mesh.alpha = alpha; mesh.level = level;
    mesh.part=part; mesh.stage=currentStage;
    if (part) memcpy(&mesh.partState,(void *)part,sizeof(mesh.partState));
    mesh.linked=linked!=0;
    if (offset) memcpy(mesh.offset,offset,sizeof(mesh.offset));
    memcpy(mesh.root,&DAT_06989c9c,sizeof(mesh.root));
    const OBJECT &owner=*AsObject(recording->entity);
    const float *ownerBone=owner.BoneMatrices ? (const float *)owner.BoneMatrices+mesh.partState.LinkBone*12 : nullptr;
    float inverse[12], rotated[3]{};
    if (owner.BoneMatrices) {
        mesh.relativeValid=RenderInverseMatrix(ownerBone,inverse);
        if (mesh.relativeValid) R_ConcatTransforms(inverse,mesh.root,mesh.relative);
        if (!mesh.linked) BMD__RotationPosition((void *)(DAT_05828d58+owner.Type*0xbc),(float *)ownerBone,mesh.offset,rotated);
    }
    const BMD &context=*AsModel(DAT_05828d58+model*0xbc);
    for (int axis=0;axis<3;++axis) mesh.extraOrigin[axis]=context.BodyOrigin[axis]-owner.Position[axis]-rotated[axis];
    mesh.option = option; mesh.flags = flags; mesh.translate = translate;
    mesh.lod = _DAT_005597c8;
    memcpy(mesh.context, (void *)(DAT_05828d58 + model * 0xbc), sizeof(mesh.context));
    memcpy(mesh.bones, g_BoneScratch, sizeof(mesh.bones));
    memcpy(mesh.object, object, sizeof(mesh.object));
    memcpy(mesh.light, light, sizeof(mesh.light));
}

void Equipment_Draw(const CHARACTER* entity, int stage) {
    const int slot = Slot(entity);
    if (slot < 0 || scene != SceneFlag || world != gMapManager.GetCurrentMap()) return;
    EquipmentFrame &frame = frames[slot];
    if (!frame.valid || frame.entity != entity || !AsObject(entity)->Live ||
        frame.spawn != AsObject(entity)->InitialSceneFrame || frame.type != AsObject(entity)->Type ||
        frame.key != AsCharacter(entity)->Key) return;
    // DESVIACION: el dibujo reproduce la última muestra SIM. No avanza PART,
    // no consume azar ni crea joints/partículas persistentes por pasada.
    char savedBones[sizeof(g_BoneScratch)];
    memcpy(savedBones, g_BoneScratch, sizeof(savedBones));
    const float savedLod = _DAT_005597c8;
    char savedQuaternions[sizeof(DAT_05826e18)]; float savedRoot[12];
    memcpy(savedQuaternions,DAT_05826e18,sizeof(savedQuaternions)); memcpy(savedRoot,&DAT_06989c9c,sizeof(savedRoot));
    const EquipmentFrame &previous=previousFrames[slot];
    const bool continuous=RenderInterpolation_OwnerContinuous(entity) && previous.valid &&
        previous.entity==entity && previous.spawn==frame.spawn && previous.type==frame.type && previous.key==frame.key;
    const float alpha=RenderInterpolation_Alpha();
    drawing = true;
    for (const Command &command : frame.commands) {
        if (command.stage != stage) continue;
        if (!command.mesh) {
            Sprite &sprite = frame.sprites[command.index];
            float position[3]; memcpy(position,sprite.position,sizeof(position));
            // Las emisiones aleatorias pueden conservar cantidad y ordinal sin
            // corresponder al mismo punto del arma (por ejemplo, modelo 0x236).
            // Reproducir la muestra SIM y acompañar sólo a su dueño, como en
            // Sprite_ReplayStep; no unir dos destellos sin identidad persistente.
            float offset[3];
            if (RenderInterpolation_OwnerOffset((const void *)entity,offset))
                for (int axis=0;axis<3;++axis) position[axis]+=offset[axis];
            CreateSprite(sprite.type, position, sprite.scale, sprite.light,
                sprite.owner, sprite.rotation, sprite.mode);
            continue;
        }
        const Mesh &mesh = frame.meshes[command.index];
        float drawAlpha=mesh.alpha;
        void *model = (void *)(DAT_05828d58 + mesh.model * 0xbc);
        EquipmentModelContext savedModel(model);
        memcpy(model, mesh.context, sizeof(mesh.context));
        memcpy(g_BoneScratch, mesh.bones, sizeof(mesh.bones));
        if (RenderInterpolation_Alpha()<1 && continuous && AsObject(entity)->BoneMatrices) {
            const Mesh *old=command.index<previous.meshes.size() ? &previous.meshes[command.index] : nullptr;
            const bool same=old && old->part==mesh.part && old->model==mesh.model && old->stage==mesh.stage &&
                old->level==mesh.level && old->option==mesh.option && old->flags==mesh.flags && old->linked==mesh.linked &&
                old->partState.Type==mesh.partState.Type && old->partState.LinkBone==mesh.partState.LinkBone;
            const OBJECT &owner=*AsObject(entity);
            float *ownerBone=(float *)owner.BoneMatrices+mesh.partState.LinkBone*12;
            float root[12]; memcpy(root,mesh.root,sizeof(root));
            if (mesh.linked && mesh.relativeValid) {
                // Anclar a los huesos visuales del cuerpo; no repetir RenderLinkObject,
                // ItemObjectAttribute, PlayAnimation ni sus emisiones.
                R_ConcatTransforms(ownerBone,(float *)mesh.relative,root);
            } else if (same) RenderBlendMatrix(old->root,mesh.root,alpha,root);
            memcpy(&DAT_06989c9c,root,sizeof(root));
            BMD &context=*AsModel(model);
            context.BodyScale=owner.Scale;
            float rotated[3]{};
            if (!mesh.linked) {
                void *ownerModel=(void *)(DAT_05828d58+owner.Type*0xbc);
                EquipmentModelContext savedOwner(ownerModel);
                AsModel(ownerModel)->BodyScale=owner.Scale;
                BMD__RotationPosition(ownerModel,ownerBone,(float *)mesh.offset,rotated);
            }
            for (int axis=0;axis<3;++axis) context.BodyOrigin[axis]=owner.Position[axis]+rotated[axis]+mesh.extraOrigin[axis];
            float frame=mesh.partState.AnimationFrame,prior=mesh.partState.PriorAnimationFrame;
            if (same && old->partState.CurrentAction==mesh.partState.CurrentAction && old->partState.PriorAction==mesh.partState.PriorAction &&
                ((const BMD *)old->context)->CurrentAction==context.CurrentAction &&
                frame>=old->partState.AnimationFrame && prior>=old->partState.PriorAnimationFrame) {
                RenderPosePair pair{}; pair.continuous=true;
                pair.previous.animationFrame=old->partState.AnimationFrame;
                pair.previous.priorAnimationFrame=old->partState.PriorAnimationFrame;
                pair.current.animationFrame=frame; pair.current.priorAnimationFrame=prior;
                const RenderPose pose=RenderSnapshots_Interpolate(pair,alpha);
                frame=pose.animationFrame; prior=pose.priorAnimationFrame;
            }
            unsigned int priorBits; memcpy(&priorBits,&prior,sizeof(priorBits));
            float zero[3]{};
            BMD_Animation(model,reinterpret_cast<float*>(g_BoneScratch),frame,priorBits,mesh.partState.PriorAction,(unsigned int *)zero,zero,1,1);
            drawAlpha=owner.Alpha;
        }
        if (drawAlpha<=_DAT_005524f8) continue;
        _DAT_005597c8 = mesh.lod;
        float zero[3] = {}, bounds[15];
        Skeleton_Transform(model, reinterpret_cast<float*>(g_BoneScratch), zero, zero, bounds, mesh.translate);
        // El objeto mínimo es local: los materiales pueden escribir UV/escala.
        alignas(float) char object[sizeof(mesh.object)];
        memcpy(object, mesh.object, sizeof(object));
        float light[3]; memcpy(light, mesh.light, sizeof(light));
        MaterialVisualScope material(&mesh.material);
        RenderPartObjectEffect((int)object, mesh.model, light, drawAlpha,
            mesh.level, mesh.option, 0, mesh.flags);
    }
    drawing = false;
    memcpy(g_BoneScratch, savedBones, sizeof(savedBones));
    _DAT_005597c8 = savedLod;
    memcpy(DAT_05826e18,savedQuaternions,sizeof(savedQuaternions)); memcpy(&DAT_06989c9c,savedRoot,sizeof(savedRoot));
}
