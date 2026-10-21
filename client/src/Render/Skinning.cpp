// Sin PCH: resolver GL moderno mediante glad.
#include "Platform/GL/glad.h"
#include "Render/Skinning.h"
#include <algorithm>
#include <cstring>

#include <vector>
extern "C" void DbgLogPublic(const char*);
namespace Skinning {
namespace {
struct BoneVertex { float xyz[3];int bone; };
struct PoseMesh { const void* key=nullptr;std::vector<BoneVertex> vertices,normals;std::vector<float> world,lighting; };
const void* poseModel=nullptr;
std::vector<float> palette;
std::vector<PoseMesh> poseMeshes;
std::size_t poseMeshCount=0;
float rotationScale=1,bodyScale=1,origin[3]{},direction[3]{},coefficients[3]{};
unsigned long long revision=0,uploaded=0;
GLuint program=0,array=0,buffer=0,boneBuffer=0,boneTexture=0;
bool failed=false,layoutReady=false,paletteBound=false,uniformsReady=false;
GLint savedTextureBuffer=0,savedTexture=0,lastLight=-1;
float lastScales[2]{},lastOrigin[3]{},lastDirection[3]{},lastCoefficients[3]{},lastBodyLight[3]{};
void RestorePalette() {
    GLint active;glGetIntegerv(GL_ACTIVE_TEXTURE,&active);
    glActiveTexture(GL_TEXTURE1);glBindTexture(GL_TEXTURE_BUFFER,savedTexture);
    glBindBuffer(GL_TEXTURE_BUFFER,savedTextureBuffer);glActiveTexture(active);paletteBound=false;
}
void SetVector(GLint location,const float* value,float* previous) {
    if(!uniformsReady || std::memcmp(value,previous,3*sizeof(float))) {
        glUniform3fv(location,1,value);std::copy(value,value+3,previous);
    }
}
GLint paletteLocation,scalesLocation,originLocation,directionLocation,coefficientsLocation,bodyLightLocation,lightLocation;
struct Vertex { BufferedDraw::Vertex attributes;float normal[4];float bone; };
std::vector<Vertex> vertices;
const char* shaderSource=R"GLSL(#version 330 compatibility
layout(location=6) in vec4 rawNormal;
layout(location=7) in float vertexBone;
uniform samplerBuffer palette;
uniform vec2 scales;
uniform vec3 origin;
uniform vec3 direction;
uniform vec3 coefficients;
uniform vec3 bodyLight;
uniform bool useLight;
vec3 rotateBone(vec3 p,int b) {
    int r=b*3;
    return vec3(dot(texelFetch(palette,r).xyz,p),dot(texelFetch(palette,r+1).xyz,p),dot(texelFetch(palette,r+2).xyz,p));
}
void main() {
    int b=int(vertexBone)*3;
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
    gl_FrontColor=color;gl_BackColor=color;
    gl_TexCoord[0]=gl_TextureMatrix[0]*gl_MultiTexCoord0;
    vec4 eye=gl_ModelViewMatrix*position;
    gl_FogFragCoord=abs(eye.z);gl_ClipVertex=eye;
}
)GLSL";
void CopyVertices(std::vector<BoneVertex>& out,const unsigned char* source,int count,int stride) {
    out.resize(count);
    for(int i=0;i<count;++i) { short bone;std::memcpy(&bone,source+i*stride,2);
        out[i].bone=bone;std::memcpy(out[i].xyz,source+i*stride+4,12); }
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
    paletteLocation=glGetUniformLocation(program,"palette");scalesLocation=glGetUniformLocation(program,"scales");
    originLocation=glGetUniformLocation(program,"origin");directionLocation=glGetUniformLocation(program,"direction");
    coefficientsLocation=glGetUniformLocation(program,"coefficients");bodyLightLocation=glGetUniformLocation(program,"bodyLight");
    lightLocation=glGetUniformLocation(program,"useLight");
    return true;
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
        auto& m=poseMeshes[poseMeshCount++];m.key=s.key;CopyVertices(m.vertices,s.vertices,s.vertexCount,16);
        CopyVertices(m.normals,s.normals,s.normalCount,20);
        m.world.assign(s.world,s.world+s.vertexCount*3);
        m.lighting.assign(s.lighting,s.lighting+s.normalCount);
    }
    poseModel=model;++revision;
}
bool Draw(const void* model,const void* key,unsigned elements,const BufferedDraw::Vertex* attributes,
    const int* positions,const int* normals,std::size_t count,std::size_t indices,
    const float* world,int positionCount,const float* colors,bool lit,const float* bodyLight) {
    if(!model || model!=poseModel) return false;
    const PoseMesh* found=nullptr;
    for(std::size_t i=0;i<poseMeshCount;++i) if(poseMeshes[i].key==key) { found=&poseMeshes[i];break; }
    if(!found) return false;
    const auto& m=*found;
    // Las pasadas deformadas modifican BoneVertex después de la pose. Deben
    // usar esos datos CPU, nunca una pose previa aunque coincida el modelo.
    if(positionCount!=int(m.vertices.size()) || std::memcmp(world,m.world.data(),m.world.size()*sizeof(float))) return false;
    bool gpuLight=lit;
    vertices.resize(count);
    for(std::size_t i=0;i<count;++i) {
        if(positions[i]<0 || positions[i]>=positionCount) return false;
        const auto& p=m.vertices[positions[i]];
        if(p.bone<0 || std::size_t(p.bone)*12>=palette.size()) return false;
        auto& v=vertices[i];v.attributes=attributes[i];std::copy(p.xyz,p.xyz+3,v.attributes.position);v.bone=float(p.bone);
        std::fill(v.normal,v.normal+4,0.f);
        if(lit) {
            if(normals[i]<0 || std::size_t(normals[i])>=m.normals.size()) return false;
            const auto& n=m.normals[normals[i]];
            if(n.bone<0 || std::size_t(n.bone)*12>=palette.size()) return false;
            std::copy(n.xyz,n.xyz+3,v.normal);v.normal[3]=float(n.bone);
            for(int k=0;k<3;++k) if(colors[normals[i]*3+k]!=m.lighting[normals[i]]*bodyLight[k]) gpuLight=false;
        }
    }
    BufferedDraw::Flush();if(!Initialize()) return false;
    if(!paletteBound) {
        GLint active;glGetIntegerv(GL_ACTIVE_TEXTURE,&active);
        glGetIntegerv(GL_TEXTURE_BUFFER,&savedTextureBuffer);
        glActiveTexture(GL_TEXTURE1);glGetIntegerv(GL_TEXTURE_BINDING_BUFFER,&savedTexture);
        glBindTexture(GL_TEXTURE_BUFFER,boneTexture);glBindBuffer(GL_TEXTURE_BUFFER,boneBuffer);
        if(!layoutReady) glTexBuffer(GL_TEXTURE_BUFFER,GL_RGBA32F,boneBuffer);
        glActiveTexture(active);paletteBound=true;BufferedDraw::SetExitHook(RestorePalette);
    }
    if(uploaded!=revision) { glBufferData(GL_TEXTURE_BUFFER,palette.size()*sizeof(float),palette.data(),GL_STREAM_DRAW);uploaded=revision; }
    BufferedDraw::Activate(program,array,buffer);
    if(!uniformsReady) glUniform1i(paletteLocation,1);
    const float scales[2]{rotationScale,bodyScale};
    if(!uniformsReady || std::memcmp(lastScales,scales,sizeof(scales))) {
        glUniform2f(scalesLocation,rotationScale,bodyScale);std::copy(scales,scales+2,lastScales);
    }
    SetVector(originLocation,origin,lastOrigin);SetVector(directionLocation,direction,lastDirection);
    SetVector(coefficientsLocation,coefficients,lastCoefficients);SetVector(bodyLightLocation,bodyLight,lastBodyLight);
    if(lastLight!=int(gpuLight)) { glUniform1i(lightLocation,gpuLight?1:0);lastLight=int(gpuLight); }
    uniformsReady=true;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,elements);
    glBufferData(GL_ARRAY_BUFFER,vertices.size()*sizeof(Vertex),vertices.data(),GL_STREAM_DRAW);
    if(!layoutReady) {
    GLint client;glGetIntegerv(GL_CLIENT_ACTIVE_TEXTURE,&client);
    glEnableClientState(GL_VERTEX_ARRAY);glVertexPointer(3,GL_FLOAT,sizeof(Vertex),(void*)offsetof(Vertex,attributes.position));
    glEnableClientState(GL_COLOR_ARRAY);glColorPointer(4,GL_FLOAT,sizeof(Vertex),(void*)offsetof(Vertex,attributes.color));
    glClientActiveTexture(GL_TEXTURE0);glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(4,GL_FLOAT,sizeof(Vertex),(void*)offsetof(Vertex,attributes.uv));
    glEnableVertexAttribArray(6);glVertexAttribPointer(6,4,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,normal));
    glEnableVertexAttribArray(7);glVertexAttribPointer(7,1,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,bone));
        glClientActiveTexture(client);layoutReady=true;
    }
    glDrawElements(GL_TRIANGLES,static_cast<GLsizei>(indices),GL_UNSIGNED_INT,nullptr);

    // DrawElements deja indefinidos los atributos actuales; el caller fija
    // exactamente los de la última esquina, incluso en caras incompletas.
    return true;
}
void Release() {
    BufferedDraw::Suspend();
    if(array) glDeleteVertexArrays(1,&array);if(buffer) glDeleteBuffers(1,&buffer);
    if(boneBuffer) glDeleteBuffers(1,&boneBuffer);if(boneTexture) glDeleteTextures(1,&boneTexture);
    if(program) glDeleteProgram(program);
    program=array=buffer=boneBuffer=boneTexture=0;failed=false;layoutReady=false;uniformsReady=false;lastLight=-1;uploaded=0;poseMeshCount=0;
    poseModel=nullptr;poseMeshes.clear();palette.clear();vertices.clear();
}
}
