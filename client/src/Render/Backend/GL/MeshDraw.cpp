// Sin PCH: glad debe preceder a cualquier header GL del sistema.
#include "Platform/GL/glad.h"
#include "Render/Backend/GL/ProfileUploads.h"
#undef glTexImage2D
#define glTexImage2D(...) ::ProfileUploadGL::Image2D(glad_glTexImage2D,__VA_ARGS__)
#undef glTexSubImage2D
#define glTexSubImage2D(...) ::ProfileUploadGL::Sub2D(glad_glTexSubImage2D,__VA_ARGS__)
#undef glTexImage3D
#define glTexImage3D(...) ::ProfileUploadGL::Image3D(glad_glTexImage3D,__VA_ARGS__)
#undef glTexSubImage3D
#define glTexSubImage3D(...) ::ProfileUploadGL::Sub3D(glad_glTexSubImage3D,__VA_ARGS__)
#undef glBufferData
#define glBufferData(...) ::ProfileUploadGL::BufferData(glad_glBufferData,__VA_ARGS__)
#undef glBufferSubData
#define glBufferSubData(...) ::ProfileUploadGL::BufferSubData(glad_glBufferSubData,__VA_ARGS__)

#include "Render/BufferedDraw.h"
#include "Render/Backend/GL/ShadowMap.h"
#include "Render/MeshDraw.h"
#include "Render/Backend/Api.h"
#include "Render/Skinning.h"
#include "Render/MeshTopology.h"
#include "Render/RenderState.h"
#include "Physics/ClothTypes.h"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <vector>

extern "C" void DbgLogPublic(const char*);
namespace MeshDraw::GL {
namespace {
// Buffers GL de cada malla. La topología y los vértices residentes son comunes
// (MeshTopology); acá se rehacen cuando cambia su revisión.
struct Mesh {
    GLuint array=0,buffer=0,elements=0;std::uint64_t revision=0;
    GLuint residentArray=0,residentBuffer=0;std::uint64_t residentRevision=0;
};
std::unordered_map<const void*,Mesh> meshes;
bool residentEnabled=true;
std::vector<BufferedDraw::Vertex> vertices;
// Color y UV actuales tras el envío: los de la última esquina, como la ruta CPU.
void SetCurrent(const float* color,const float* uv) {
    auto& s=RenderState::Mutable();std::memcpy(s.color,color,sizeof(s.color));std::memcpy(s.uv,uv,sizeof(s.uv));
}
void DeleteResident(Mesh& m) {
    if(m.residentArray) glDeleteVertexArrays(1,&m.residentArray);
    if(m.residentBuffer) glDeleteBuffers(1,&m.residentBuffer);
    m.residentArray=m.residentBuffer=0;m.residentRevision=0;
}
void Delete(Mesh& m) {
    if(m.array) glDeleteVertexArrays(1,&m.array);
    if(m.buffer) glDeleteBuffers(1,&m.buffer);
    if(m.elements) glDeleteBuffers(1,&m.elements);
    m.array=m.buffer=m.elements=0;m.revision=0;
    DeleteResident(m);
}
bool Resident(Mesh& m,MeshTopology::Mesh& t,const Input& input) {
    if(!MeshTopology::Resident(t,input)) return false;
    if(m.residentArray && m.residentRevision==t.residentRevision) return true;
    DeleteResident(m);
    BufferedDraw::Suspend();
    GLint array,buffer;glGetIntegerv(GL_VERTEX_ARRAY_BINDING,&array);glGetIntegerv(GL_ARRAY_BUFFER_BINDING,&buffer);
    glGenVertexArrays(1,&m.residentArray);glGenBuffers(1,&m.residentBuffer);
    glBindVertexArray(m.residentArray);glBindBuffer(GL_ARRAY_BUFFER,m.residentBuffer);
    glBufferData(GL_ARRAY_BUFFER,t.resident.size()*sizeof(Skinning::StaticVertex),t.resident.data(),GL_STATIC_DRAW);
    Skinning::ResidentLayout(m.residentBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m.elements);
    glBindVertexArray(array);glBindBuffer(GL_ARRAY_BUFFER,buffer);
    if(!m.residentArray || !m.residentBuffer) { DeleteResident(m);return false; }
    m.residentRevision=t.residentRevision;
    return true;
}
// Topología común y los buffers GL de la malla, rehechos si cambió.
MeshTopology::Mesh* Topology(Mesh& m,const Input& input) {
    MeshTopology::Mesh* t=MeshTopology::Get(input);
    if(!t) { Delete(m);return nullptr; }
    if(m.array && m.revision==t->revision) return t;
    Delete(m);
    BufferedDraw::Suspend();
    GLint array,buffer;glGetIntegerv(GL_VERTEX_ARRAY_BINDING,&array);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING,&buffer);
    glGenVertexArrays(1,&m.array);glGenBuffers(1,&m.buffer);glGenBuffers(1,&m.elements);
    glBindVertexArray(m.array);glBindBuffer(GL_ARRAY_BUFFER,m.buffer);
    using Vertex=BufferedDraw::Vertex;
    glEnableVertexAttribArray(0);glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,position));
    glEnableVertexAttribArray(1);glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,color));
    glEnableVertexAttribArray(2);glVertexAttribPointer(2,4,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,uv));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m.elements);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,t->indices.size()*sizeof(unsigned),t->indices.data(),GL_STATIC_DRAW);
    glBindVertexArray(array);glBindBuffer(GL_ARRAY_BUFFER,buffer);
    if(!m.array || !m.buffer || !m.elements) { Delete(m);return nullptr; }
    m.revision=t->revision;
    return t;
}
}
bool DrawShadow(const void* model,const char* meshRecord,const float* world,const float* origin) {
    if(!residentEnabled || !model || !meshRecord) return false;
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
    auto& gl=meshes[input.key];MeshTopology::Mesh* mesh=Topology(gl,input);if(!mesh) return false;
    if(mesh->minimum.position<0 || mesh->maximum.position>=input.positionCount) return false;
    if(!Resident(gl,*mesh,input)) return false;
    // La ruta CPU emite cada vértice con el color y la UV actuales.
    const auto current=BufferedDraw::CaptureAttributes(true,true);
    static const float noLight[3]{};
    Skinning::ResidentDraw draw{model,input.key,gl.residentArray,mesh->indices.size(),mesh->maxBone,
        world,input.positionCount,nullptr,input.normalCount,false,false,noLight,current.color,current.uv,0.f,0.f};
    draw.shadow=true;draw.shadowOrigin=origin;
    return Skinning::DrawResident(draw);
}
void ToggleResident() {
    BufferedDraw::Suspend();residentEnabled=!residentEnabled;
    DbgLogPublic(residentEnabled?"BMD: mallas residentes":"BMD: mallas armadas en CPU");
}
void Release() {
    BufferedDraw::Suspend();
    for(auto& entry:meshes) Delete(entry.second);
    meshes.clear();vertices.clear();MeshTopology::Release();Skinning::Release();
}
bool Draw(const Input& input) {
    if(input.faceCount<=0 || input.positionCount<=0 || !input.faces || !input.positions) return false;
    const bool lit=input.mode==2 && input.light;
    const bool chrome=input.mode==4,texture=input.mode==2;
    if((lit || chrome) && (input.normalCount<=0 || !(chrome?input.chromeUV:input.colors))) return false;
    if(texture && (input.uvCount<=0 || !input.uv)) return false;
    auto& gl=meshes[input.key];MeshTopology::Mesh* found=Topology(gl,input);if(!found) return false;
    const auto& mesh=*found;
    if(mesh.minimum.position<0 || mesh.maximum.position>=input.positionCount ||
       ((lit || chrome) && (mesh.minimum.normal<0 || mesh.maximum.normal>=input.normalCount)) ||
       (texture && (mesh.minimum.uv<0 || mesh.maximum.uv>=input.uvCount))) return false;
    // DESVIACION: BMD 0x00440D50 proporciona todos los atributos en las
    // pasadas iluminadas/texturadas y chrome. Consultar sólo lo heredado.
    const auto captured=input.attributes?*input.attributes:BufferedDraw::CaptureAttributes(!lit&&!chrome,!texture&&!chrome);
    float color[4],uv[4];std::copy(captured.color,captured.color+4,color);std::copy(captured.uv,captured.uv+4,uv);
    if(chrome) { std::copy(input.bodyLight,input.bodyLight+3,color);color[3]=input.alpha<1.f?input.alpha:1.f; }
    else if(lit) color[3]=input.alpha<1.f?input.alpha:1.f;
    // DESVIACION: sombras proyectadas (opción). La malla con sus posiciones
    // del mundo ya calculadas en CPU; lo semitransparente no proyecta.
    if(!chrome && color[3]>=0.99f && ShadowMapGL::Capturing()) {
        static std::vector<ShadowMapGL::CasterVertex> triangles;
        triangles.resize(mesh.indices.size());
        const bool offset=texture && input.offset;
        for(std::size_t i=0;i<mesh.indices.size();++i) {
            const auto& c=mesh.corners[mesh.indices[i]];auto& t=triangles[i];
            std::memcpy(t.position,input.positions+c.position*3,sizeof(t.position));
            t.uv[0]=texture?input.uv[c.uv*2]+(offset?input.u:0.f):0.f;
            t.uv[1]=texture?input.uv[c.uv*2+1]+(offset?input.v:0.f):0.f;
        }
        ShadowMapGL::Capture(triangles.data(),triangles.size());
    }
    if(residentEnabled && !chrome && Resident(gl,*found,input)) {
        const bool offset=texture && input.offset;
        const Skinning::ResidentDraw draw{input.model,input.key,gl.residentArray,mesh.indices.size(),mesh.maxBone,
            input.positions,input.positionCount,input.colors,input.normalCount,lit,texture,input.bodyLight,color,uv,
            offset?input.u:0.f,offset?input.v:0.f};
        if(Skinning::DrawResident(draw)) {
            // Mismo contrato que la ruta CPU: el color y la UV actuales quedan
            // en los de la última esquina emitida.
            const auto& c=mesh.corners[mesh.indices.back()];
            float lastColor[4],lastUV[4];std::copy(color,color+4,lastColor);std::copy(uv,uv+4,lastUV);
            if(lit) std::memcpy(lastColor,input.colors+c.normal*3,sizeof(float)*3);
            if(texture) {
                lastUV[0]=input.uv[c.uv*2]+(offset?input.u:0.f);lastUV[1]=input.uv[c.uv*2+1]+(offset?input.v:0.f);
                lastUV[2]=0;lastUV[3]=1;
            }
            SetCurrent(lastColor,lastUV);return true;
        }
    }
    vertices.resize(mesh.corners.size());
    for(std::size_t i=0;i<mesh.corners.size();++i) {
        const auto& c=mesh.corners[i];auto& vertex=vertices[i];
        std::memcpy(vertex.position,input.positions+c.position*3,sizeof(vertex.position));
        std::memcpy(vertex.color,color,sizeof(color));std::memcpy(vertex.uv,uv,sizeof(uv));
        if(lit) std::memcpy(vertex.color,input.colors+c.normal*3,sizeof(float)*3);
        if(texture || chrome) {
            const auto* t=chrome?input.chromeUV+c.normal*2:input.uv+c.uv*2;
            vertex.uv[0]=t[0];vertex.uv[1]=t[1];
            if(texture && input.offset) { vertex.uv[0]+=input.u;vertex.uv[1]+=input.v; }
            vertex.uv[2]=0;vertex.uv[3]=1;
        }
    }
    // DESVIACION: skinning rígido BMD por hueso en el shader, con respaldo
    // para datos CPU deformados o una pose que no corresponde al dibujo.
    if(Skinning::Draw(input.model,input.key,gl.elements,vertices.data(),mesh.positions.data(),mesh.normals.data(),
        vertices.size(),mesh.indices.size(),input.positions,input.positionCount,input.colors,lit,input.bodyLight)) {
        const auto& last=vertices[mesh.indices.back()];SetCurrent(last.color,last.uv);return true;
    }
    // Conservar el color y UV de la última esquina, incluso si no completa
    // un triángulo. DrawElements no define esos atributos tras el envío.
    const auto& last=vertices[mesh.indices.back()];
    // DESVIACION: DrawIndexed envía lo pendiente antes de cambiar de VAO.
    SetCurrent(last.color,last.uv);
    BufferedDraw::DrawIndexed(gl.array,gl.buffer,vertices.data(),vertices.size(),mesh.indices.size(),&last);
    return true;
}
}
