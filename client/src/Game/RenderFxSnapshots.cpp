#include "stdafx.h"
#include "Game/RenderFxSnapshots.h"
#include "Game/RenderDerivedState.h"
#include "Game/MapManager.h"
#include <cstdint>
#include <vector>

namespace {
struct Identity {
    std::uint64_t generation=0, epoch=0;
    unsigned int type=0, subtype=0, owner=0;
    bool live=false;
};
struct Pool {
    unsigned char *base;
    std::size_t bytes;
    int count, typeOffset, typeBytes, subtypeOffset, ownerOffset;
    std::vector<unsigned char> previous, current;
    std::vector<Identity> before, after;
    std::vector<std::uint64_t> generations;
    std::vector<bool> continuous;
    Pool(void *value, std::size_t stride, int size, int type, int width, int sub, int owner):
        base((unsigned char *)value), bytes(stride), count(size), typeOffset(type),
        typeBytes(width), subtypeOffset(sub), ownerOffset(owner),
        previous(stride*size), current(stride*size), before(size), after(size),
        generations(size), continuous(size,false) {}
};
Pool *Pools() {
    // IDA: OBJECT 0x1BC; JOINT 0x9D8; partículas/hojas 0x70; blur 0x2F0.
    // Los offsets se toman de los creadores/lectores activos, no de los anclajes
    // históricos DAT que pueden apuntar al interior del primer slot.
    static Pool pools[] = {
        {DAT_07b11670,0x1bc,200,2,2,4,0xfc},
        {DAT_07b27150,0x9d8,500,4,4,8,0x40},
        {DAT_07abf5f0,0x70,3000,4,4,8,0x3c},
        {DAT_07c5ab3c,0x70,200,4,4,-1,-1},
        {g_WeatherSlotPool,0x1bc,40,2,2,4,0xfc},
        {DAT_083a1218,0x1bc,10,2,2,4,0xfc},
        {DAT_083a2e90,0x1bc,10,2,2,4,0xfc},
        {g_RenderPool_07c608a8,0x2f0,100,4,4,-1,12},
        {g_PlayerRenderPool,0x1bc,100,2,2,4,0xfc},
        {g_PointerPool,0x70,100,4,4,-1,-1}
    };
    static_assert(sizeof(pools)/sizeof(pools[0])==(int)RenderFxPool::Count,"Pools FX");
    return pools;
}
int scene=-1, world=-1;
DWORD models=0;
std::uint64_t epoch=0;
bool published=false;
bool Ready() {
    return SceneFlag==2 ? DAT_083a7c49!=0 : SceneFlag==4 ?
        CharSelectSceneInitialized && DAT_05826cb0>50 :
        SceneFlag==5 && DAT_083a7c10 && DAT_07e11d1c<=30;
}
bool Context() {
    return Ready() && scene==SceneFlag && world==gMapManager.GetCurrentMap() && models==DAT_05828d58;
}
Identity ReadIdentity(const Pool& pool, int index) {
    Identity id;
    const unsigned char *source=pool.base+index*pool.bytes;
    id.live=source[0]!=0;
    if (!id.live) return id; // Nunca interpretar el payload de un slot libre.
    id.generation=pool.generations[index]; id.epoch=epoch;
    memcpy(&id.type,source+pool.typeOffset,pool.typeBytes);
    if (pool.subtypeOffset>=0) memcpy(&id.subtype,source+pool.subtypeOffset,4);
    if (pool.ownerOffset>=0) memcpy(&id.owner,source+pool.ownerOffset,4);
    return id;
}
bool Same(const Identity& a, const Identity& b) {
    return a.live && b.live && a.generation==b.generation && a.epoch==b.epoch &&
        a.type==b.type && a.subtype==b.subtype && a.owner==b.owner;
}
}

// DESVIACION: capturas externas en CADA paso; ninguna escribe estado del juego.
void RenderFxSnapshots_BeginStep() {
    const bool context=published && Context();
    for (int domain=0;domain<(int)RenderFxPool::Count;++domain) {
        Pool &pool=Pools()[domain];
        for (int index=0;index<pool.count;++index) {
            const Identity id=ReadIdentity(pool,index);
            pool.continuous[index]=context && Same(pool.after[index],id);
            pool.before[index]=id;
            if (id.live) memcpy(pool.previous.data()+index*pool.bytes,pool.base+index*pool.bytes,pool.bytes);
        }
    }
    scene=SceneFlag; world=gMapManager.GetCurrentMap(); models=DAT_05828d58;
}
void RenderFxSnapshots_EndStep() {
    const bool context=Context();
    for (int domain=0;domain<(int)RenderFxPool::Count;++domain) {
        Pool &pool=Pools()[domain];
        for (int index=0;index<pool.count;++index) {
            const Identity id=ReadIdentity(pool,index);
            pool.continuous[index]=pool.continuous[index] && context && Same(pool.before[index],id);
            pool.after[index]=id;
            if (id.live) memcpy(pool.current.data()+index*pool.bytes,pool.base+index*pool.bytes,pool.bytes);
        }
    }
    published=context;
}
void RenderFxSnapshots_New(const void *slot) {
    const std::uintptr_t address=(std::uintptr_t)slot;
    for (int domain=0;domain<(int)RenderFxPool::Count;++domain) {
        Pool &pool=Pools()[domain];
        const std::uintptr_t offset=address-(std::uintptr_t)pool.base;
        if (offset<pool.bytes*pool.count && offset%pool.bytes==0) {
            RenderDerivedState_Forget(slot);
            ++pool.generations[offset/pool.bytes];
            return;
        }
    }
}
void RenderFxSnapshots_Reset() { ++epoch; published=false; RenderDerivedState_Cancel(); }
int RenderFxSnapshots_Count(RenderFxPool domain) {
    const int index=(int)domain;
    return index>=0 && index<(int)RenderFxPool::Count ? Pools()[index].count : 0;
}
bool RenderFxSnapshots_Read(RenderFxPool domain, int index, RenderFxView& out) {
    const int count=RenderFxSnapshots_Count(domain);
    out={};
    if (index<0 || index>=count || !Ready()) return false;
    const Pool &pool=Pools()[(int)domain];
    const Identity live=ReadIdentity(pool,index);
    if (!live.live) return false;
    out.bytes=pool.bytes;
    out.previous=out.current=pool.base+index*pool.bytes;
    if (!published || !Context() || !Same(pool.after[index],live)) return true;
    out.current=pool.current.data()+index*pool.bytes;
    out.continuous=pool.continuous[index];
    // Un alta sólo tiene muestra actual; no exponer bytes viejos como previo.
    out.previous=out.continuous ? pool.previous.data()+index*pool.bytes : out.current;
    return true;
}

void RenderFxSnapshots_SaveRenderState() {
    for (int domain=0;domain<(int)RenderFxPool::Count;++domain) {
        const Pool &pool=Pools()[domain];
        for (int index=0;index<pool.count;++index) {
            const unsigned char *slot=pool.base+index*pool.bytes;
            if (slot[0]) RenderDerivedState_SaveFx((void *)slot,pool.bytes);
        }
    }
}

bool RenderFxSnapshots_Owner(const void *owner, RenderFxView& out) {
    for (int domain=0;domain<(int)RenderFxPool::Count;++domain) {
        const Pool &pool=Pools()[domain];
        const uintptr_t offset=(uintptr_t)owner-(uintptr_t)pool.base;
        if (offset<pool.bytes*pool.count && offset%pool.bytes==0)
            return RenderFxSnapshots_Read((RenderFxPool)domain,(int)(offset/pool.bytes),out);
    }
    return false;
}
