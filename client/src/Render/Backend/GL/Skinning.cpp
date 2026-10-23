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
#include "Render/Backend/GL/FragmentPipeline.h"
#include "Render/RenderProfile.h"
#include "Render/Backend/GL/ProfileQueries.h"
#include <algorithm>
#include <array>
#include <map>
#include <cstring>

#include <vector>
extern "C" void DbgLogPublic(const char*);
namespace Skinning {
namespace {
// Los arrays de vértices y normales son los del BMD: estables mientras el
// modelo está cargado. Sólo se copian la posición y la luz calculadas por la
// CPU, para detectar si alguien las modificó después de la pose.
struct PoseMesh { const void* key=nullptr;const unsigned char* vertices=nullptr;const unsigned char* normals=nullptr;
    int vertexCount=0,normalCount=0;std::vector<float> world,lighting; };
constexpr int VertexStride=16,NormalStride=20;
int BoneOf(const unsigned char* base,int stride,int index) { short bone;std::memcpy(&bone,base+index*stride,2);return bone; }
const float* XyzOf(const unsigned char* base,int stride,int index) { return reinterpret_cast<const float*>(base+index*stride+4); }
const void* poseModel=nullptr;
std::vector<float> palette;
std::vector<PoseMesh> poseMeshes;
std::size_t poseMeshCount=0;
float rotationScale=1,bodyScale=1,origin[3]{},direction[3]{},coefficients[3]{};
unsigned long long revision=0,uploaded=0;
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
GLint paletteLocation,scalesLocation,originLocation,directionLocation,coefficientsLocation,bodyLightLocation,lightLocation,offsetLocation;
struct Vertex { BufferedDraw::Vertex attributes;float normal[4];float bone; };
std::vector<Vertex> vertices;
// DESVIACION: uvOffset suma el desplazamiento de textura de la malla en el
// shader; la ruta que arma los vértices en CPU ya lo trae aplicado y envía 0.
const char* shaderSource=R"GLSL(#version 330 compatibility
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
vec3 rotateBone(vec3 p,int b) {
    int r=paletteBase+b*3;
    return vec3(dot(texelFetch(palette,r).xyz,p),dot(texelFetch(palette,r+1).xyz,p),dot(texelFetch(palette,r+2).xyz,p));
}
void main() {
    int b=paletteBase+int(vertexBone)*3;
    vec3 p=rotateBone(gl_Vertex.xyz,int(vertexBone))*scales.x;
    p+=vec3(texelFetch(palette,b).w,texelFetch(palette,b+1).w,texelFetch(palette,b+2).w);
    p=p*scales.y+origin;
    vec4 position=vec4(p,1);
    gl_Position=gl_ModelViewProjectionMatrix*position;
    vec4 color=gl_Color;
    if(useLight) {
        vec3 normal=rotateBone(rawNormal.xyz,int(rawNormal.w));
        float light=max((direction.y*normal.y+direction.x*normal.x+direction.z*normal.z)*coefficients.x+coefficients.y,coefficients.z);
        color.rgb=light*bodyLight;
    }
#ifdef EXPLICIT_FRAGMENT
    color=clamp(color,0,1);
#endif
    gl_FrontColor=color;gl_BackColor=color;
    gl_TexCoord[0]=gl_TextureMatrix[0]*(gl_MultiTexCoord0+vec4(uvOffset,0,0));
    vec4 eye=gl_ModelViewMatrix*position;
    gl_FogFragCoord=abs(eye.z);gl_ClipVertex=eye;
}
)GLSL";
GLuint uniformProgram=0;
std::map<GLuint,std::array<GLint,9>> uniformLocations;
void SelectUniforms(GLuint next){
    if(uniformProgram==next)return;uniformProgram=next;uniformsReady=false;lastLight=-1;
    auto found=uniformLocations.find(next);
    if(found==uniformLocations.end()){
        const char* names[]{"paletteBase","palette","scales","origin","direction","coefficients","bodyLight","useLight","uvOffset"};
        std::array<GLint,9> locations{};for(int i=0;i<9;++i)locations[i]=glGetUniformLocation(next,names[i]);
        found=uniformLocations.emplace(next,locations).first;
    }
    const auto& p=found->second;paletteBaseLocation=p[0];paletteLocation=p[1];scalesLocation=p[2];originLocation=p[3];directionLocation=p[4];coefficientsLocation=p[5];bodyLightLocation=p[6];lightLocation=p[7];offsetLocation=p[8];
}
bool Initialize() {
    if(program) return true;
    if(failed) return false;
    const GLuint shader=glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shader,1,&shaderSource,nullptr);glCompileShader(shader);
    GLint ok;glGetShaderiv(shader,GL_COMPILE_STATUS,&ok);
    if(!ok) { char log[2048]{};glGetShaderInfoLog(shader,sizeof(log),nullptr,log);DbgLogPublic(log);glDeleteShader(shader);failed=true;return false; }
    program=glCreateProgram();glAttachShader(program,shader);glLinkProgram(program);glDeleteShader(shader);
    glGetProgramiv(program,GL_LINK_STATUS,&ok);
    if(!ok) { char log[2048]{};glGetProgramInfoLog(program,sizeof(log),nullptr,log);DbgLogPublic(log);glDeleteProgram(program);program=0;failed=true;return false; }
    glGenVertexArrays(1,&array);glGenBuffers(1,&buffer);glGenBuffers(1,&boneBuffer);glGenTextures(1,&boneTexture);
    FragmentPipelineGL::Create(program,shaderSource);
    SelectUniforms(program);
    return true;
}
const PoseMesh* FindPose(const void* model,const void* key,const float* world,int positionCount) {
    if(!model || model!=poseModel) return nullptr;
    for(std::size_t i=0;i<poseMeshCount;++i) if(poseMeshes[i].key==key) {
        const auto& m=poseMeshes[i];
        // Las pasadas deformadas modifican BoneVertex después de la pose. Deben
        // usar esos datos CPU, nunca una pose previa aunque coincida el modelo.
        if(positionCount!=m.vertexCount || std::memcmp(world,m.world.data(),m.world.size()*sizeof(float))) return nullptr;
        return &m;
    }
    return nullptr;
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
    if(uploaded!=revision) {
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
        paletteUsed+=bytes;uploaded=revision;
    }
    return true;
}
void SetUniforms(bool gpuLight,const float* bodyLight,const float* offset) {
    if(!uniformsReady) glUniform1i(paletteLocation,1);
    glUniform1i(paletteBaseLocation,paletteBase);
    const float scales[2]{rotationScale,bodyScale};
    if(!uniformsReady || std::memcmp(lastScales,scales,sizeof(scales))) {
        glUniform2f(scalesLocation,rotationScale,bodyScale);std::copy(scales,scales+2,lastScales);
    }
    SetVector(originLocation,origin,lastOrigin);SetVector(directionLocation,direction,lastDirection);
    SetVector(coefficientsLocation,coefficients,lastCoefficients);SetVector(bodyLightLocation,bodyLight,lastBodyLight);
    if(lastLight!=int(gpuLight)) { glUniform1i(lightLocation,gpuLight?1:0);lastLight=int(gpuLight); }
    if(!uniformsReady || std::memcmp(lastOffset,offset,sizeof(lastOffset))) {
        glUniform2f(offsetLocation,offset[0],offset[1]);std::copy(offset,offset+2,lastOffset);
    }
    uniformsReady=true;
}
}
void Invalidate() { poseModel=nullptr; }
void Capture(const void* model,const float* bones,int boneCount,float rotation,float scale,
    const float* translation,const float* lightDirection,const float* lightCoefficients,
    const Source* sources,std::size_t count) {
    Invalidate();if(!model || !bones || boneCount<=0) return;
    palette.assign(bones,bones+std::size_t(boneCount)*12);rotationScale=rotation;bodyScale=scale;
    std::copy(translation,translation+3,origin);std::copy(lightDirection,lightDirection+3,direction);
    std::copy(lightCoefficients,lightCoefficients+3,coefficients);
    // DESVIACION: conservar capacidad por orden de malla entre objetos.
    // No liberar y recrear todos los vectores al cambiar de modelo.
    if(poseMeshes.size()<count) poseMeshes.resize(count);
    poseMeshCount=0;
    for(std::size_t i=0;i<count;++i) { const auto& s=sources[i];
        if(s.vertexCount<=0 || !s.vertices || !s.world || s.normalCount<0 || (s.normalCount && (!s.normals || !s.lighting))) continue;
        auto& m=poseMeshes[poseMeshCount++];m.key=s.key;
        m.vertices=s.vertices;m.normals=s.normals;m.vertexCount=s.vertexCount;m.normalCount=s.normalCount;
        m.world.assign(s.world,s.world+s.vertexCount*3);
        m.lighting.assign(s.lighting,s.lighting+s.normalCount);
    }
    poseModel=model;++revision;
}
bool Draw(const void* model,const void* key,unsigned elements,const BufferedDraw::Vertex* attributes,
    const int* positions,const int* normals,std::size_t count,std::size_t indices,
    const float* world,int positionCount,const float* colors,bool lit,const float* bodyLight) {
    const PoseMesh* found=FindPose(model,key,world,positionCount);
    if(!found) return false;
    const auto& m=*found;
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
    FragmentPipelineGL::Draw material(program,&vertices[0].attributes.color[3],vertices.size(),sizeof(Vertex),true);
    BufferedDraw::Activate(material.Program(),array,buffer);material.Apply();
    SelectUniforms(material.Program());
    const float noOffset[2]{};
    SetUniforms(gpuLight,bodyLight,noOffset);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,elements);
    glBufferData(GL_ARRAY_BUFFER,vertices.size()*sizeof(Vertex),vertices.data(),GL_STREAM_DRAW);
    if(!layoutReady) {
    GLint client;glGetIntegerv(GL_CLIENT_ACTIVE_TEXTURE,&client);
    glEnableVertexAttribArray(0);glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,attributes.position));
    glEnableVertexAttribArray(1);glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,attributes.color));
    glEnableVertexAttribArray(2);glVertexAttribPointer(2,4,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,attributes.uv));
    glEnableClientState(GL_VERTEX_ARRAY);glVertexPointer(3,GL_FLOAT,sizeof(Vertex),(void*)offsetof(Vertex,attributes.position));
    glEnableClientState(GL_COLOR_ARRAY);glColorPointer(4,GL_FLOAT,sizeof(Vertex),(void*)offsetof(Vertex,attributes.color));
    glClientActiveTexture(GL_TEXTURE0);glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(4,GL_FLOAT,sizeof(Vertex),(void*)offsetof(Vertex,attributes.uv));
    glEnableVertexAttribArray(6);glVertexAttribPointer(6,4,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,normal));
    glEnableVertexAttribArray(7);glVertexAttribPointer(7,1,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,bone));
        glClientActiveTexture(client);layoutReady=true;
    }
    RenderProfile::Draw(indices);
    glDrawElements(GL_TRIANGLES,static_cast<GLsizei>(indices),GL_UNSIGNED_INT,nullptr);

    // DrawElements deja indefinidos los atributos actuales; el caller fija
    // exactamente los de la última esquina, incluso en caras incompletas.
    return true;
}
void ResidentLayout(unsigned residentBuffer) {
    // Los dos programas comparten el VAO: la variante moderna lee los
    // atributos genéricos (0 posición, 1 color, 2 UV) y la de referencia los
    // arrays de compatibilidad. El color siempre es constante por envío.
    GLint client;glGetIntegerv(GL_CLIENT_ACTIVE_TEXTURE,&client);
    glBindBuffer(GL_ARRAY_BUFFER,residentBuffer);
    glEnableVertexAttribArray(0);glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(StaticVertex),(void*)offsetof(StaticVertex,position));
    glDisableVertexAttribArray(1);
    glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,sizeof(StaticVertex),(void*)offsetof(StaticVertex,uv));
    glEnableClientState(GL_VERTEX_ARRAY);glVertexPointer(3,GL_FLOAT,sizeof(StaticVertex),(void*)offsetof(StaticVertex,position));
    glDisableClientState(GL_COLOR_ARRAY);
    glClientActiveTexture(GL_TEXTURE0);glTexCoordPointer(2,GL_FLOAT,sizeof(StaticVertex),(void*)offsetof(StaticVertex,uv));
    glEnableVertexAttribArray(6);glVertexAttribPointer(6,4,GL_FLOAT,GL_FALSE,sizeof(StaticVertex),(void*)offsetof(StaticVertex,normal));
    glEnableVertexAttribArray(7);glVertexAttribPointer(7,1,GL_FLOAT,GL_FALSE,sizeof(StaticVertex),(void*)offsetof(StaticVertex,bone));
    glClientActiveTexture(client);
}
bool DrawResident(const ResidentDraw& d) {
    const PoseMesh* found=FindPose(d.model,d.key,d.world,d.positionCount);
    if(!found) return false;
    const auto& m=*found;
    if(d.maxBone<0 || std::size_t(d.maxBone)*12>=palette.size()) return false;
    if(d.lit) {
        // El shader calcula la luz por normal; si la CPU dejó otro color (un
        // efecto que lo modificó), dibujar con los colores CPU.
        if(d.normalCount!=m.normalCount || !d.colors) return false;
        for(int n=0;n<m.normalCount;++n)
            for(int k=0;k<3;++k) if(d.colors[n*3+k]!=m.lighting[n]*d.bodyLight[k]) return false;
    }
    BufferedDraw::Flush();if(!Initialize()) return false;
    if(!UploadPalette()) return false;
    FragmentPipelineGL::Draw material(program,&d.color[3],1,4*sizeof(float),true);
    BufferedDraw::Activate(material.Program(),d.array,0);material.Apply();
    SelectUniforms(material.Program());
    const float offset[2]{d.uOffset,d.vOffset};
    SetUniforms(d.lit,d.bodyLight,offset);
    // UV por vértice sólo si la malla se texturiza; si no, el valor actual
    // capturado, como la ruta CPU. El estado de los arrays queda en el VAO.
    GLint client;glGetIntegerv(GL_CLIENT_ACTIVE_TEXTURE,&client);
    glClientActiveTexture(GL_TEXTURE0);
    if(d.textured) { glEnableVertexAttribArray(2);glEnableClientState(GL_TEXTURE_COORD_ARRAY); }
    else {
        glDisableVertexAttribArray(2);glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glVertexAttrib4fv(2,d.uv);glMultiTexCoord4fv(GL_TEXTURE0,d.uv);
    }
    glClientActiveTexture(client);
    glVertexAttrib4fv(1,d.color);glColor4fv(d.color);
    RenderProfile::Draw(d.indices);
    glDrawElements(GL_TRIANGLES,static_cast<GLsizei>(d.indices),GL_UNSIGNED_INT,nullptr);
    return true;
}
void Release() {
    BufferedDraw::Suspend();
    if(array) glDeleteVertexArrays(1,&array);if(buffer) glDeleteBuffers(1,&buffer);
    if(boneBuffer) glDeleteBuffers(1,&boneBuffer);if(boneTexture) glDeleteTextures(1,&boneTexture);
    FragmentPipelineGL::Release(program);
    if(program) glDeleteProgram(program);
    uniformLocations.clear();uniformProgram=0;program=array=buffer=boneBuffer=boneTexture=0;failed=false;layoutReady=false;uniformsReady=false;lastLight=-1;uploaded=0;poseMeshCount=0;paletteCapacity=paletteUsed=0;
    poseModel=nullptr;poseMeshes.clear();palette.clear();vertices.clear();
}
}
