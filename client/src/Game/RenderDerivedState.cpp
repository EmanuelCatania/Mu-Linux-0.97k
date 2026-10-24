#include "stdafx.h"
#include "Render/ResourceTables.h"
#include "Game/RenderDerivedState.h"
#include "Game/RenderFxSnapshots.h"
#include "Scene/ObjectRegistry.h"
#include "Item/ItemDefines.h"
#include "Game/MapManager.h"
#include <vector>
#include "Game/FramePointerSet.h"

namespace {
struct Block {
    void *address;
    const void *owner;
    std::uint64_t generation;
    bool registered, bones, valid=true;
    std::vector<unsigned char> bytes;
};
std::vector<Block> blocks;
FramePointerSet savedObjects;
std::size_t used=0;
bool active=false;
int scene=-1, world=-1;
DWORD models=0;
void Save(void *address, std::size_t size, const void *owner=nullptr,
    bool registered=false, bool bones=false) {
    if (!address || !size) return;
    // Incluir capturas diferidas durante el dibujo, sin duplicar el ámbito inicial.
    RenderProfile::InterpolationScope measured(RenderProfile::InterpolationWork::Capture);
    if (used==blocks.size()) blocks.emplace_back();
    Block &block=blocks[used++];
    block.address=address; block.owner=owner;
    block.registered=registered; block.bones=bones; block.valid=true;
    block.generation=registered ? SceneObjects::Generation(owner) : 0;
    block.bytes.resize(size);
    memcpy(block.bytes.data(),address,size);
}
void SaveObject(void *address, std::size_t size, bool registered=false) {
    if (!address || !savedObjects.Insert(address)) return;
    Save(address,size,address,registered);
    const auto found=SceneObjects::Bones().find(address);
    if (found!=SceneObjects::Bones().end() &&
        found->second.buffer==AsObject(address)->BoneMatrices)
        Save((void *)found->second.buffer,found->second.bytes,address,registered,true);
}
}
// DESVIACION: los cambios de presentación viven durante la pasada completa,
// conservando su orden interno. Ninguna pose/OBB/material interpolados llegan
// al siguiente input o tick. Las altas invalidan su copia previa; no se
// suprimen llamadas a creadores. Los sprites conservan su Capture/ReplayStep.
void RenderDerivedState_Begin() {
    active=false; used=0; savedObjects.Reset();
    const bool ready=SceneFlag==2 ? DAT_083a7c49!=0 :
        SceneFlag==4 ? CharSelectSceneInitialized && DAT_05826cb0>50 :
        SceneFlag==5 && DAT_083a7c10 && DAT_07e11d1c<=30;
    if (!ready) return;
    active=true; scene=SceneFlag; world=gMapManager.GetCurrentMap(); models=DAT_05828d58;
    // DESVIACION: los slots libres de personajes e items no se dibujan ni se
    // interpolan; copiar sólo los vivos.
    if (DAT_07abf5d0) for (int slot=0; slot<400; ++slot) {
        auto *character=(CHARACTER *)DAT_07abf5d0+slot;
        if (character->Object.Live) SaveObject(character,sizeof(CHARACTER));
    }
    SaveObject(DAT_07abf050,sizeof(CHARACTER));
    if (DAT_07e12840) for (int slot=0; slot<1000; ++slot) {
        auto *item=(BYTE *)DAT_07e12840+slot*0x204+72;
        if (AsObject(item)->Live) SaveObject(item,sizeof(OBJECT));
    }
    for (int slot=0; slot<200; ++slot) SaveObject(DAT_07b11670+slot*0x1bc,sizeof(OBJECT));
    for (int slot=0; slot<40; ++slot) SaveObject(g_WeatherSlotPool+slot*0x1bc,sizeof(OBJECT));
    for (int slot=0; slot<10; ++slot) SaveObject(DAT_083a1218+slot*0x1bc,sizeof(OBJECT));
    // Los objetos del mapa se guardan al tocarlos (RenderDerivedState_SaveObject):
    // el dibujo sólo modifica los visibles y la interpolación los que se movieron.
    RenderFxSnapshots_SaveRenderState();
    if (models) Save((void *)models,ResourceTables::ModelCount()*0xbc);
    Save(DAT_05826e18,sizeof(DAT_05826e18));
    Save(g_BoneScratch,sizeof(g_BoneScratch));
    Save(&DAT_06989c9c,12*sizeof(float));
    Save(&_DAT_005597c8,sizeof(_DAT_005597c8));
    Save(&ObjectSelect_Type,sizeof(ObjectSelect_Type));
    Save(&ObjectSelect_Angle,sizeof(ObjectSelect_Angle));
    Save(&_DAT_07ea9530,sizeof(_DAT_07ea9530));
    Save(&_DAT_07ea9534,sizeof(_DAT_07ea9534));
    Save(&ObjectSelect_HeadAngle,sizeof(ObjectSelect_HeadAngle));
    Save(&ObjectSelect_AnimationFrame,sizeof(ObjectSelect_AnimationFrame));
    Save(&ObjectSelect_PriorAnimationFrame,sizeof(ObjectSelect_PriorAnimationFrame));
    Save(&ObjectSelect_PriorAction,sizeof(ObjectSelect_PriorAction));
}
void RenderDerivedState_SaveObject(void *node) {
    if (active && node) SaveObject(node,sizeof(OBJECT),true);
}
void RenderDerivedState_SaveFx(void *slot, std::size_t size) {
    if (active && savedObjects.Insert(slot)) Save(slot,size,slot);
}
// Una recarga puede conservar escena/mapa/base BMD: invalida explícitamente.
void RenderDerivedState_Cancel() { active=false; }
void RenderDerivedState_Forget(const void *owner) {
    if (!active) return;
    for (std::size_t i=0; i<used; ++i) if (blocks[i].owner==owner) blocks[i].valid=false;
}
void RenderDerivedState_End() {
    if (!active) return;
    active=false;
    if (scene!=SceneFlag || world!=gMapManager.GetCurrentMap() || models!=DAT_05828d58) return;
    for (std::size_t i=0; i<used; ++i) {
        Block &block=blocks[i];
        if (!block.valid) continue;
        // Validar antes de leer o escribir una dirección que pudo liberarse.
        if (block.registered && (!SceneObjects::Contains(block.owner) ||
            SceneObjects::Generation(block.owner)!=block.generation)) continue;
        if (block.bones && !SceneObjects::HasBoneRange(block.owner,block.address,0,block.bytes.size())) continue;
        memcpy(block.address,block.bytes.data(),block.bytes.size());
    }
}
