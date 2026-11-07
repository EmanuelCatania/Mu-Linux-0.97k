#include "stdafx.h"
#include "Physics/ClothTypes.h"
#include "Entity/ClothPresentation.h"
#include "Game/RenderInterpolation.h"
#include "Game/RenderInterpolationMath.h"
#include "Game/MapManager.h"
#include <unordered_map>
#include <vector>

namespace {
struct Point { float p[3]; };
struct Sample {
    const void *nodes=nullptr, *owner=nullptr;
    unsigned long long step=0;
    std::vector<Point> previous,current;
    bool continuous=false;
};
std::unordered_map<const void *,Sample> samples;
unsigned long long step=0;
int scene=-1,world=-1;
void Capture(void *cloth, std::vector<Point>& points) {
    const int count=ClothSystem(cloth).NodeCount;
    const char *nodes=reinterpret_cast<const char*>(ClothSystem(cloth).Nodes);
    if (count<=0 || !nodes) { points.clear(); return; }
    points.resize(count);
    for (int i=0;i<count;++i) memcpy(points[i].p,ClothSystem(cloth).Nodes[i].Position,sizeof(points[i].p));
}
}
void ClothPresentation_BeginStep() {
    ++step;
    if (scene!=SceneFlag || world!=gMapManager.GetCurrentMap()) samples.clear();
    scene=SceneFlag; world=gMapManager.GetCurrentMap();
}
void ClothPresentation_Forget(const void *cloth) { samples.erase(cloth); }
void ClothPresentation_Before(void *cloth) {
    Sample &sample=samples[cloth];
    if (sample.step==step) return; // Varias llamadas SIM conservan el mismo previo.
    const void *nodes=ClothSystem(cloth).Nodes, *owner=ClothSystem(cloth).Owner;
    sample.continuous=sample.step+1==step && sample.nodes==nodes && sample.owner==owner;
    sample.nodes=nodes; sample.owner=owner;
    Capture(cloth,sample.previous);
}
void ClothPresentation_After(void *cloth) {
    Sample &sample=samples[cloth];
    Capture(cloth,sample.current);
    sample.continuous=sample.continuous && sample.previous.size()==sample.current.size();
    sample.step=step;
}
// DESVIACION: sólo sustituir el vértice enviado al draw; nunca escribir nodos.
void ClothPresentation_Vertex(const void *cloth, int index, float *position) {
    const auto found=samples.find(cloth);
    if (found==samples.end() || scene!=SceneFlag || world!=gMapManager.GetCurrentMap()) return;
    const Sample &sample=found->second;
    if (!sample.continuous || sample.step!=step || index<0 || (size_t)index>=sample.current.size() ||
        !RenderInterpolation_OwnerContinuous(sample.owner)) return;
    for (int axis=0;axis<3;++axis)
        position[axis]=RenderBlend(sample.previous[index].p[axis],sample.current[index].p[axis],RenderInterpolation_Alpha());
}
