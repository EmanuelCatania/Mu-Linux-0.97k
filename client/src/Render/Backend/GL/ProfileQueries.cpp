// Sin PCH: glad es el único proveedor de tipos y funciones GL aquí.
#include "Platform/GL/glad.h"
#include "Platform/GLFunctions.h"
#include "Render/Backend/GL/ProfileQueries.h"
#include <array>
#include <cstdio>
extern "C" void DbgLogPublic(const char*);
namespace RenderProfileGL {
namespace {
// Ocho cuadros de queries: nunca reutilizar un nombre con resultado pendiente.
constexpr int RingSize=8,MaxSegments=64;
struct Segment { GLuint query=0;RenderProfile::Pass pass=RenderProfile::Pass::Other; };
struct Slot { std::array<Segment,MaxSegments> segments{};int used=0,map=-1;bool pending=false,overflow=false; };
std::array<Slot,RingSize> slots{};
int current=-1;bool running=false,initialized=false,available=true;
std::uint64_t skippedFrames=0,skippedSegments=0;
void Stop() { if(running) { glEndQuery(GL_TIME_ELAPSED);running=false; } }
bool MeasuredOnGPU(RenderProfile::Pass pass) {
    using P=RenderProfile::Pass;
    // La corrida R1 detectó draws durante SIM: medirlos sin moverlos.
    // El tiempo elapsed incluye huecos de CPU, no representa GPU busy.
    return pass==P::Simulation || pass==P::Terrain || pass==P::Objects || pass==P::Characters || pass==P::Effects || pass==P::UI || pass==P::Scene;
}
}
bool Supported() { return available && Platform::GL33Ready() && glGetQueryiv && glGenQueries && glDeleteQueries && glBeginQuery && glEndQuery && glGetQueryObjectiv && glGetQueryObjectui64v; }
void BeginFrame(int map) {
    current=-1;running=false;
    if(!Supported())return;
    if(!initialized) {
        GLint bits=0;glGetQueryiv(GL_TIME_ELAPSED,GL_QUERY_COUNTER_BITS,&bits);
        if(bits==0) { available=false;return; }
        for(auto& slot:slots)for(auto& segment:slot.segments)glGenQueries(1,&segment.query);
        initialized=true;
        char line[480];std::snprintf(line,sizeof(line),"PerfilR1 dispositivo: fabricante=%.90s renderer=%.180s version=%.100s",reinterpret_cast<const char*>(glGetString(GL_VENDOR)),reinterpret_cast<const char*>(glGetString(GL_RENDERER)),reinterpret_cast<const char*>(glGetString(GL_VERSION)));DbgLogPublic(line);
    }
    for(auto& slot:slots) {
        if(!slot.pending)continue;
        bool ready=true;
        for(int i=0;i<slot.used;++i) {
            GLint available=0;glGetQueryObjectiv(slot.segments[i].query,GL_QUERY_RESULT_AVAILABLE,&available);
            if(!available) { ready=false;break; }
        }
        if(!ready)continue;
        double times[RenderProfile::PassCount]{};bool present[RenderProfile::PassCount]{};
        for(int i=0;i<slot.used;++i) {
            GLuint64 ns=0;glGetQueryObjectui64v(slot.segments[i].query,GL_QUERY_RESULT,&ns);
            const auto p=static_cast<std::size_t>(slot.segments[i].pass);
            times[p]+=static_cast<double>(ns)*1e-6;present[p]=true;
        }
        if(!slot.overflow)RenderProfile::GpuResult(slot.map,times,present);
        slot.pending=false;
    }
    for(int i=0;i<RingSize;++i)if(!slots[i].pending) {
        current=i;slots[i].map=map;slots[i].used=0;slots[i].overflow=false;return;
    }
    ++skippedFrames;
}
void SetPass(RenderProfile::Pass pass) {
    Stop();
    if(current<0 || !MeasuredOnGPU(pass))return;
    auto& slot=slots[current];
    if(slot.used==MaxSegments) { ++skippedSegments;slot.overflow=true;return; }
    auto& segment=slot.segments[slot.used++];segment.pass=pass;
    glBeginQuery(GL_TIME_ELAPSED,segment.query);running=true;
}
void EndFrame() {
    Stop();if(current>=0)slots[current].pending=slots[current].used!=0;current=-1;
}
void Release() {
    Stop();
    if(initialized)for(auto& slot:slots)for(auto& segment:slot.segments)if(segment.query)glDeleteQueries(1,&segment.query);
    slots={};initialized=false;available=true;current=-1;skippedFrames=skippedSegments=0;
}
std::uint64_t SkippedFrames() { return skippedFrames; }
std::uint64_t SkippedSegments() { return skippedSegments; }
void BlendFunction(unsigned source,unsigned destination) {
    GLint src=0,dst=0,srcAlpha=0,dstAlpha=0;
    glGetIntegerv(GL_BLEND_SRC_RGB,&src);glGetIntegerv(GL_BLEND_DST_RGB,&dst);
    glGetIntegerv(GL_BLEND_SRC_ALPHA,&srcAlpha);glGetIntegerv(GL_BLEND_DST_ALPHA,&dstAlpha);
    if(static_cast<unsigned>(src)!=source || static_cast<unsigned>(dst)!=destination ||
       static_cast<unsigned>(srcAlpha)!=source || static_cast<unsigned>(dstAlpha)!=destination)RenderProfile::Change(RenderProfile::State::Blend);
}
void SetEnabled(unsigned cap,bool enabled) {
    if(RenderProfile::active && (glIsEnabled(cap)!=0)!=enabled) {
        if(cap==GL_BLEND)RenderProfile::Change(RenderProfile::State::Blend);
        if(cap==GL_DEPTH_TEST)RenderProfile::Change(RenderProfile::State::Depth);
        if(cap==GL_TEXTURE_2D)RenderProfile::Change(RenderProfile::State::Texture);
    }
    if(enabled)glEnable(cap);else glDisable(cap);
}
void PopAttributes() {
    if(!RenderProfile::active) { glPopAttrib();return; }
    constexpr GLenum keys[]={GL_BLEND,GL_BLEND_SRC_RGB,GL_BLEND_DST_RGB,GL_BLEND_SRC_ALPHA,GL_BLEND_DST_ALPHA,
        GL_DEPTH_TEST,GL_DEPTH_WRITEMASK,GL_DEPTH_FUNC,GL_TEXTURE_2D,GL_TEXTURE_BINDING_2D};
    GLint before[10]{},after[10]{};
    for(int i=0;i<10;++i)glGetIntegerv(keys[i],before+i);
    glPopAttrib();
    for(int i=0;i<10;++i)glGetIntegerv(keys[i],after+i);
    // Una restauración puede cambiar varios grupos; contabilizar cada grupo
    // una vez, sin inferir comandos individuales dentro de la pila nativa.
    bool blend=false,depth=false,texture=false;
    for(int i=0;i<5;++i)blend|=before[i]!=after[i];
    for(int i=5;i<8;++i)depth|=before[i]!=after[i];
    for(int i=8;i<10;++i)texture|=before[i]!=after[i];
    if(blend)RenderProfile::Change(RenderProfile::State::Blend);
    if(depth)RenderProfile::Change(RenderProfile::State::Depth);
    if(texture)RenderProfile::Change(RenderProfile::State::Texture);
}
void DepthFunction(unsigned function) {
    GLint current=0;glGetIntegerv(GL_DEPTH_FUNC,&current);
    if(static_cast<unsigned>(current)!=function)RenderProfile::Change(RenderProfile::State::Depth);
}
void ObserveProgram(unsigned program) {
    if(!RenderProfile::active)return;
    GLint current=0;glGetIntegerv(GL_CURRENT_PROGRAM,&current);
    if(static_cast<unsigned>(current)!=program)RenderProfile::Change(RenderProfile::State::Program);
}
void ObserveTexture(unsigned target,unsigned texture) {
    if(!RenderProfile::active)return;
    GLint current=0;
    if(target==GL_TEXTURE_2D)glGetIntegerv(GL_TEXTURE_BINDING_2D,&current);
    else if(target==GL_TEXTURE_BUFFER)glGetIntegerv(GL_TEXTURE_BINDING_BUFFER,&current);
    else return;
    if(static_cast<unsigned>(current)!=texture)RenderProfile::Change(RenderProfile::State::Texture);
}
void DepthWrite(bool enabled) {
    GLboolean mask=0;glGetBooleanv(GL_DEPTH_WRITEMASK,&mask);
    if((mask!=0)!=enabled)RenderProfile::Change(RenderProfile::State::Depth);
}
}
