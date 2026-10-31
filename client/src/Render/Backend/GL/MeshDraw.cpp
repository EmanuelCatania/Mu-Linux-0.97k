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
#include "Render/MeshDraw.h"
#include "Render/Skinning.h"
#include "Physics/ClothTypes.h"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <vector>

extern "C" void DbgLogPublic(const char*);
namespace MeshDraw {
namespace {
struct Corner { int position,normal,uv; };
struct Mesh {
    GLuint array=0,buffer=0,elements=0;
    // Malla residente: vértices fijos del BMD subidos una vez (Skinning::StaticVertex).
    GLuint residentArray=0,residentBuffer=0;
    const unsigned char* residentVertices=nullptr;
    int maxBone=-1;bool residentTried=false;
    std::vector<unsigned char> faces;
    // Origen y muestra de las caras ya verificadas: si coinciden, el modelo
    // no cambió y se evita comparar todas las caras en cada envío.
    const unsigned char* facesSource=nullptr;std::uint32_t facesSample=0;
    std::vector<Corner> corners;
    std::vector<unsigned> indices;
    Corner minimum{},maximum{};
    std::vector<int> positions,normals;
};
std::unordered_map<const void*,Mesh> meshes;
bool residentEnabled=true;
std::vector<BufferedDraw::Vertex> vertices;
short ReadShort(const unsigned char* p) { short value;std::memcpy(&value,p,sizeof(value));return value; }
void Delete(Mesh& m) {
    if(m.array) glDeleteVertexArrays(1,&m.array);
    if(m.buffer) glDeleteBuffers(1,&m.buffer);
    if(m.elements) glDeleteBuffers(1,&m.elements);
    if(m.residentArray) glDeleteVertexArrays(1,&m.residentArray);
    if(m.residentBuffer) glDeleteBuffers(1,&m.residentBuffer);
    m.array=m.buffer=m.elements=m.residentArray=m.residentBuffer=0;
    m.residentVertices=nullptr;m.maxBone=-1;m.residentTried=false;
}
// DESVIACION: los datos fijos de cada esquina (posición y normal en el
// espacio del hueso, hueso y UV) se suben una vez por malla. Por cuadro sólo
// viaja la paleta de huesos; el color y el desplazamiento de UV son uniformes.
bool Resident(Mesh& m,const Input& input) {
    if(m.residentArray && m.residentVertices==input.rawVertices) return true;
    if(m.residentTried && m.residentVertices==input.rawVertices) return false;
    if(m.residentArray) { glDeleteVertexArrays(1,&m.residentArray);glDeleteBuffers(1,&m.residentBuffer);m.residentArray=m.residentBuffer=0; }
    m.residentTried=true;m.residentVertices=input.rawVertices;m.maxBone=-1;
    if(!input.rawVertices) return false;
    std::vector<Skinning::StaticVertex> data(m.corners.size());
    for(std::size_t i=0;i<m.corners.size();++i) {
        const auto& c=m.corners[i];auto& v=data[i];
        short bone;std::memcpy(&bone,input.rawVertices+c.position*16,2);
        if(bone<0) return false;
        std::memcpy(v.position,input.rawVertices+c.position*16+4,sizeof(v.position));v.bone=float(bone);
        m.maxBone=std::max(m.maxBone,int(bone));
        v.normal[0]=v.normal[1]=v.normal[2]=v.normal[3]=0;
        if(input.rawNormals && c.normal>=0 && c.normal<input.normalCount) {
            short normalBone;std::memcpy(&normalBone,input.rawNormals+c.normal*20,2);
            if(normalBone<0) return false;
            std::memcpy(v.normal,input.rawNormals+c.normal*20+4,3*sizeof(float));v.normal[3]=float(normalBone);
            m.maxBone=std::max(m.maxBone,int(normalBone));
        }
        v.uv[0]=v.uv[1]=0;
        if(input.uv && c.uv>=0 && c.uv<input.uvCount) { v.uv[0]=input.uv[c.uv*2];v.uv[1]=input.uv[c.uv*2+1]; }
    }
    BufferedDraw::Suspend();
    GLint array,buffer;glGetIntegerv(GL_VERTEX_ARRAY_BINDING,&array);glGetIntegerv(GL_ARRAY_BUFFER_BINDING,&buffer);
    glGenVertexArrays(1,&m.residentArray);glGenBuffers(1,&m.residentBuffer);
    glBindVertexArray(m.residentArray);glBindBuffer(GL_ARRAY_BUFFER,m.residentBuffer);
    glBufferData(GL_ARRAY_BUFFER,data.size()*sizeof(Skinning::StaticVertex),data.data(),GL_STATIC_DRAW);
    Skinning::ResidentLayout(m.residentBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m.elements);
    glBindVertexArray(array);glBindBuffer(GL_ARRAY_BUFFER,buffer);
    if(!m.residentArray || !m.residentBuffer) return false;
    return true;
}
std::uint32_t FacesSample(const unsigned char* faces,std::size_t size) {
    // FNV-1a de la primera y la última cara: barato y suficiente para
    // reconocer un modelo recargado en la misma dirección.
    std::uint32_t hash=2166136261u;
    auto mix=[&](const unsigned char* p,std::size_t n){ for(std::size_t i=0;i<n;++i){hash^=p[i];hash*=16777619u;} };
    mix(faces,std::min<std::size_t>(size,36));if(size>36) mix(faces+size-36,36);
    return hash;
}
bool Topology(Mesh& m,const Input& input) {
    const std::size_t size=std::size_t(input.faceCount)*36;
    const std::uint32_t sample=FacesSample(input.faces,size);
    if(m.facesSource==input.faces && m.faces.size()==size && m.facesSample==sample) return true;
    if(m.faces.size()==size && !std::memcmp(m.faces.data(),input.faces,size)) {
        m.facesSource=input.faces;m.facesSample=sample;return true;
    }
    Delete(m);m.corners.clear();m.indices.clear();m.faces.clear();
    std::unordered_map<std::uint64_t,unsigned> unique;
    for(int f=0;f<input.faceCount;++f) {
        const auto* face=input.faces+f*36;const int n=static_cast<signed char>(*face);
        if(n>4) return false;
        for(int v=0;v<n;++v) {
            const Corner c{ReadShort(face+2+v*2),ReadShort(face+10+v*2),ReadShort(face+18+v*2)};
            const auto key=std::uint64_t(std::uint16_t(c.position)) |
                (std::uint64_t(std::uint16_t(c.normal))<<16) | (std::uint64_t(std::uint16_t(c.uv))<<32);
            const auto entry=unique.emplace(key,static_cast<unsigned>(m.corners.size()));
            if(entry.second) m.corners.push_back(c);
            m.indices.push_back(entry.first->second);
        }
    }
    if(m.corners.empty()) return false;
    m.positions.clear();m.normals.clear();
    for(const auto& c:m.corners) { m.positions.push_back(c.position);m.normals.push_back(c.normal); }
    m.minimum=m.maximum=m.corners.front();
    for(const auto& c:m.corners) {
        m.minimum.position=std::min(m.minimum.position,c.position);m.maximum.position=std::max(m.maximum.position,c.position);
        m.minimum.normal=std::min(m.minimum.normal,c.normal);m.maximum.normal=std::max(m.maximum.normal,c.normal);
        m.minimum.uv=std::min(m.minimum.uv,c.uv);m.maximum.uv=std::max(m.maximum.uv,c.uv);
    }
    BufferedDraw::Suspend();
    GLint array,buffer,client;glGetIntegerv(GL_VERTEX_ARRAY_BINDING,&array);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING,&buffer);glGetIntegerv(GL_CLIENT_ACTIVE_TEXTURE,&client);
    glGenVertexArrays(1,&m.array);glGenBuffers(1,&m.buffer);glGenBuffers(1,&m.elements);
    glBindVertexArray(m.array);glBindBuffer(GL_ARRAY_BUFFER,m.buffer);
    using Vertex=BufferedDraw::Vertex;
    glEnableVertexAttribArray(0);glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,position));
    glEnableVertexAttribArray(1);glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,color));
    glEnableVertexAttribArray(2);glVertexAttribPointer(2,4,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,uv));
    glEnableClientState(GL_VERTEX_ARRAY);glVertexPointer(3,GL_FLOAT,sizeof(Vertex),(void*)offsetof(Vertex,position));
    glEnableClientState(GL_COLOR_ARRAY);glColorPointer(4,GL_FLOAT,sizeof(Vertex),(void*)offsetof(Vertex,color));
    glClientActiveTexture(GL_TEXTURE0);glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(4,GL_FLOAT,sizeof(Vertex),(void*)offsetof(Vertex,uv));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m.elements);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,m.indices.size()*sizeof(unsigned),m.indices.data(),GL_STATIC_DRAW);
    glClientActiveTexture(client);glBindVertexArray(array);glBindBuffer(GL_ARRAY_BUFFER,buffer);
    if(!m.array || !m.buffer || !m.elements) { Delete(m);return false; }
    m.faces.assign(input.faces,input.faces+size);m.facesSource=input.faces;m.facesSample=sample;return true;
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
    auto& mesh=meshes[input.key];if(!Topology(mesh,input)) return false;
    if(mesh.minimum.position<0 || mesh.maximum.position>=input.positionCount) return false;
    if(!Resident(mesh,input)) return false;
    // La ruta CPU emite cada vértice con el color y la UV actuales.
    const auto current=BufferedDraw::CaptureAttributes(true,true);
    static const float noLight[3]{};
    Skinning::ResidentDraw draw{model,input.key,mesh.residentArray,mesh.indices.size(),mesh.maxBone,
        world,input.positionCount,nullptr,input.normalCount,false,false,noLight,current.color,current.uv,0.f,0.f};
    draw.shadow=true;draw.shadowOrigin=origin;
    if(!Skinning::DrawResident(draw)) return false;
    glColor4fv(current.color);glTexCoord4fv(current.uv);
    return true;
}
void ToggleResident() {
    BufferedDraw::Suspend();residentEnabled=!residentEnabled;
    DbgLogPublic(residentEnabled?"BMD: mallas residentes":"BMD: mallas armadas en CPU");
}
void Release() {
    BufferedDraw::Suspend();
    for(auto& entry:meshes) Delete(entry.second);
    meshes.clear();vertices.clear();Skinning::Release();
}
bool Draw(const Input& input) {
    if(input.faceCount<=0 || input.positionCount<=0 || !input.faces || !input.positions) return false;
    const bool lit=input.mode==2 && input.light;
    const bool chrome=input.mode==4,texture=input.mode==2;
    if((lit || chrome) && (input.normalCount<=0 || !(chrome?input.chromeUV:input.colors))) return false;
    if(texture && (input.uvCount<=0 || !input.uv)) return false;
    auto& mesh=meshes[input.key];if(!Topology(mesh,input)) return false;
    if(mesh.minimum.position<0 || mesh.maximum.position>=input.positionCount ||
       ((lit || chrome) && (mesh.minimum.normal<0 || mesh.maximum.normal>=input.normalCount)) ||
       (texture && (mesh.minimum.uv<0 || mesh.maximum.uv>=input.uvCount))) return false;
    // DESVIACION: BMD 0x00440D50 proporciona todos los atributos en las
    // pasadas iluminadas/texturadas y chrome. Consultar sólo lo heredado.
    const auto captured=input.attributes?*input.attributes:BufferedDraw::CaptureAttributes(!lit&&!chrome,!texture&&!chrome);
    float color[4],uv[4];std::copy(captured.color,captured.color+4,color);std::copy(captured.uv,captured.uv+4,uv);
    if(chrome) { std::copy(input.bodyLight,input.bodyLight+3,color);color[3]=input.alpha<1.f?input.alpha:1.f; }
    else if(lit) color[3]=input.alpha<1.f?input.alpha:1.f;
    if(residentEnabled && !chrome && Resident(mesh,input)) {
        const bool offset=texture && input.offset;
        const Skinning::ResidentDraw draw{input.model,input.key,mesh.residentArray,mesh.indices.size(),mesh.maxBone,
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
            glColor4fv(lastColor);glTexCoord4fv(lastUV);return true;
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
    if(Skinning::Draw(input.model,input.key,mesh.elements,vertices.data(),mesh.positions.data(),mesh.normals.data(),
        vertices.size(),mesh.indices.size(),input.positions,input.positionCount,input.colors,lit,input.bodyLight)) {
        const auto& last=vertices[mesh.indices.back()];glColor4fv(last.color);glTexCoord4fv(last.uv);return true;
    }
    // Conservar el color y UV de la última esquina, incluso si no completa
    // un triángulo. DrawElements no define esos atributos tras el envío.
    const auto& last=vertices[mesh.indices.back()];
    // DESVIACION: DrawIndexed envía lo pendiente antes de cambiar de VAO.
    // Mantener el programa activo hasta la siguiente frontera de compatibilidad.
    glColor4fv(last.color);glTexCoord4fv(last.uv);
    BufferedDraw::DrawIndexed(mesh.array,mesh.buffer,vertices.data(),vertices.size(),mesh.indices.size(),&last);
    return true;
}
}
