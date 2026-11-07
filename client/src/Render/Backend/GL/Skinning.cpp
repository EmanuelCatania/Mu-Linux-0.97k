// Sin PCH: resolver GL moderno mediante glad.
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

#include "Render/Skinning.h"
#include "Render/Backend/Api.h"
#include "Render/Backend/GL/FragmentPipeline.h"
#include "Render/RenderProfile.h"
#include "Render/Backend/GL/ProfileQueries.h"
#include <algorithm>
#include <array>
#include <map>
#include <cstring>

#include <vector>
extern "C" void DbgLogPublic(const char*);
namespace Skinning::GL {
namespace {
// La pose (paleta, escalas, luz) es común a los backends: Skinning::Pose.
using Pose::BoneOf;using Pose::XyzOf;using Pose::VertexStride;using Pose::NormalStride;
unsigned long long uploaded=0;
std::size_t paletteCapacity=0,paletteUsed=0;
GLint paletteBase=0,paletteBaseLocation=0;
GLuint program=0,array=0,buffer=0,boneBuffer=0,boneTexture=0;
bool failed=false,layoutReady=false,paletteBound=false,uniformsReady=false;
GLint savedTextureBuffer=0,savedTexture=0,lastLight=-1;
float lastScales[2]{},lastOrigin[3]{},lastDirection[3]{},lastCoefficients[3]{},lastBodyLight[3]{},lastOffset[2]{};
void RestorePalette() {
    GLint active;glGetIntegerv(GL_ACTIVE_TEXTURE,&active);
    glActiveTexture(GL_TEXTURE1);RenderProfileGL::ObserveTexture(GL_TEXTURE_BUFFER,savedTexture);glBindTexture(GL_TEXTURE_BUFFER,savedTexture);
    glBindBuffer(GL_TEXTURE_BUFFER,savedTextureBuffer);glActiveTexture(active);paletteBound=false;
}
void SetVector(GLint location,const float* value,float* previous) {
    if(!uniformsReady || std::memcmp(value,previous,3*sizeof(float))) {
        glUniform3fv(location,1,value);std::copy(value,value+3,previous);
    }
}
GLint paletteLocation,scalesLocation,originLocation,directionLocation,coefficientsLocation,bodyLightLocation,lightLocation,offsetLocation,shadowLocation,shadowOriginLocation;
int lastShadow=-1;float lastShadowOrigin[3]{};
struct Vertex { BufferedDraw::Vertex attributes;float normal[4];float bone; };
std::vector<Vertex> vertices;
// DESVIACION: uvOffset suma el desplazamiento de textura de la malla en el
// shader; la ruta que arma los vértices en CPU ya lo trae aplicado y envía 0.
const char* shaderSource=R"GLSL(layout(location=0) in vec3 aPosition;
layout(location=1) in vec4 aColor;
layout(location=2) in vec4 aUV;
layout(location=6) in vec4 rawNormal;
layout(location=7) in float vertexBone;
uniform samplerBuffer palette;
uniform int paletteBase;
uniform vec2 scales;
uniform vec3 origin;
uniform vec3 direction;
uniform vec3 coefficients;
uniform vec3 bodyLight;
uniform bool useLight;
uniform vec2 uvOffset;
uniform bool shadowMode;
uniform vec3 shadowOrigin;
vec3 rotateBone(vec3 p,int b) {
    int r=paletteBase+b*3;
    return vec3(dot(texelFetch(palette,r).xyz,p),dot(texelFetch(palette,r+1).xyz,p),dot(texelFetch(palette,r+2).xyz,p));
}
void main() {
    int b=paletteBase+int(vertexBone)*3;
    vec3 p=rotateBone(aPosition,int(vertexBone))*scales.x;
    p+=vec3(texelFetch(palette,b).w,texelFetch(palette,b+1).w,texelFetch(palette,b+2).w);
    p=p*scales.y+origin;
    if(shadowMode) {
        vec3 l=p-shadowOrigin;
        p=vec3(l.x+(l.x+2000.0)*l.z/(l.z-4000.0)+shadowOrigin.x,l.y+shadowOrigin.y,shadowOrigin.z+5.0);
    }
    vec4 position=vec4(p,1);
    gl_Position=uModelViewProjection*position;
    vec4 color=aColor;
    if(useLight) {
        vec3 normal=rotateBone(rawNormal.xyz,int(rawNormal.w));
        float light=max((direction.y*normal.y+direction.x*normal.x+direction.z*normal.z)*coefficients.x+coefficients.y,coefficients.z);
        color.rgb=light*bodyLight;
    }
    vColor=clamp(color,0.0,1.0);
    vUV=uTextureMatrix*(aUV+vec4(uvOffset,0,0));
    // El cliente no activa planos de recorte de usuario (issue #240).
    vFogDistance=abs((uModelView*position).z);
}
)GLSL";
GLuint uniformProgram=0;
std::map<GLuint,std::array<GLint,11>> uniformLocations;
void SelectUniforms(GLuint next){
    if(uniformProgram==next)return;uniformProgram=next;uniformsReady=false;lastLight=-1;lastShadow=-1;
    auto found=uniformLocations.find(next);
    if(found==uniformLocations.end()){
        const char* names[]{"paletteBase","palette","scales","origin","direction","coefficients","bodyLight","useLight","uvOffset","shadowMode","shadowOrigin"};
        std::array<GLint,11> locations{};for(int i=0;i<11;++i)locations[i]=glGetUniformLocation(next,names[i]);
        found=uniformLocations.emplace(next,locations).first;
    }
    const auto& p=found->second;paletteBaseLocation=p[0];paletteLocation=p[1];scalesLocation=p[2];originLocation=p[3];directionLocation=p[4];coefficientsLocation=p[5];bodyLightLocation=p[6];lightLocation=p[7];offsetLocation=p[8];shadowLocation=p[9];shadowOriginLocation=p[10];
}
bool Initialize() {
    if(program) return true;
    if(failed) return false;
    program=FragmentPipelineGL::Create(shaderSource);
    if(!program) { failed=true;return false; }
    glGenVertexArrays(1,&array);glGenBuffers(1,&buffer);glGenBuffers(1,&boneBuffer);glGenTextures(1,&boneTexture);
    SelectUniforms(program);
    return true;
}
// Enlaza la paleta y sube la pose actual si todavía no está en la GPU.
bool UploadPalette() {
    if(!paletteBound) {
        GLint active;glGetIntegerv(GL_ACTIVE_TEXTURE,&active);
        glGetIntegerv(GL_TEXTURE_BUFFER,&savedTextureBuffer);
        glActiveTexture(GL_TEXTURE1);glGetIntegerv(GL_TEXTURE_BINDING_BUFFER,&savedTexture);
        RenderProfileGL::ObserveTexture(GL_TEXTURE_BUFFER,boneTexture);glBindTexture(GL_TEXTURE_BUFFER,boneTexture);glBindBuffer(GL_TEXTURE_BUFFER,boneBuffer);
        if(!layoutReady) glTexBuffer(GL_TEXTURE_BUFFER,GL_RGBA32F,boneBuffer);
        glActiveTexture(active);paletteBound=true;BufferedDraw::SetExitHook(RestorePalette);
    }
    if(uploaded!=Pose::Revision()) {
        const auto& palette=Pose::Palette();
        const auto bytes=palette.size()*sizeof(float);
        // DESVIACION: regiones nuevas sin sobrescribir matrices que la GPU
        // aún usa; huérfanar el almacenamiento al agotar el bloque.
        if(bytes>paletteCapacity || paletteUsed+bytes>paletteCapacity) {
            paletteCapacity=std::max(bytes,std::size_t(65536));paletteUsed=0;
            glBufferData(GL_TEXTURE_BUFFER,paletteCapacity,nullptr,GL_STREAM_DRAW);
        }
        void* destination=glMapBufferRange(GL_TEXTURE_BUFFER,paletteUsed,bytes,
            GL_MAP_WRITE_BIT|GL_MAP_UNSYNCHRONIZED_BIT|GL_MAP_INVALIDATE_RANGE_BIT);
        if(!destination) return false;
        std::memcpy(destination,palette.data(),bytes);
        RenderProfile::BufferUpload(bytes); // También contar el tramo escrito por mapeo.
        if(!glUnmapBuffer(GL_TEXTURE_BUFFER)) return false;
        paletteBase=static_cast<GLint>(paletteUsed/(4*sizeof(float)));
        paletteUsed+=bytes;uploaded=Pose::Revision();
    }
    return true;
}
void SetUniforms(bool gpuLight,const float* bodyLight,const float* offset,const float* shadowOrigin=nullptr) {
    const bool shadow=shadowOrigin!=nullptr;
    if(lastShadow!=int(shadow)) { glUniform1i(shadowLocation,shadow?1:0);lastShadow=int(shadow); }
    if(shadow && (!uniformsReady || std::memcmp(lastShadowOrigin,shadowOrigin,sizeof(lastShadowOrigin)))) {
        glUniform3fv(shadowOriginLocation,1,shadowOrigin);std::copy(shadowOrigin,shadowOrigin+3,lastShadowOrigin);
    }
    if(!uniformsReady) glUniform1i(paletteLocation,1);
    glUniform1i(paletteBaseLocation,paletteBase);
    const float scales[2]{Pose::RotationScale(),Pose::BodyScale()};
    if(!uniformsReady || std::memcmp(lastScales,scales,sizeof(scales))) {
        glUniform2f(scalesLocation,scales[0],scales[1]);std::copy(scales,scales+2,lastScales);
    }
    SetVector(originLocation,Pose::Origin(),lastOrigin);SetVector(directionLocation,Pose::Direction(),lastDirection);
    SetVector(coefficientsLocation,Pose::Coefficients(),lastCoefficients);SetVector(bodyLightLocation,bodyLight,lastBodyLight);
    if(lastLight!=int(gpuLight)) { glUniform1i(lightLocation,gpuLight?1:0);lastLight=int(gpuLight); }
    if(!uniformsReady || std::memcmp(lastOffset,offset,sizeof(lastOffset))) {
        glUniform2f(offsetLocation,offset[0],offset[1]);std::copy(offset,offset+2,lastOffset);
    }
    uniformsReady=true;
}
}
bool Draw(const void* model,const void* key,unsigned elements,const BufferedDraw::Vertex* attributes,
    const int* positions,const int* normals,std::size_t count,std::size_t indices,
    const float* world,int positionCount,const float* colors,bool lit,const float* bodyLight) {
    const Pose::Mesh* found=Pose::Find(model,key,world,positionCount);
    if(!found) return false;
    const auto& m=*found;
    const auto& palette=Pose::Palette();
    bool gpuLight=lit;
    vertices.resize(count);
    for(std::size_t i=0;i<count;++i) {
        if(positions[i]<0 || positions[i]>=positionCount) return false;
        const int bone=BoneOf(m.vertices,VertexStride,positions[i]);
        if(bone<0 || std::size_t(bone)*12>=palette.size()) return false;
        const float* xyz=XyzOf(m.vertices,VertexStride,positions[i]);
        auto& v=vertices[i];v.attributes=attributes[i];std::copy(xyz,xyz+3,v.attributes.position);v.bone=float(bone);
        std::fill(v.normal,v.normal+4,0.f);
        if(lit) {
            if(normals[i]<0 || normals[i]>=m.normalCount) return false;
            const int normalBone=BoneOf(m.normals,NormalStride,normals[i]);
            if(normalBone<0 || std::size_t(normalBone)*12>=palette.size()) return false;
            const float* n=XyzOf(m.normals,NormalStride,normals[i]);
            std::copy(n,n+3,v.normal);v.normal[3]=float(normalBone);
            for(int k=0;k<3;++k) if(colors[normals[i]*3+k]!=m.lighting[normals[i]]*bodyLight[k]) gpuLight=false;
        }
    }
    BufferedDraw::Flush();if(!Initialize()) return false;
    if(!UploadPalette()) return false;
    BufferedDraw::Activate(program,array,buffer);FragmentPipelineGL::Apply(program);
    SelectUniforms(program);
    const float noOffset[2]{};
    SetUniforms(gpuLight,bodyLight,noOffset);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,elements);
    glBufferData(GL_ARRAY_BUFFER,vertices.size()*sizeof(Vertex),vertices.data(),GL_STREAM_DRAW);
    if(!layoutReady) {
        glEnableVertexAttribArray(0);glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,attributes.position));
        glEnableVertexAttribArray(1);glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,attributes.color));
        glEnableVertexAttribArray(2);glVertexAttribPointer(2,4,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,attributes.uv));
        glEnableVertexAttribArray(6);glVertexAttribPointer(6,4,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,normal));
        glEnableVertexAttribArray(7);glVertexAttribPointer(7,1,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,bone));
        layoutReady=true;
    }
    RenderProfile::Draw(indices);
    glDrawElements(GL_TRIANGLES,static_cast<GLsizei>(indices),GL_UNSIGNED_INT,nullptr);

    // DrawElements deja indefinidos los atributos actuales; el caller fija
    // exactamente los de la última esquina, incluso en caras incompletas.
    return true;
}
void ResidentLayout(unsigned residentBuffer) {
    // El color siempre es constante por envío (atributo 1 deshabilitado).
    glBindBuffer(GL_ARRAY_BUFFER,residentBuffer);
    glEnableVertexAttribArray(0);glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(StaticVertex),(void*)offsetof(StaticVertex,position));
    glDisableVertexAttribArray(1);
    glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,sizeof(StaticVertex),(void*)offsetof(StaticVertex,uv));
    glEnableVertexAttribArray(6);glVertexAttribPointer(6,4,GL_FLOAT,GL_FALSE,sizeof(StaticVertex),(void*)offsetof(StaticVertex,normal));
    glEnableVertexAttribArray(7);glVertexAttribPointer(7,1,GL_FLOAT,GL_FALSE,sizeof(StaticVertex),(void*)offsetof(StaticVertex,bone));
}
bool DrawResident(const ResidentDraw& d) {
    const Pose::Mesh* found=Pose::Find(d.model,d.key,d.world,d.positionCount);
    if(!found) return false;
    if(d.maxBone<0 || std::size_t(d.maxBone)*12>=Pose::Palette().size()) return false;
    if(d.lit && !Pose::LightMatches(*found,d.colors,d.normalCount,d.bodyLight)) return false;
    BufferedDraw::Flush();if(!Initialize()) return false;
    if(!UploadPalette()) return false;
    BufferedDraw::Activate(program,d.array,0);FragmentPipelineGL::Apply(program);
    SelectUniforms(program);
    const float offset[2]{d.uOffset,d.vOffset};
    SetUniforms(d.lit,d.bodyLight,offset,d.shadow?d.shadowOrigin:nullptr);
    // UV por vértice sólo si la malla se texturiza; si no, el valor actual
    // capturado, como la ruta CPU. El estado de los arrays queda en el VAO.
    if(d.textured) glEnableVertexAttribArray(2);
    else { glDisableVertexAttribArray(2);glVertexAttrib4fv(2,d.uv); }
    glVertexAttrib4fv(1,d.color);
    RenderProfile::Draw(d.indices);
    glDrawElements(GL_TRIANGLES,static_cast<GLsizei>(d.indices),GL_UNSIGNED_INT,nullptr);
    return true;
}
void Release() {
    BufferedDraw::Suspend();
    if(array) glDeleteVertexArrays(1,&array);if(buffer) glDeleteBuffers(1,&buffer);
    if(boneBuffer) glDeleteBuffers(1,&boneBuffer);if(boneTexture) glDeleteTextures(1,&boneTexture);
    if(program) FragmentPipelineGL::Release(program);
    uniformLocations.clear();uniformProgram=0;lastShadow=-1;program=array=buffer=boneBuffer=boneTexture=0;failed=false;layoutReady=false;uniformsReady=false;lastLight=-1;uploaded=0;paletteCapacity=paletteUsed=0;
    vertices.clear();
}
}
