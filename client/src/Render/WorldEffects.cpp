#include "stdafx.h"
#include "Game/RenderInterpolation.h"
#include "Game/RenderInterpolationMath.h"
#include "Render/MaterialVisuals.h"
#include "Render/Entity_Render_3D.h"
#include "Render/WorldEffects.h"
#include "Render/EquipmentVisuals.h"
#include "Entity/ObjectPose.h"
#include "Scene/ObjectRegistry.h"
#include "Game/MapManager.h"
#include <unordered_map>
#include <vector>

namespace {
struct Sprite {
    unsigned short type;
    float position[3], scale, light[3], rotation;
    int owner, mode;
};
struct Frame {
    std::uint64_t generation;
    int type;
    std::vector<Sprite> sprites;
};
std::unordered_map<const void *, Frame> frames, previousFrames;
Frame *recording = nullptr;
int scene = -1, world = -1;
}

void WorldEffects_BeginStep() {
    recording = nullptr;
    previousFrames.swap(frames);
    if (scene!=SceneFlag || world!=gMapManager.GetCurrentMap()) previousFrames.clear();
    frames.clear();
    scene = SceneFlag;
    world = gMapManager.GetCurrentMap();
}

int WorldEffects_AddSprite(unsigned short type, float *position, float scale,
    float *light, int owner, float rotation, int mode) {
    if (!recording) return 0;
    Sprite sprite;
    sprite.type = type; sprite.scale = scale; sprite.owner = owner;
    sprite.rotation = rotation; sprite.mode = mode;
    memcpy(sprite.position, position, sizeof(sprite.position));
    memcpy(sprite.light, light, sizeof(sprite.light));
    recording->sprites.push_back(sprite);
    // El retorno legacy sólo se propagaba al caller del walker, que lo ignora.
    return 0;
}

void WorldEffects_Tick(int entity) {
    void *object = (void *)entity;
    if (!SceneObjects::Contains(object) || !AsObject(entity)->Live) return;
    if (frames.find(object) != frames.end()) return;
    Frame &frame = frames[object];
    frame.generation = SceneObjects::Generation(object);
    frame.type = AsObject(entity)->Type;
    void *model = (void *)(DAT_05828d58 + frame.type * 0xbc);
    EquipmentModelContext savedModel(model);
    ObjectPose_PrepareBounds(entity, 0);
    *(float *)((char *)model + 0x84) = 0.0f;
    *(BYTE *)((char *)model + 0x45) = AsObject(entity)->ContrastEnable;
    Entity_GetLightScale(entity, (int)model);
    // DESVIACION: los efectos necesitan una pose SIM propia, incluso si alpha
    // oculta la malla; nunca heredan huesos del último objeto dibujado.
    memset(g_BoneScratch, 0, sizeof(g_BoneScratch));
    if (*(short *)((char *)model + 0x26) > 0)
        ObjectPose_PrepareBones(entity, model, (float *)g_BoneScratch, 0);
    ObjectPose_TickMeshEvents(entity, model, (float *)g_BoneScratch, 0);
    MoltSilhouette_Tick(entity, model, (float *)g_BoneScratch, 0);
    MaterialVisuals_TickObjectDraw(entity);
    recording = &frame;
    Entity_SpawnEffects(entity);
    recording = nullptr;
}

void WorldEffects_Draw(int entity) {
    const void *object = (void *)entity;
    // Validar registro/generación antes de leer la dirección: puede haberse
    // liberado o reutilizado entre la publicación SIM y esta pasada.
    if (scene != SceneFlag || world != gMapManager.GetCurrentMap() ||
        !SceneObjects::Contains(object)) return;
    const auto found = frames.find(object);
    if (found == frames.end() || found->second.generation != SceneObjects::Generation(object) ||
        !AsObject(entity)->Live || found->second.type != AsObject(entity)->Type) return;
    const Frame &frame=found->second;
    // Sin comandos no hay posiciones que mezclar ni sprites que reproducir.
    if (frame.sprites.empty()) return;
    const auto old=previousFrames.find(object);
    const bool continuous=RenderInterpolation_OwnerContinuous(object) && old!=previousFrames.end() &&
        old->second.generation==frame.generation && old->second.type==frame.type && old->second.sprites.size()==frame.sprites.size();
    for (size_t i=0;i<frame.sprites.size();++i) {
        const Sprite &sprite=frame.sprites[i];
        float position[3]; memcpy(position,sprite.position,sizeof(position));
        if (continuous) {
            const Sprite &previous=old->second.sprites[i];
            if (previous.type==sprite.type && previous.owner==sprite.owner && previous.mode==sprite.mode)
                for (int axis=0;axis<3;++axis) position[axis]=RenderBlend(previous.position[axis],sprite.position[axis],RenderInterpolation_Alpha());
        }
        CreateSprite(sprite.type,position,sprite.scale,(float *)sprite.light,sprite.owner,sprite.rotation,sprite.mode);
    }
}
