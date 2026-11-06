// MeshDraw con WebGPU: mallas BMD residentes con skinning en la GPU. Las
// validaciones y el color/UV que deja cada envío son los del backend GL. Lo
// que no admite el camino residente (chrome, una pose deformada en CPU)
// devuelve false y el juego dibuja la malla por el camino de referencia.
#include "Render/Backend/Api.h"
#include "Render/Backend/WebGPU/Renderer.h"
#include "Render/MeshTopology.h"
#include "Render/RenderState.h"
#include "Physics/ClothTypes.h"
#include <algorithm>
#include <cstring>

extern "C" void DbgLogPublic(const char*);

namespace MeshDraw::WGPU {
namespace {
// Color y UV actuales tras el envío: los de la última esquina, como la ruta CPU.
void SetCurrent(const float* color,const float* uv) {
    auto& s=RenderState::Mutable();std::memcpy(s.color,color,sizeof(s.color));std::memcpy(s.uv,uv,sizeof(s.uv));
}
}
bool DrawShadow(const void* model,const char* meshRecord,const float* world,const float* origin) {
    if(!::WebGPU::ResidentEnabled() || !model || !meshRecord) return false;
    const auto* source=reinterpret_cast<const ClothModelMesh*>(meshRecord);
    Input input{};
    input.key=meshRecord;
    input.faces=source->Triangles;
    input.faceCount=source->TriangleCount;input.positionCount=source->VertexCount;
    input.normalCount=source->NormalCount;input.uvCount=source->TexCoordCount;
    input.positions=world;input.uv=static_cast<const float*>(source->TexCoords);
    input.model=model;
    input.rawVertices=source->Vertices;
    input.rawNormals=static_cast<const unsigned char*>(source->Normals);
    if(input.faceCount<=0 || input.positionCount<=0 || !input.faces || !world) return false;
    MeshTopology::Mesh* mesh=MeshTopology::Get(input);if(!mesh) return false;
    if(mesh->minimum.position<0 || mesh->maximum.position>=input.positionCount) return false;
    if(!MeshTopology::Resident(*mesh,input)) return false;
    // La ruta CPU emite cada vértice con el color y la UV actuales.
    const auto current=BufferedDraw::CaptureAttributes(true,true);
    static const float noLight[3]{};
    Skinning::ResidentDraw draw{model,input.key,0,mesh->indices.size(),mesh->maxBone,
        world,input.positionCount,nullptr,input.normalCount,false,false,noLight,current.color,current.uv,0.f,0.f};
    draw.shadow=true;draw.shadowOrigin=origin;
    return ::WebGPU::DrawResidentMesh(input.key,*mesh,draw);
}
void ToggleResident() {
    BufferedDraw::Suspend();::WebGPU::SetResident(!::WebGPU::ResidentEnabled());
    DbgLogPublic(::WebGPU::ResidentEnabled()?"BMD: mallas residentes":"BMD: mallas armadas en CPU");
}
void Release() { ::WebGPU::ReleaseMeshes();MeshTopology::Release(); }
bool Draw(const Input& input) {
    if(!::WebGPU::ResidentEnabled()) return false;
    if(input.faceCount<=0 || input.positionCount<=0 || !input.faces || !input.positions) return false;
    const bool lit=input.mode==2 && input.light;
    const bool chrome=input.mode==4,texture=input.mode==2;
    if(chrome) return false;
    if(lit && (input.normalCount<=0 || !input.colors)) return false;
    if(texture && (input.uvCount<=0 || !input.uv)) return false;
    MeshTopology::Mesh* found=MeshTopology::Get(input);if(!found) return false;
    const auto& mesh=*found;
    if(mesh.minimum.position<0 || mesh.maximum.position>=input.positionCount ||
       (lit && (mesh.minimum.normal<0 || mesh.maximum.normal>=input.normalCount)) ||
       (texture && (mesh.minimum.uv<0 || mesh.maximum.uv>=input.uvCount))) return false;
    if(!MeshTopology::Resident(*found,input)) return false;
    const auto captured=input.attributes?*input.attributes:BufferedDraw::CaptureAttributes(!lit,!texture);
    float color[4],uv[4];std::copy(captured.color,captured.color+4,color);std::copy(captured.uv,captured.uv+4,uv);
    if(lit) color[3]=input.alpha<1.f?input.alpha:1.f;
    const bool offset=texture && input.offset;
    const Skinning::ResidentDraw draw{input.model,input.key,0,mesh.indices.size(),mesh.maxBone,
        input.positions,input.positionCount,input.colors,input.normalCount,lit,texture,input.bodyLight,color,uv,
        offset?input.u:0.f,offset?input.v:0.f};
    if(!::WebGPU::DrawResidentMesh(input.key,*found,draw)) return false;
    // Mismo contrato que la ruta CPU: el color y la UV actuales quedan en los
    // de la última esquina emitida.
    const auto& c=mesh.corners[mesh.indices.back()];
    float lastColor[4],lastUV[4];std::copy(color,color+4,lastColor);std::copy(uv,uv+4,lastUV);
    if(lit) std::memcpy(lastColor,input.colors+c.normal*3,sizeof(float)*3);
    if(texture) {
        lastUV[0]=input.uv[c.uv*2]+(offset?input.u:0.f);lastUV[1]=input.uv[c.uv*2+1]+(offset?input.v:0.f);
        lastUV[2]=0;lastUV[3]=1;
    }
    SetCurrent(lastColor,lastUV);
    return true;
}
}
