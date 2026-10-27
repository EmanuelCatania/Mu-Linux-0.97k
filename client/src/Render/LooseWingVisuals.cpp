#include "stdafx.h"
#include "Render/LooseWingVisuals.h"
#include "Render/EquipmentVisuals.h"
#include "Game/MapManager.h"
#include <array>
#include <memory>
#include <vector>

namespace {
struct Preview {
    OBJECT object{};
    float angle[3]{}, headAngle[3]{}, bodyHeight=0, time=0;
    unsigned int level=0;
    bool spin=false;
};
struct Owner {
    OBJECT object{};
    std::array<float,200*12> bones{};
};
std::vector<Preview> recording, published;
std::vector<unsigned int> expected;
// Sólo crece hasta la mayor cantidad simultánea de previews. Los objetos no
// cambian de dirección ni se liberan mientras un joint pueda conservar Target.
std::vector<std::unique_ptr<Owner>> owners;
int renderScene=-1, renderWorld=-1, publishedScene=-1, publishedWorld=-1;
DWORD renderModels=0, publishedModels=0;
bool inRender=false;

struct Scratch {
    char bones[sizeof(g_BoneScratch)], quaternions[sizeof(DAT_05826e18)];
    float root[12], lod;
    Scratch():lod(_DAT_005597c8) {
        memcpy(bones,g_BoneScratch,sizeof(bones));
        memcpy(quaternions,DAT_05826e18,sizeof(quaternions));
        memcpy(root,&DAT_06989c9c,sizeof(root));
    }
    ~Scratch() {
        memcpy(g_BoneScratch,bones,sizeof(bones));
        memcpy(DAT_05826e18,quaternions,sizeof(quaternions));
        memcpy(&DAT_06989c9c,root,sizeof(root));
        _DAT_005597c8=lod;
    }
};
}

void LooseWingVisuals_BeginStep() { expected.clear(); }
void LooseWingVisuals_ExpectPreview(unsigned int level) { expected.push_back(level); }
void LooseWingVisuals_BeginRender() {
    recording.clear(); inRender=true;
    renderScene=SceneFlag; renderWorld=gMapManager.GetCurrentMap(); renderModels=DAT_05828d58;
}
void LooseWingVisuals_SubmitPreview(const OBJECT &object, const float *angle,
    const float *headAngle, float bodyHeight, unsigned int level, bool spin) {
    if (!inRender || object.Type!=790) return;
    Preview value;
    value.object=object; value.bodyHeight=bodyHeight; value.time=WorldTime;
    value.level=level; value.spin=spin;
    memcpy(value.angle,angle,sizeof(value.angle));
    memcpy(value.headAngle,headAngle,sizeof(value.headAngle));
    recording.push_back(value);
}
void LooseWingVisuals_EndRender() {
    if (!inRender) return;
    inRender=false;
    published.clear();
    if (renderScene!=SceneFlag || renderWorld!=gMapManager.GetCurrentMap() || renderModels!=DAT_05828d58) return;
    published.swap(recording);
    publishedScene=renderScene; publishedWorld=renderWorld; publishedModels=renderModels;
}

// DESVIACION: la UI publica sólo parámetros de la última pasada completa,
// nunca huesos ni direcciones de pila. El planificador SIM valida presencia y
// multiplicidad actuales; cada paso calcula su propia pose sin GL. Los joints
// de vida 0 vencen en MoveJoints antes del próximo draw del mundo, como antes.
// Los sprites pendientes se capturan al final del paso y se reproducen allí.
void LooseWingVisuals_TickPreviews() {
    for (auto &owner:owners) owner->object.Live=0;
    if (publishedScene!=SceneFlag || publishedWorld!=gMapManager.GetCurrentMap() ||
        publishedModels!=DAT_05828d58 || !DAT_05828d58) return;
    std::vector<bool> consumed(expected.size(),false);
    for (std::size_t index=0;index<published.size();++index) {
        const Preview &value=published[index];
        std::size_t match=0;
        while (match<expected.size() && (consumed[match] || expected[match]!=value.level)) ++match;
        if (match==expected.size()) continue; // La celda ya no está en la UI lógica.
        consumed[match]=true;
        while (owners.size()<=index) owners.emplace_back(new Owner);
        Owner &owner=*owners[index];
        owner.object=value.object;
        owner.object.Live=1;
        owner.object.EnableBoneMatrix=1;
        owner.object.BoneMatrices=(vec34_t *)owner.bones.data();
        void *model=(void *)(DAT_05828d58+790*0xbc);
        EquipmentModelContext context(model);
        Scratch scratch;
        BMD &b=*AsModel(model);
        b.CurrentAction=0; b.BodyHeight=value.bodyHeight; b.BodyScale=owner.object.Scale;
        memcpy(b.BodyOrigin,owner.object.Position,sizeof(b.BodyOrigin));
        float angle[3]; memcpy(angle,value.angle,sizeof(angle));
        if (value.spin) angle[1]=value.time*0.45f;
        float head[3]; memcpy(head,value.headAngle,sizeof(head));
        BMD_Animation(model,(int)owner.bones.data(),0,0,0,(unsigned int *)angle,head,0,0);
        memcpy(g_BoneScratch,owner.bones.data(),sizeof(g_BoneScratch));
        // La emisión pendiente pertenece a la pasada publicada: conservar su
        // fase temporal, como los sprites que antes sobrevivían hasta este paso.
        const float simulationTime=WorldTime;
        WorldTime=value.time;
        Equipment_EmitWing790(model,(int)&owner.object,(int)&owner.object);
        WorldTime=simulationTime;
    }
}

// IDA: pose de Entity_Render (0x005038E0), antes de RenderPartObject.
void LooseWingVisuals_TickGround(int entity) {
    OBJECT &o=*AsObject(entity);
    if (o.Type!=790 || o.Alpha<=_DAT_005524f8) return;
    void *model=(void *)(DAT_05828d58+790*0xbc);
    EquipmentModelContext context(model);
    Scratch scratch;
    BMD &b=*AsModel(model);
    b.CurrentAction=o.CurrentAction;
    Entity_SetGravity(o.Type,model);
    b.BodyScale=o.Scale;
    memcpy(b.BodyOrigin,o.Position,sizeof(b.BodyOrigin));
    memset(g_BoneScratch,0,sizeof(g_BoneScratch));
    unsigned int prior; memcpy(&prior,&o.PriorAnimationFrame,sizeof(prior));
    BMD_Animation(model,(int)g_BoneScratch,o.AnimationFrame,prior,o.PriorAction,
        (unsigned int *)o.Angle,o.HeadAngle,0,0);
    Equipment_EmitWing790(model,entity,entity);
}
