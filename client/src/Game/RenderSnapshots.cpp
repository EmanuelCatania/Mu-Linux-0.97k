#include "stdafx.h"
#include "Render/EquipmentVisuals.h"
#include "Game/RenderSnapshots.h"
#include "Game/MapManager.h"
#include <cmath>
#include <cstdint>

namespace {
float Blend(float a, float b, float alpha) {
    if (alpha <= 0.0f) return a;
    if (alpha >= 1.0f) return b;
    return a + (b - a) * alpha;
}
float BlendAngle(float a, float b, float alpha) {
    if (alpha <= 0.0f) return a;
    if (alpha >= 1.0f) return b;
    float delta = std::fmod(b - a, 360.0f);
    if (delta > 180.0f) delta -= 360.0f;
    if (delta < -180.0f) delta += 360.0f;
    float result = std::fmod(a + delta * alpha, 360.0f);
    if (result < 0.0f) result += 360.0f;
    return result;
}
struct Sample {
    RenderPose pose;
    std::uint64_t generation = 0;
    std::uintptr_t owner = 0;
    int key = 0, type = 0, detail = 0;
    bool live = false, teleport = false;
};
struct Track {
    Sample previous, current;
    bool published = false, continuous = false;
};
Track characters[401], parts[401][11], items[1000], points[100];
std::uint64_t characterGeneration[401] = {}, itemGeneration[1000] = {}, pointGeneration[100] = {};
int scene = -1, world = -1;

bool SameIdentity(const Sample& a, const Sample& b) {
    return a.live && b.live && a.generation == b.generation && a.owner == b.owner &&
           a.key == b.key && a.type == b.type && a.detail == b.detail;
}
bool Same(const Sample& a, const Sample& b) {
    return SameIdentity(a, b) && !a.teleport && !b.teleport;
}
void Begin(Track& track, const Sample& sample, bool sameScene) {
    track.continuous = sameScene && track.published && Same(track.current, sample);
    track.previous = sample;
}
void End(Track& track, const Sample& sample, bool sameScene) {
    track.continuous = track.continuous && sameScene && Same(track.previous, sample);
    track.current = sample;
    track.published = true;
}
CHARACTER* Character(int slot) {
    if (slot == 400) return (CHARACTER*)DAT_07abf050;
    return DAT_07abf5d0 ? ((CHARACTER*)DAT_07abf5d0) + slot : nullptr;
}
int CharacterSlot(const void* entity) {
    if (entity == DAT_07abf050) return 400;
    const std::uintptr_t address = (std::uintptr_t)entity, base = (std::uintptr_t)DAT_07abf5d0;
    if (!base || address < base || address - base >= 400 * sizeof(CHARACTER)) return -1;
    const std::uintptr_t offset = address - base;
    return offset % sizeof(CHARACTER) == 0 ? (int)(offset / sizeof(CHARACTER)) : -1;
}
Sample ObjectSample(const OBJECT& object) {
    Sample sample;
    sample.live = object.Live != 0;
    if (!sample.live) return sample; // El payload de un slot libre puede no estar inicializado.
    sample.type = object.Type;
    sample.teleport = object.Teleport != 0;
    memcpy(sample.pose.position, object.Position, sizeof(sample.pose.position));
    memcpy(sample.pose.angle, object.Angle, sizeof(sample.pose.angle));
    memcpy(sample.pose.headAngle, object.HeadAngle, sizeof(sample.pose.headAngle));
    sample.pose.animationFrame = object.AnimationFrame;
    sample.pose.priorAnimationFrame = object.PriorAnimationFrame;
    sample.pose.action = object.CurrentAction;
    sample.pose.priorAction = object.PriorAction;
    sample.pose.scale = object.Scale;
    sample.pose.alpha = object.Alpha;
    return sample;
}
Sample CharacterSample(int slot) {
    CHARACTER* character = Character(slot);
    if (!character) return {};
    Sample sample = ObjectSample(character->Object);
    sample.owner = (std::uintptr_t)character;
    sample.generation = characterGeneration[slot];
    if (sample.live) sample.key = character->Key;
    return sample;
}
PART_t* Part(CHARACTER& c, int part) {
    if (part < 6) return &c.BodyPart[part];
    if (part < 8) return &c.Weapon[part - 6];
    if (part == 8) return &c.Wing;
    if (part == 9) return &c.Helper;
    return &c.Flag;
}
Sample PartSample(int slot, int index) {
    Sample sample = CharacterSample(slot);
    CHARACTER* character = Character(slot);
    if (!character || !sample.live) return sample;
    const PART_t& part = *Part(*character, index);
    sample.live = part.Type != -1;
    if (!sample.live) return sample;
    sample.type = part.Type;
    // Cambiar nivel/opción/hueso sustituye la pieza aunque conserve el modelo.
    sample.detail = part.Level | (part.Option1 << 8) | (part.LinkBone << 16);
    sample.pose = {};
    sample.pose.animationFrame = part.AnimationFrame;
    sample.pose.priorAnimationFrame = part.PriorAnimationFrame;
    sample.pose.action = part.CurrentAction;
    sample.pose.priorAction = part.PriorAction;
    return sample;
}
Sample ItemSample(int slot) {
    Sample sample = ObjectSample(*(OBJECT*)(DAT_07e12840 + slot * 0x204 + 72));
    sample.owner = (std::uintptr_t)DAT_07e12840;
    sample.generation = itemGeneration[slot];
    return sample;
}
Sample PointSample(int slot) {
    const char* point = DAT_07c80110 + slot * 0x70;
    Sample sample;
    sample.live = point[0] != 0;
    if (!sample.live) return sample;
    sample.generation = pointGeneration[slot];
    sample.key = *(const int*)(point + 4);
    memcpy(sample.pose.position, point + 0x10, sizeof(sample.pose.position));
    sample.pose.scale = *(const float*)(point + 0x0c);
    sample.pose.alpha = *(const float*)(point + 0x48) * 0.40000001f;
    return sample;
}
void Capture(bool begin) {
    const bool sameScene = scene == SceneFlag && world == gMapManager.GetCurrentMap();
    for (int slot = 0; slot < 401; ++slot) {
        if (begin) Begin(characters[slot], CharacterSample(slot), sameScene);
        else End(characters[slot], CharacterSample(slot), sameScene);
        for (int part = 0; part < 11; ++part) {
            if (begin) Begin(parts[slot][part], PartSample(slot, part), sameScene);
            else End(parts[slot][part], PartSample(slot, part), sameScene);
        }
    }
    for (int slot = 0; slot < 1000; ++slot) {
        if (begin) Begin(items[slot], ItemSample(slot), sameScene);
        else End(items[slot], ItemSample(slot), sameScene);
    }
    for (int slot = 0; slot < 100; ++slot) {
        if (begin) Begin(points[slot], PointSample(slot), sameScene);
        else End(points[slot], PointSample(slot), sameScene);
    }
    scene = SceneFlag; world = gMapManager.GetCurrentMap();
}
bool Read(const Track& track, const Sample& live, RenderPosePair& out) {
    if (!live.live) return false;
    out.previous = live.pose;
    out.current = live.pose;
    out.continuous = false;
    if (!track.published || scene != SceneFlag || world != gMapManager.GetCurrentMap() ||
        !SameIdentity(track.current, live)) return true;
    out.previous = track.previous.pose;
    out.current = track.current.pose;
    out.continuous = track.continuous && !live.teleport;
    return true;
}
void LimitDisplacement(RenderPosePair& pair, float maximum) {
    float square = 0.0f;
    for (int axis = 0; axis < 3; ++axis) {
        const float delta = pair.current.position[axis] - pair.previous.position[axis];
        square += delta * delta;
    }
    pair.continuous = pair.continuous && maximum >= 0.0f && square <= maximum * maximum;
}
}

float RenderSnapshots_Alpha(int remainder) {
    return remainder <= 0 ? 0.0f : remainder >= 40 ? 1.0f : remainder / 40.0f;
}
RenderPose RenderSnapshots_Interpolate(const RenderPosePair& pair, float alpha) {
    if (!pair.continuous || alpha >= 1.0f) return pair.current;
    RenderPose pose = pair.current;
    if (alpha < 0.0f) alpha = 0.0f;
    for (int axis = 0; axis < 3; ++axis) {
        pose.position[axis] = Blend(pair.previous.position[axis], pair.current.position[axis], alpha);
        pose.angle[axis] = BlendAngle(pair.previous.angle[axis], pair.current.angle[axis], alpha);
        pose.headAngle[axis] = BlendAngle(pair.previous.headAngle[axis], pair.current.headAngle[axis], alpha);
    }
    const bool sameAction = pair.previous.action == pose.action && pair.previous.priorAction == pose.priorAction;
    if (sameAction && pose.animationFrame >= pair.previous.animationFrame &&
        pose.priorAnimationFrame >= pair.previous.priorAnimationFrame) {
        pose.animationFrame = Blend(pair.previous.animationFrame, pose.animationFrame, alpha);
        pose.priorAnimationFrame = Blend(pair.previous.priorAnimationFrame, pose.priorAnimationFrame, alpha);
    }
    pose.scale = Blend(pair.previous.scale, pose.scale, alpha);
    pose.alpha = Blend(pair.previous.alpha, pose.alpha, alpha);
    return pose;
}
void RenderSnapshots_BeginStep() { Capture(true); }
void RenderSnapshots_EndStep() { Capture(false); }
// DESVIACION: generaciones externas identifican reuso incluso dentro del mismo
// paso, sin ampliar CHARACTER, PART_t, ITEM_t ni los pools del binario.
void RenderSnapshots_NewCharacter(const void* entity) {
    Equipment_Invalidate(entity);
    const int slot = CharacterSlot(entity);
    if (slot >= 0) ++characterGeneration[slot];
}
void RenderSnapshots_NewItem(int slot) { if (slot >= 0 && slot < 1000) ++itemGeneration[slot]; }
void RenderSnapshots_NewDamagePoint(int slot) { if (slot >= 0 && slot < 100) ++pointGeneration[slot]; }
bool RenderSnapshots_Character(int slot, float maximum, RenderPosePair& out) {
    if (slot < 0 || slot >= 401 || !Read(characters[slot], CharacterSample(slot), out)) return false;
    LimitDisplacement(out, maximum);
    return true;
}
bool RenderSnapshots_Part(int slot, int part, float maximum, RenderPosePair& out) {
    if (slot < 0 || slot >= 401 || part < 0 || part >= 11 || !Read(parts[slot][part], PartSample(slot, part), out))
        return false;
    RenderPosePair owner;
    if (!RenderSnapshots_Character(slot, maximum, owner)) return false;
    // Una pieza tampoco mezcla animación si su propietario saltó de posición.
    out.continuous = out.continuous && owner.continuous;
    return true;
}
bool RenderSnapshots_Item(int slot, float maximum, RenderPosePair& out) {
    if (slot < 0 || slot >= 1000 || !Read(items[slot], ItemSample(slot), out)) return false;
    LimitDisplacement(out, maximum);
    return true;
}
bool RenderSnapshots_DamagePoint(int slot, RenderPosePair& out) {
    return slot >= 0 && slot < 100 && Read(points[slot], PointSample(slot), out);
}
