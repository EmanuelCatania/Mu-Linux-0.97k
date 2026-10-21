// Sin PCH: glad debe preceder a cualquier header GL del sistema.
#include "Platform/GL/glad.h"
#include "Render/BufferedDraw.h"
#include "Render/MeshDraw.h"
#include "Render/Skinning.h"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <vector>

namespace MeshDraw {
namespace {
struct Corner { int position,normal,uv; };
struct Mesh {
    GLuint array=0,buffer=0,elements=0;
    std::vector<unsigned char> faces;
    std::vector<Corner> corners;
    std::vector<unsigned> indices;
    Corner minimum{},maximum{};
    std::vector<int> positions,normals;
};
std::unordered_map<const void*,Mesh> meshes;
std::vector<BufferedDraw::Vertex> vertices;
short ReadShort(const unsigned char* p) { short value;std::memcpy(&value,p,sizeof(value));return value; }
void Delete(Mesh& m) {
    if(m.array) glDeleteVertexArrays(1,&m.array);
    if(m.buffer) glDeleteBuffers(1,&m.buffer);
    if(m.elements) glDeleteBuffers(1,&m.elements);
    m.array=m.buffer=m.elements=0;
}
bool Topology(Mesh& m,const Input& input) {
    const std::size_t size=std::size_t(input.faceCount)*36;
    if(m.faces.size()==size && !std::memcmp(m.faces.data(),input.faces,size)) return true;
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
    glEnableClientState(GL_VERTEX_ARRAY);glVertexPointer(3,GL_FLOAT,sizeof(Vertex),(void*)offsetof(Vertex,position));
    glEnableClientState(GL_COLOR_ARRAY);glColorPointer(4,GL_FLOAT,sizeof(Vertex),(void*)offsetof(Vertex,color));
    glClientActiveTexture(GL_TEXTURE0);glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(4,GL_FLOAT,sizeof(Vertex),(void*)offsetof(Vertex,uv));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m.elements);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,m.indices.size()*sizeof(unsigned),m.indices.data(),GL_STATIC_DRAW);
    glClientActiveTexture(client);glBindVertexArray(array);glBindBuffer(GL_ARRAY_BUFFER,buffer);
    if(!m.array || !m.buffer || !m.elements) { Delete(m);return false; }
    m.faces.assign(input.faces,input.faces+size);return true;
}
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
    float color[4],uv[4];glGetFloatv(GL_CURRENT_COLOR,color);
    GLint active;glGetIntegerv(GL_ACTIVE_TEXTURE,&active);glActiveTexture(GL_TEXTURE0);
    glGetFloatv(GL_CURRENT_TEXTURE_COORDS,uv);glActiveTexture(active);
    if(chrome) { std::copy(input.bodyLight,input.bodyLight+3,color);color[3]=input.alpha<1.f?input.alpha:1.f; }
    else if(lit) color[3]=input.alpha<1.f?input.alpha:1.f;
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
    BufferedDraw::DrawIndexed(mesh.array,mesh.buffer,vertices.data(),vertices.size(),mesh.indices.size());
    return true;
}
}
