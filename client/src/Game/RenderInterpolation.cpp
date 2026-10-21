#include "stdafx.h"
#include "Game/RenderInterpolation.h"
#include "Game/RenderInterpolationMath.h"
#include "Game/RenderSnapshots.h"
#include "Game/RenderFxSnapshots.h"
#include "Scene/ObjectRegistry.h"
#include <algorithm>

namespace {
bool active=false;
float fraction=1;
int CharacterSlot(const void *owner) {
    if (owner==DAT_07abf050) return 400;
    const uintptr_t offset=(uintptr_t)owner-(uintptr_t)DAT_07abf5d0;
    return DAT_07abf5d0 && offset<400*sizeof(CHARACTER) && offset%sizeof(CHARACTER)==0 ? (int)(offset/sizeof(CHARACTER)) : -1;
}
void ApplyObject(OBJECT &object, const RenderPosePair &pair) {
    const RenderPose pose=RenderSnapshots_Interpolate(pair,fraction);
    memcpy(object.Position,pose.position,sizeof(pose.position));
    memcpy(object.Angle,pose.angle,sizeof(pose.angle));
    memcpy(object.HeadAngle,pose.headAngle,sizeof(pose.headAngle));
    object.AnimationFrame=pose.animationFrame; object.PriorAnimationFrame=pose.priorAnimationFrame;
    object.Scale=pose.scale; object.Alpha=pose.alpha;
}
RenderPose Pose(const unsigned char *bytes) {
    OBJECT object; memcpy(&object,bytes,sizeof(object));
    RenderPose pose;
    memcpy(pose.position,object.Position,sizeof(pose.position));
    memcpy(pose.angle,object.Angle,sizeof(pose.angle));
    memcpy(pose.headAngle,object.HeadAngle,sizeof(pose.headAngle));
    pose.animationFrame=object.AnimationFrame; pose.priorAnimationFrame=object.PriorAnimationFrame;
    pose.scale=object.Scale; pose.alpha=object.Alpha; pose.action=object.CurrentAction; pose.priorAction=object.PriorAction;
    return pose;
}
bool Near(const float *a, const float *b) {
    float square=0;
    for (int i=0;i<3;++i) { const float delta=b[i]-a[i]; square+=delta*delta; }
    return square<=RenderInterpolation_MaxDisplacement*RenderInterpolation_MaxDisplacement;
}
template<class T> T Load(const unsigned char *p, int offset) { T v; memcpy(&v,p+offset,sizeof(v)); return v; }
void BlendField(unsigned char *out, const RenderFxView &view, int offset, bool angle=false) {
    const float a=Load<float>(view.previous,offset),b=Load<float>(view.current,offset);
    const float value=angle ? RenderBlendAngle(a,b,fraction) : RenderBlend(a,b,fraction);
    memcpy(out+offset,&value,sizeof(value));
}
void BlendVector(unsigned char *out, const RenderFxView &view, int offset, bool angle=false) {
    for (int i=0;i<3;++i) BlendField(out,view,offset+i*4,angle);
}
bool Near(const RenderFxView &view, int offset) {
    float a[3],b[3]; memcpy(a,view.previous+offset,12); memcpy(b,view.current+offset,12);
    return Near(a,b);
}
void ApplyFx() {
    struct Source { void *base; int stride; };
    const Source sources[]={
        {DAT_07b11670,0x1bc},{DAT_07b27150,0x9d8},{DAT_07abf5f0,0x70},
        {DAT_07c5ab3c,0x70},{g_WeatherSlotPool,0x1bc},{DAT_083a1218,0x1bc},
        {DAT_083a2e90,0x1bc},{g_RenderPool_07c608a8,0x2f0},
        {g_PlayerRenderPool,0x1bc},{g_PointerPool,0x70}
    };
    static_assert(sizeof(sources)/sizeof(sources[0])==(int)RenderFxPool::Count,"Pools interpolados");
    for (int domain=0;domain<(int)RenderFxPool::Count;++domain) {
        const auto pool=(RenderFxPool)domain;
        for (int index=0;index<RenderFxSnapshots_Count(pool);++index) {
            RenderFxView view;
            if (!RenderFxSnapshots_Read(pool,index,view) || !view.continuous) continue;
            auto *out=(unsigned char *)sources[domain].base+index*sources[domain].stride;
            if (pool==RenderFxPool::Effects || pool==RenderFxPool::Weather ||
                pool==RenderFxPool::Bugs || pool==RenderFxPool::Ambient || pool==RenderFxPool::Fades) {
                RenderPosePair pair{Pose(view.previous),Pose(view.current),true};
                OBJECT current; memcpy(&current,view.current,sizeof(current));
                OBJECT previous; memcpy(&previous,view.previous,sizeof(previous));
                pair.continuous=!current.Teleport && !previous.Teleport && Near(pair.previous.position,pair.current.position);
                ApplyObject(*(OBJECT *)out,pair);
            } else if (pool==RenderFxPool::Joints) {
                if (!Near(view,0x10) || !Near(view,0x44)) continue;
                const int previousCount=Load<int>(view.previous,0x50),count=Load<int>(view.current,0x50);
                if (previousCount<0 || count<0 || previousCount>=MAX_JOINT_TAILS || count>=MAX_JOINT_TAILS ||
                    Load<int>(view.previous,0x54)!=Load<int>(view.current,0x54)) continue;
                BlendVector(out,view,0x10); BlendVector(out,view,0x28,true); BlendVector(out,view,0x44);
                BlendVector(out,view,0x34); BlendField(out,view,0x0c);
                // La cola nueva nace colapsada en el último extremo previo;
                // NumTails/MaxTails y lifetime conservan siempre su valor SIM.
                for (int row=0;row<=count;++row) for (int component=0;component<12;++component) {
                    const int oldOffset=0x58+(std::min)(row,previousCount)*48+component*4;
                    const int offset=0x58+row*48+component*4;
                    const float value=RenderBlend(Load<float>(view.previous,oldOffset),Load<float>(view.current,offset),fraction);
                    memcpy(out+offset,&value,4);
                }
            } else if (pool==RenderFxPool::Trails) {
                const int oldCount=Load<int>(view.previous,16),count=Load<int>(view.current,16);
                if (oldCount<1 || oldCount>30 || count<1 || count>30) continue;
                if (!Near(view,32) || !Near(view,392)) continue;
                for (int start:{32,392}) for (int row=0;row<count;++row) for (int axis=0;axis<3;++axis) {
                    const int offset=start+row*12+axis*4,oldOffset=start+(std::min)(row,oldCount-1)*12+axis*4;
                    const float value=RenderBlend(Load<float>(view.previous,oldOffset),Load<float>(view.current,offset),fraction);
                    memcpy(out+offset,&value,4);
                }
                BlendVector(out,view,20);
            } else {
                if (!Near(view,0x10)) continue;
                BlendVector(out,view,0x10);
                if (pool==RenderFxPool::Particles || pool==RenderFxPool::Leaves) {
                    BlendField(out,view,0x0c); BlendVector(out,view,0x1c,true); BlendVector(out,view,0x28);
                    if (pool==RenderFxPool::Particles) { BlendField(out,view,0x40,true); BlendField(out,view,0x48); }
                } else if (pool==RenderFxPool::Flares) {
                    BlendField(out,view,0x0c); BlendField(out,view,0x24,true);
                    BlendVector(out,view,0x28); BlendField(out,view,0x34);
                }
            }
        }
    }
}
}
float RenderInterpolation_Alpha() { return active ? fraction : 1.0f; }
bool RenderInterpolation_OwnerContinuous(const void *owner) {
    RenderPosePair pair;
    const int slot=CharacterSlot(owner);
    if (!active) return false;
    if (slot>=0) return RenderSnapshots_Character(slot,RenderInterpolation_MaxDisplacement,pair) && pair.continuous;
    if (SceneObjects::Contains(owner)) return RenderSnapshots_Object(owner,RenderInterpolation_MaxDisplacement,pair) && pair.continuous;
    const uintptr_t itemOffset=(uintptr_t)owner-((uintptr_t)DAT_07e12840+72);
    if (DAT_07e12840 && itemOffset<1000*0x204 && itemOffset%0x204==0)
        return RenderSnapshots_Item((int)(itemOffset/0x204),RenderInterpolation_MaxDisplacement,pair) && pair.continuous;
    RenderFxView view;
    return RenderFxSnapshots_Owner(owner,view) && view.bytes!=0x2f0 && view.continuous && Near(view,0x10);
}
void RenderInterpolation_Begin(int remainder) {
    active=true; fraction=RenderSnapshots_Alpha(remainder);
    // DESVIACION: sólo campos de presentación; el guard ya capturó SIM.
    RenderCameraPair camera;
    if (RenderSnapshots_Camera(RenderInterpolation_MaxDisplacement,camera) && camera.continuous) {
        for (int axis=0;axis<3;++axis) {
            CameraPosition[axis]=RenderBlend(camera.previous.position[axis],camera.current.position[axis],fraction);
            CameraAngle[axis]=RenderBlendAngle(camera.previous.angle[axis],camera.current.angle[axis],fraction);
        }
        CameraFOV=RenderBlend(camera.previous.fov,camera.current.fov,fraction);
        CameraViewFar=RenderBlend(camera.previous.farPlane,camera.current.farPlane,fraction);
        CameraDistance=RenderBlend(camera.previous.distance,camera.current.distance,fraction);
    }
    for (int slot=0;slot<401;++slot) {
        auto *c=slot==400 ? (CHARACTER *)DAT_07abf050 : DAT_07abf5d0 ? (CHARACTER *)DAT_07abf5d0+slot : nullptr;
        RenderPosePair pair;
        if (!c || !RenderSnapshots_Character(slot,RenderInterpolation_MaxDisplacement,pair)) continue;
        ApplyObject(c->Object,pair);
        for (int part=0;part<11;++part) {
            if (!RenderSnapshots_Part(slot,part,RenderInterpolation_MaxDisplacement,pair)) continue;
            PART_t *piece=part<6 ? &c->BodyPart[part] : part<8 ? &c->Weapon[part-6] : part==8 ? &c->Wing : part==9 ? &c->Helper : &c->Flag;
            const RenderPose pose=RenderSnapshots_Interpolate(pair,fraction);
            piece->AnimationFrame=pose.animationFrame; piece->PriorAnimationFrame=pose.priorAnimationFrame;
        }
    }
    if (DAT_07e12840) for (int slot=0;slot<1000;++slot) {
        RenderPosePair pair;
        if (RenderSnapshots_Item(slot,RenderInterpolation_MaxDisplacement,pair))
            ApplyObject(*(OBJECT *)((unsigned char *)DAT_07e12840+slot*0x204+72),pair);
    }
    for (const void *node:SceneObjects::LiveNodes()) {
        RenderPosePair pair;
        if (RenderSnapshots_Object(node,RenderInterpolation_MaxDisplacement,pair)) ApplyObject(*(OBJECT *)node,pair);
    }
    ApplyFx();
}
void RenderInterpolation_End() { active=false; fraction=1; }

bool RenderInterpolation_OwnerOffset(const void *owner, float *offset) {
    if (!RenderInterpolation_OwnerContinuous(owner)) return false;
    RenderPosePair pair;
    bool pose=false;
    const int slot=CharacterSlot(owner);
    if (slot>=0) pose=RenderSnapshots_Character(slot,RenderInterpolation_MaxDisplacement,pair);
    else if (SceneObjects::Contains(owner)) pose=RenderSnapshots_Object(owner,RenderInterpolation_MaxDisplacement,pair);
    else {
        const uintptr_t itemOffset=(uintptr_t)owner-((uintptr_t)DAT_07e12840+72);
        if (DAT_07e12840 && itemOffset<1000*0x204 && itemOffset%0x204==0)
            pose=RenderSnapshots_Item((int)(itemOffset/0x204),RenderInterpolation_MaxDisplacement,pair);
    }
    if (!pose) {
        RenderFxView view;
        if (!RenderFxSnapshots_Owner(owner,view)) return false;
        memcpy(pair.previous.position,view.previous+0x10,12);
        memcpy(pair.current.position,view.current+0x10,12);
    }
    for (int axis=0;axis<3;++axis) offset[axis]=RenderBlend(pair.previous.position[axis],pair.current.position[axis],fraction)-pair.current.position[axis];
    return true;
}
