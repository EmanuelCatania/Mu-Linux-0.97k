#include "stdafx.h"
#include "Render/EquipmentVisuals.h"
#include "Game/MapManager.h"
#include <vector>

namespace {
struct Sprite {
    unsigned short type;
    float position[3], scale, light[3], rotation;
    int owner, mode;
};
struct Mesh {
    int model;
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
    int entity = 0, spawn = 0, type = 0, key = 0;
    bool valid = false;
};
EquipmentFrame frames[401];
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
    scene = SceneFlag; world = gMapManager.GetCurrentMap();
    for (EquipmentFrame &frame : frames) {
        frame.valid = false;
        frame.meshes.clear(); frame.sprites.clear(); frame.commands.clear();
    }
}
void Equipment_Invalidate(const void *entity) {
    const int slot = Slot(entity);
    if (slot >= 0) frames[slot].valid = false;
}
void Equipment_BeginCharacter(int entity) {
    const int slot = Slot((void *)entity);
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
    sprite.rotation = rotation; sprite.mode = mode;
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
    float alpha, unsigned int level, unsigned char option, unsigned int flags, char translate) {
    if (!recording) return;
    recording->commands.push_back({true, (unsigned int)recording->meshes.size(), currentStage});
    recording->meshes.emplace_back();
    Mesh &mesh = recording->meshes.back();
    mesh.model = model; mesh.alpha = alpha; mesh.level = level;
    mesh.option = option; mesh.flags = flags; mesh.translate = translate;
    mesh.lod = _DAT_005597c8;
    memcpy(mesh.context, (void *)(DAT_05828d58 + model * 0xbc), sizeof(mesh.context));
    memcpy(mesh.bones, g_BoneScratch, sizeof(mesh.bones));
    memcpy(mesh.object, object, sizeof(mesh.object));
    memcpy(mesh.light, light, sizeof(mesh.light));
}

void Equipment_Draw(int entity, int stage) {
    const int slot = Slot((void *)entity);
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
    drawing = true;
    for (const Command &command : frame.commands) {
        if (command.stage != stage) continue;
        if (!command.mesh) {
            Sprite &sprite = frame.sprites[command.index];
            CreateSprite(sprite.type, sprite.position, sprite.scale, sprite.light,
                sprite.owner, sprite.rotation, sprite.mode);
            continue;
        }
        const Mesh &mesh = frame.meshes[command.index];
        void *model = (void *)(DAT_05828d58 + mesh.model * 0xbc);
        EquipmentModelContext savedModel(model);
        memcpy(model, mesh.context, sizeof(mesh.context));
        memcpy(g_BoneScratch, mesh.bones, sizeof(mesh.bones));
        _DAT_005597c8 = mesh.lod;
        float zero[3] = {}, bounds[15];
        Skeleton_Transform(model, (int)g_BoneScratch, zero, zero, bounds, mesh.translate);
        // El objeto mínimo es local: los materiales pueden escribir UV/escala.
        alignas(float) char object[sizeof(mesh.object)];
        memcpy(object, mesh.object, sizeof(object));
        float light[3]; memcpy(light, mesh.light, sizeof(light));
        RenderPartObjectEffect((int)object, mesh.model, light, mesh.alpha,
            mesh.level, mesh.option, 0, mesh.flags);
    }
    drawing = false;
    memcpy(g_BoneScratch, savedBones, sizeof(savedBones));
    _DAT_005597c8 = savedLod;
}
