// Sin PCH: las funciones modernas se resuelven con glad, no GL/gl.h.
#include "Platform/GL/glad.h"
#include "Render/Backend/GL/ProfileUploads.h"
#undef glBufferData
#define glBufferData(...) ::ProfileUploadGL::BufferData(glad_glBufferData,__VA_ARGS__)

#include "Platform/GLFunctions.h"
#include "Render/BufferedDraw.h"
#include "Render/Backend/Api.h"
#include "Render/Backend/GL/FragmentPipeline.h"
#include "Render/Backend/GL/ProfileQueries.h"
#include "Render/Gfx.h"
#include "Render/RenderProfile.h"
#include "Render/RenderState.h"
#include <algorithm>
#include <cstddef>
#include <cstring>

extern "C" void DbgLogPublic(const char*);
// El flujo de vértices lo comparten los backends (lo usan los inline del encabezado).
namespace BufferedDraw { Stream stream; }
namespace BufferedDraw::GL {
namespace {
// Modos de primitiva con la numeración de OpenGL 1.1 (0..9). Los que no
// existen en 3.3 core (quads, quad strip, polígono) se convierten acá.
enum Mode : unsigned { Points=0, Lines=1, LineLoop=2, LineStrip=3, Triangles=4, TriangleStrip=5, TriangleFan=6, Quads=7, QuadStrip=8, Polygon=9 };
GLuint program=0,buffer=0,array=0,quadBuffer=0;
std::size_t quadCapacity=0;
bool failed=false;
// DESVIACION: los envíos generales escriben en un buffer circular con mapeo
// no sincronizado, en vez de pedir almacenamiento nuevo con glBufferData en
// cada uno. Al llenarse se huérfana entero, como hacía cada glBufferData.
constexpr std::size_t RingBytes=4u<<20;
std::size_t ringCapacity=0,ringUsed=0;
// Devuelve el primer vértice escrito, o -1 si el mapeo falló.
GLint RingWrite(const Vertex* data,std::size_t count) {
    const std::size_t bytes=count*sizeof(Vertex);
    if(bytes>ringCapacity || ringUsed+bytes>ringCapacity) {
        ringCapacity=std::max(bytes,RingBytes);ringUsed=0;
        glBufferData(GL_ARRAY_BUFFER,ringCapacity,nullptr,GL_STREAM_DRAW);
    }
    void* destination=glMapBufferRange(GL_ARRAY_BUFFER,ringUsed,bytes,
        GL_MAP_WRITE_BIT|GL_MAP_UNSYNCHRONIZED_BIT|GL_MAP_INVALIDATE_RANGE_BIT);
    if(!destination) return -1;
    std::memcpy(destination,data,bytes);
    RenderProfile::BufferUpload(bytes); // También contar el tramo escrito por mapeo.
    if(!glUnmapBuffer(GL_ARRAY_BUFFER)) { ringCapacity=ringUsed=0;return -1; }
    const GLint first=static_cast<GLint>(ringUsed/sizeof(Vertex));
    ringUsed+=bytes;return first;
}
std::vector<Vertex> pending;
unsigned pendingMode=Triangles;
bool bound=false;
void (*exitHook)()=nullptr;
GLuint boundProgram=0,boundArray=0,boundBuffer=0;
// DESVIACION: compartir la propiedad del pipeline entre geometría general
// y mallas indexadas. Suspend olvida los bindings para que otro productor
// pueda usar el dispositivo; el próximo envío vuelve a ligar los suyos.
void BindPipeline(GLuint nextProgram,GLuint nextArray,GLuint nextBuffer) {
    if (!bound) { bound=true;boundProgram=boundArray=boundBuffer=0; }
    if (boundProgram!=nextProgram) { RenderProfileGL::ObserveProgram(nextProgram);glUseProgram(nextProgram);boundProgram=nextProgram; }
    if (boundArray!=nextArray) { glBindVertexArray(nextArray);boundArray=nextArray; }
    if (boundBuffer!=nextBuffer) { glBindBuffer(GL_ARRAY_BUFFER,nextBuffer);boundBuffer=nextBuffer; }
}
// DESVIACION: cada quad conserva los triángulos 0-1-2 y 0-2-3 sin duplicar
// sus vértices en el buffer de envío.
void EnsureQuadIndices(std::size_t count) {
    if (quadCapacity>=count) return;
    quadCapacity=std::max(count,std::max(std::size_t(256),quadCapacity*2));
    std::vector<unsigned> indices;indices.reserve(quadCapacity*6);
    for (std::size_t q=0;q<quadCapacity;++q) {
        const unsigned v=static_cast<unsigned>(q*4);
        indices.insert(indices.end(),{v,v+1,v+2,v,v+2,v+3});
    }
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,indices.size()*sizeof(unsigned),indices.data(),GL_STATIC_DRAW);
}
const char* vertexBody=R"GLSL(layout(location=0) in vec3 aPosition;
layout(location=1) in vec4 aColor;
layout(location=2) in vec4 aUV;
void main() {
    vec4 position=vec4(aPosition,1);
    gl_Position=uModelViewProjection*position;
    vColor=clamp(aColor,0.0,1.0);
    vUV=uTextureMatrix*aUV;
    vFogDistance=abs((uModelView*position).z);
}
)GLSL";
void SetCurrent(const float* color,const float* uv) {
    auto& s=RenderState::Mutable();
    if(color)std::memcpy(s.color,color,sizeof(s.color));
    if(uv)std::memcpy(s.uv,uv,sizeof(s.uv));
}
bool Independent(unsigned mode) { return mode==Triangles||mode==Quads||mode==Lines||mode==Points; }
}
bool Initialize() {
    if (program) return true;
    if (failed || !Platform::GL33Ready()) return false;
    program=FragmentPipelineGL::Create(vertexBody);
    if (!program) { failed=true;return false; }
    glGenBuffers(1,&buffer);glGenBuffers(1,&quadBuffer);glGenVertexArrays(1,&array);
    glBindVertexArray(array);glBindBuffer(GL_ARRAY_BUFFER,buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,quadBuffer);
    glEnableVertexAttribArray(0);glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,position));
    glEnableVertexAttribArray(1);glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,color));
    glEnableVertexAttribArray(2);glVertexAttribPointer(2,4,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,uv));
    glBindVertexArray(0);
    bound=false;
    if (!buffer || !quadBuffer || !array || glGetError()!=GL_NO_ERROR) { Release();failed=true;return false; }
    // Contexto nuevo: el estado en CPU arranca igual que el del dispositivo.
    Gfx::ResetState();
    return true;
}
void Release() {
    Suspend();
    stream.active=false;stream.vertices.clear();pending.clear();
    if (array) glDeleteVertexArrays(1,&array);
    if (buffer) glDeleteBuffers(1,&buffer);
    if (quadBuffer) glDeleteBuffers(1,&quadBuffer);
    if (program) FragmentPipelineGL::Release(program);
    array=buffer=program=quadBuffer=0;quadCapacity=0;ringCapacity=ringUsed=0;failed=false;
}
void NativeColor(float r,float g,float b,float a) { const float c[4]{r,g,b,a};SetCurrent(c,nullptr); }
void NativeUV(float u,float v) { const float t[4]{u,v,0,1};SetCurrent(nullptr,t); }
// Un vértice fuera de Begin/End no dibuja nada, igual que en OpenGL.
void NativeVertex(float,float,float) {}
Vertex CaptureAttributes(bool color,bool uv){
    Vertex attributes{{},{1,1,1,1},{0,0,0,1}};
    const auto& s=RenderState::Get();
    if(color)std::memcpy(attributes.color,s.color,sizeof(attributes.color));
    if(uv)std::memcpy(attributes.uv,s.uv,sizeof(attributes.uv));
    return attributes;
}
// Primitive no sigue la numeración de OpenGL; traducir a los modos 0..9.
void Begin(Primitive primitive){
    static constexpr unsigned modes[]{Points,Lines,Triangles,Quads,TriangleFan,LineLoop,LineStrip,TriangleStrip,QuadStrip,Polygon};
    Begin(modes[static_cast<unsigned>(primitive)]);
}
void Begin(unsigned mode) {
    if (!program && !Initialize()) return;
    stream.mode=mode;stream.vertices.clear();
    const auto& s=RenderState::Get();
    std::memcpy(stream.color,s.color,sizeof(stream.color));std::memcpy(stream.uv,s.uv,sizeof(stream.uv));
    const bool direct=Independent(mode);
    if (direct && !pending.empty() && pendingMode!=mode) Flush();
    if (direct) pendingMode=mode;
    stream.destination=direct ? &pending : &stream.vertices;
    stream.first=stream.destination->size();
    stream.active=true;
}
// DESVIACION: el terreno ya entrega todos los atributos; evitar la captura
// individual de cada esquina. Abanico y quad comparten la diagonal 0-2.
void Quad(const Vertex* corners,bool) {
    if (!program && !Initialize()) return;
    if (!pending.empty() && pendingMode!=Quads) Flush();
    pendingMode=Quads;pending.insert(pending.end(),corners,corners+4);
    SetCurrent(corners[3].color,corners[3].uv);
}
void Flush(const Vertex*) {
    if (pending.empty() || !program) return;
    BindPipeline(program,array,buffer);FragmentPipelineGL::Apply(program);
    GLint first=RingWrite(pending.data(),pending.size());
    if(first<0) {
        // Respaldo: el envío completo como antes.
        glBufferData(GL_ARRAY_BUFFER,pending.size()*sizeof(Vertex),pending.data(),GL_STREAM_DRAW);first=0;
    }
    if (pendingMode==Quads) {
        const std::size_t quads=pending.size()/4;
        RenderProfile::Draw(quads*6);
        EnsureQuadIndices(quads);
        glDrawElementsBaseVertex(GL_TRIANGLES,static_cast<GLsizei>(quads*6),GL_UNSIGNED_INT,nullptr,first);
    } else {
        RenderProfile::Draw(pending.size());
        glDrawArrays(pendingMode,first,(GLsizei)pending.size());
    }
    pending.clear();
}
void SetExitHook(void (*hook)()) { exitHook=hook; }
void Suspend() {
    Flush();
    if(exitHook) { auto hook=exitHook;exitHook=nullptr;hook(); }
    bound=false;boundProgram=boundArray=boundBuffer=0;
}
// DESVIACION: compartir la propiedad también con el shader de skinning.
void Activate(unsigned shader,unsigned nextArray,unsigned nextBuffer) {
    Flush();BindPipeline(shader,nextArray,nextBuffer);
}
void DrawIndexed(unsigned meshArray,unsigned meshBuffer,const Vertex* vertices,std::size_t count,std::size_t indices,const Vertex*) {
    if (!program && !Initialize()) return;
    Flush();
    BindPipeline(program,meshArray,meshBuffer);FragmentPipelineGL::Apply(program);
    glBufferData(GL_ARRAY_BUFFER,count*sizeof(Vertex),vertices,GL_STREAM_DRAW);
    RenderProfile::Draw(indices);
    glDrawElements(GL_TRIANGLES,static_cast<GLsizei>(indices),GL_UNSIGNED_INT,nullptr);
}

void End() {
    if (!stream.active) return;
    stream.active=false;
    const bool direct=stream.destination==&pending;
    const auto count=stream.destination->size()-stream.first;
    const unsigned mode=stream.mode;
    if (direct) {
        // OpenGL descarta los vértices incompletos al cerrar una primitiva.
        const unsigned group=mode==Triangles?3:mode==Quads?4:mode==Lines?2:1;
        pending.resize(pending.size()-count%group);
    } else if ((mode==TriangleFan||mode==Polygon) && count==4) {
        if (!pending.empty() && pendingMode!=Quads) Flush();
        pendingMode=Quads;pending.insert(pending.end(),stream.vertices.begin(),stream.vertices.end());
    } else if (mode==TriangleFan||mode==Polygon) {
        // Cada abanico conserva sus triángulos y su orden. No se conectan
        // abanicos de tiles distintos al reunirlos en un mismo envío.
        if (!pending.empty() && pendingMode!=Triangles) Flush();
        pendingMode=Triangles;
        for (std::size_t i=1;i+1<count;++i) {
            pending.push_back(stream.vertices[0]);pending.push_back(stream.vertices[i]);pending.push_back(stream.vertices[i+1]);
        }
    } else if (mode==QuadStrip) {
        // Quad i = (2i, 2i+1, 2i+3, 2i+2), con la misma diagonal que Quads.
        if (!pending.empty() && pendingMode!=Quads) Flush();
        pendingMode=Quads;
        for (std::size_t i=0;i+3<count;i+=2) {
            const auto& v=stream.vertices;
            pending.push_back(v[i]);pending.push_back(v[i+1]);pending.push_back(v[i+3]);pending.push_back(v[i+2]);
        }
    } else {
        // Strips y lazos no se concatenan: conservar sus conexiones originales.
        Flush();pendingMode=mode;
        pending.insert(pending.end(),stream.vertices.begin(),stream.vertices.end());
        Flush();
    }
    // El color y la UV actuales quedan en los últimos fijados dentro del bloque.
    SetCurrent(stream.color,stream.uv);
}
}
