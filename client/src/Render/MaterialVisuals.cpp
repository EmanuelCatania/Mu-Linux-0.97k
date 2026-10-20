#include "stdafx.h"
#include "Render/MaterialVisuals.h"
#include "Render/LooseWingVisuals.h"
#include "Item/ContentCatalog.h"
#include "Config/UserSettings.h"
#include "Game/MapManager.h"
#include <unordered_map>
#include <vector>

extern "C" BYTE ShopItems[];

namespace {
const MaterialVisualSample *current=nullptr;
MaterialVisualSample empty;
struct ObjectSample { MaterialVisualSample sample; };
struct UiSamples { std::vector<MaterialVisualSample> values; std::size_t cursor=0; };
std::unordered_map<const void *, ObjectSample> objects;
std::unordered_map<int, UiSamples> ui;
int scene=-1, world=-1;
bool wingPlanning=false;
bool RandomMaterial(int type) {
    return type==414 || type==565 || type==567 || type==606 || type==607 || type==608;
}
void PrepareGrid(BYTE *pool, int cells) {
    for (int i=0; i<cells; ++i) {
        ITEM &item=*(ITEM *)(pool+i*68);
        if (item.Type!=-1 && (int)item.Key>0) MaterialVisuals_PrepareUiItem(item.Type,item.Level);
    }
}
}
void MaterialVisuals_BeginStep() {
    current=nullptr; objects.clear(); ui.clear();
    scene=SceneFlag; world=gMapManager.GetCurrentMap();
}
void MaterialVisuals_Prepare(MaterialVisualSample &sample, int model, bool attributes, bool draw) {
    sample=MaterialVisualSample{}; sample.model=model;
    // IDA: ItemObjectAttribute (0x00502BA0), mismo número y orden de muestras.
    if (attributes) {
        if (model==414 || model==567) sample.attribute[0]=rand();
        else if (model==565 || model==606) {
            sample.attribute[0]=rand(); sample.attribute[1]=rand();
        }
    }
    // IDA: BMD_SetupRenderByType (0x00504130), UV V antes que U.
    if (draw && (model==607 || model==608)) {
        sample.v=(float)(rand()%10)*_DAT_005524f4;
        sample.u=(float)(rand()%10)*_DAT_005524f4;
    }
}
void MaterialVisuals_PrepareObject(const void *owner, int model, bool attributes, bool draw) {
    if (!RandomMaterial(model)) return;
    MaterialVisuals_Prepare(objects[owner].sample, model, attributes, draw);
}
void MaterialVisuals_PrepareWeaponObject(int entity, int model) {
    OBJECT &o=*AsObject(entity);
    auto &sample=objects[(void *)entity].sample;
    const float alpha=o.Alpha;
    const BYTE action=o.CurrentAction;
    MaterialVisuals_Prepare(sample,model,true,alpha>_DAT_005524f8);
    sample.alpha=alpha; sample.action=action;
    // IDA: 239/244 usaban el slot como arma y dejaban estos resets publicados.
    // DESVIACION: publicar una vez por paso, conservando alpha/acción del draw.
    const short type=o.Type;
    const DWORD gravity=*(DWORD *)&o.Gravity;
    o.Type=(short)model;
    MaterialVisualScope scope(&sample);
    ItemObjectAttributeForRender(entity);
    o.Type=type;
    if (type==244) *(DWORD *)&o.Gravity=gravity;
}
void MaterialVisuals_TickObjectDraw(int entity) {
    const OBJECT &o=*AsObject(entity);
    if (gMapManager.GetCurrentMap()!=4 || o.Alpha<_DAT_005524f8 ||
        o.RenderType==0x80 || (DAT_07e11d30==0 && o.HiddenMesh==-2)) return;
    if (o.Type==23 || o.Type==19 || o.Type==20 || o.Type==3 || o.Type==4)
        rand(); // IDA: Draw_RenderObject (0x004FAE00), muestra descartada.
}
void MaterialVisuals_PrepareUiItem(int itemType, int level) {
    int drawLevel=level;
    const int model=ItemMaterials_ResolvePreviewModel(itemType,level,&drawLevel);
    if (wingPlanning ? model!=790 : !RandomMaterial(model)) return;
    if (!DAT_05828d58) return;
    const char *bmd=(const char *)DAT_05828d58+model*0xbc;
    const short meshes=*(const short *)(bmd+0x24);
    const auto meshBase=*(const unsigned int *)(bmd+0x28);
    // El mismo guard de RenderItem3D, antes de crear su objeto local.
    if (meshes<=0 || meshes>1000 || meshBase<0x100000) return;
    if (model==790) { LooseWingVisuals_ExpectPreview((unsigned int)drawLevel); return; }
    auto &samples=ui[model].values;
    samples.emplace_back();
    MaterialVisuals_Prepare(samples.back(),model,true,true);
}
void MaterialVisuals_BeginRender() {
    current=nullptr;
    for (auto &entry:ui) entry.second.cursor=0;
}
const MaterialVisualSample *MaterialVisuals_UiSample(int model) {
    if (scene!=SceneFlag || world!=gMapManager.GetCurrentMap()) return nullptr;
    const auto found=ui.find(model);
    if (found==ui.end()) return nullptr;
    auto &samples=found->second;
    return samples.cursor<samples.values.size() ? &samples.values[samples.cursor++] : nullptr;
}
const MaterialVisualSample *MaterialVisuals_ObjectSample(const void *owner, int model) {
    if (scene!=SceneFlag || world!=gMapManager.GetCurrentMap()) return nullptr;
    const auto found=objects.find(owner);
    return found!=objects.end() && found->second.sample.model==model ? &found->second.sample : nullptr;
}
const MaterialVisualSample *MaterialVisuals_Current() { return current; }
const MaterialVisualSample &MaterialVisuals_DrawSample(const void *owner, int model) {
    if (current && current->model==model) return *current;
    const auto *sample=MaterialVisuals_ObjectSample(owner,model);
    return sample ? *sample : empty;
}
MaterialVisualScope::MaterialVisualScope(const MaterialVisualSample *sample):previous(current) { current=sample; }
MaterialVisualScope::~MaterialVisualScope() { current=previous; }

// DESVIACION: recorrer en SIM sólo los productores de previews, sin GL ni
// hit-tests. Las muestras se conservan por modelo y ocurrencia en cada draw.
void MaterialVisuals_TickWingPreviews() {
    LooseWingVisuals_BeginStep();
    wingPlanning=true;
    MaterialVisuals_TickUi();
    wingPlanning=false;
    LooseWingVisuals_TickPreviews();
}

void MaterialVisuals_TickUi() {
    const bool ready=SceneFlag==2 ? DAT_083a7c49!=0 :
        SceneFlag==4 ? CharSelectSceneInitialized && DAT_05826cb0>50 :
        SceneFlag==5 && DAT_083a7c10 && DAT_07e11d1c<=30;
    if (!ready) return;
    if (SceneFlag==5 && !gUserSettings.GetAntilag(ANTILAG_INTERFACE)) {
        ItemMaterials_PrepareQuestUi();
        if (!GoldenArcherOpenType || GoldenArcherOpenType==3) {
            if (InventoryOpened) {
                ItemMaterials_PrepareEquipmentUi();
                PrepareGrid(OffsetInventoryItems,64);
            }
            if (ShopOpened) PrepareGrid(ShopItems,120);
            if (TradeOpened) { PrepareGrid(Inventory,32); PrepareGrid(OffsetTradeItems,32); }
            if (WarehouseOpened) PrepareGrid(OffsetWarehouseItems,120);
            if (ChaosMixOpened) PrepareGrid(OffsetMixItems,32);
            if (EventWindowOpened) PrepareGrid(OffsetMixItems,32);
            if (DAT_07e91388>0 && !DAT_07eaa13c && *(short *)DAT_07e91350>=0)
                MaterialVisuals_PrepareUiItem(*(short *)DAT_07e91350,*(int *)(DAT_07e91350+4));
            for (int slot=0; slot<3; ++slot) {
                const int index=Item_FindQuickSlotByCategory(slot);
                if (index!=-1) MaterialVisuals_PrepareUiItem(*(short *)(OffsetInventoryItems+index*68),*(int *)(OffsetInventoryItems+index*68+4));
            }
        }
    }
    if (DAT_083a7c24==0x97 && DAT_07eaa13c==1)
        MaterialVisuals_PrepareUiItem(*(short *)DAT_07e91350,*(int *)(DAT_07e91350+4));
    else if (DAT_083a7c24==0x99)
        MaterialVisuals_PrepareUiItem(*(short *)DAT_07ea5240,*(int *)(DAT_07ea5240+4));
}
