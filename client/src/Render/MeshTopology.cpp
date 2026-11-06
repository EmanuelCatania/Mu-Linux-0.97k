// Topología de las mallas BMD, común a los backends (ver MeshTopology.h).
#include "Render/MeshTopology.h"
#include <algorithm>
#include <cstring>
#include <unordered_map>

namespace MeshTopology {
namespace {
std::unordered_map<const void*,Mesh> meshes;
std::uint64_t nextRevision=1;
short ReadShort(const unsigned char* p) { short value;std::memcpy(&value,p,sizeof(value));return value; }
std::uint32_t FacesSample(const unsigned char* faces,std::size_t size) {
    // FNV-1a de la primera y la última cara: barato y suficiente para
    // reconocer un modelo recargado en la misma dirección.
    std::uint32_t hash=2166136261u;
    auto mix=[&](const unsigned char* p,std::size_t n){ for(std::size_t i=0;i<n;++i){hash^=p[i];hash*=16777619u;} };
    mix(faces,std::min<std::size_t>(size,36));if(size>36) mix(faces+size-36,36);
    return hash;
}
void Clear(Mesh& m) {
    m.corners.clear();m.indices.clear();m.faces.clear();m.positions.clear();m.normals.clear();
    m.facesSource=nullptr;m.facesSample=0;
    m.residentVertices=nullptr;m.residentTried=false;m.maxBone=-1;m.resident.clear();
}
}
Mesh* Get(const MeshDraw::Input& input) {
    auto& m=meshes[input.key];
    const std::size_t size=std::size_t(input.faceCount)*36;
    const std::uint32_t sample=FacesSample(input.faces,size);
    if(m.facesSource==input.faces && m.faces.size()==size && m.facesSample==sample) return &m;
    if(m.faces.size()==size && !std::memcmp(m.faces.data(),input.faces,size)) {
        m.facesSource=input.faces;m.facesSample=sample;return &m;
    }
    Clear(m);m.revision=nextRevision++;
    std::unordered_map<std::uint64_t,unsigned> unique;
    for(int f=0;f<input.faceCount;++f) {
        const auto* face=input.faces+f*36;const int n=static_cast<signed char>(*face);
        if(n>4) { Clear(m);return nullptr; }
        for(int v=0;v<n;++v) {
            const Corner c{ReadShort(face+2+v*2),ReadShort(face+10+v*2),ReadShort(face+18+v*2)};
            const auto key=std::uint64_t(std::uint16_t(c.position)) |
                (std::uint64_t(std::uint16_t(c.normal))<<16) | (std::uint64_t(std::uint16_t(c.uv))<<32);
            const auto entry=unique.emplace(key,static_cast<unsigned>(m.corners.size()));
            if(entry.second) m.corners.push_back(c);
            m.indices.push_back(entry.first->second);
        }
    }
    if(m.corners.empty()) { Clear(m);return nullptr; }
    for(const auto& c:m.corners) { m.positions.push_back(c.position);m.normals.push_back(c.normal); }
    m.minimum=m.maximum=m.corners.front();
    for(const auto& c:m.corners) {
        m.minimum.position=std::min(m.minimum.position,c.position);m.maximum.position=std::max(m.maximum.position,c.position);
        m.minimum.normal=std::min(m.minimum.normal,c.normal);m.maximum.normal=std::max(m.maximum.normal,c.normal);
        m.minimum.uv=std::min(m.minimum.uv,c.uv);m.maximum.uv=std::max(m.maximum.uv,c.uv);
    }
    m.faces.assign(input.faces,input.faces+size);m.facesSource=input.faces;m.facesSample=sample;
    return &m;
}
// DESVIACION: los datos fijos de cada esquina (posición y normal en el
// espacio del hueso, hueso y UV) se suben una vez por malla. Por cuadro sólo
// viaja la paleta de huesos; el color y el desplazamiento de UV son uniformes.
bool Resident(Mesh& m,const MeshDraw::Input& input) {
    if(!m.resident.empty() && m.residentVertices==input.rawVertices) return true;
    if(m.residentTried && m.residentVertices==input.rawVertices) return false;
    m.resident.clear();m.residentTried=true;m.residentVertices=input.rawVertices;m.maxBone=-1;
    if(!input.rawVertices) return false;
    std::vector<Skinning::StaticVertex> data(m.corners.size());
    int maxBone=-1;
    for(std::size_t i=0;i<m.corners.size();++i) {
        const auto& c=m.corners[i];auto& v=data[i];
        short bone;std::memcpy(&bone,input.rawVertices+c.position*16,2);
        if(bone<0) return false;
        std::memcpy(v.position,input.rawVertices+c.position*16+4,sizeof(v.position));v.bone=float(bone);
        maxBone=std::max(maxBone,int(bone));
        v.normal[0]=v.normal[1]=v.normal[2]=v.normal[3]=0;
        if(input.rawNormals && c.normal>=0 && c.normal<input.normalCount) {
            short normalBone;std::memcpy(&normalBone,input.rawNormals+c.normal*20,2);
            if(normalBone<0) return false;
            std::memcpy(v.normal,input.rawNormals+c.normal*20+4,3*sizeof(float));v.normal[3]=float(normalBone);
            maxBone=std::max(maxBone,int(normalBone));
        }
        v.uv[0]=v.uv[1]=0;
        if(input.uv && c.uv>=0 && c.uv<input.uvCount) { v.uv[0]=input.uv[c.uv*2];v.uv[1]=input.uv[c.uv*2+1]; }
    }
    m.resident=std::move(data);m.maxBone=maxBone;m.residentRevision=nextRevision++;
    return true;
}
void Release() { meshes.clear(); }
}
