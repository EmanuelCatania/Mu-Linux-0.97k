// Sin PCH: glad es el único proveedor de tipos y funciones GL aquí.
#include "Platform/GL/glad.h"
#include "Render/RenderBackend.h"
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
bool Supported() {
    if(RenderBackend::IsWebGPU())return false; return available && Platform::GL33Ready() && glGetQueryiv && glGenQueries && glDeleteQueries && glBeginQuery && glEndQuery && glGetQueryObjectiv && glGetQueryObjectui64v; }
void BeginFrame(int map) {
    if(RenderBackend::IsWebGPU())return;
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
    if(RenderBackend::IsWebGPU())return;
    Stop();
    if(current<0 || !MeasuredOnGPU(pass))return;
    auto& slot=slots[current];
    if(slot.used==MaxSegments) { ++skippedSegments;slot.overflow=true;return; }
    auto& segment=slot.segments[slot.used++];segment.pass=pass;
    glBeginQuery(GL_TIME_ELAPSED,segment.query);running=true;
}
void EndFrame() {
    if(RenderBackend::IsWebGPU())return;
    Stop();if(current>=0)slots[current].pending=slots[current].used!=0;current=-1;
}
void Release() {
    if(RenderBackend::IsWebGPU())return;
    Stop();
    if(initialized)for(auto& slot:slots)for(auto& segment:slot.segments)if(segment.query)glDeleteQueries(1,&segment.query);
    slots={};initialized=false;available=true;current=-1;skippedFrames=skippedSegments=0;
}
std::uint64_t SkippedFrames() { return skippedFrames; }
std::uint64_t SkippedSegments() { return skippedSegments; }
void ObserveProgram(unsigned program) {
    if(RenderBackend::IsWebGPU())return;
    if(!RenderProfile::active)return;
    GLint current=0;glGetIntegerv(GL_CURRENT_PROGRAM,&current);
    if(static_cast<unsigned>(current)!=program)RenderProfile::Change(RenderProfile::State::Program);
}
void ObserveTexture(unsigned target,unsigned texture) {
    if(RenderBackend::IsWebGPU())return;
    if(!RenderProfile::active)return;
    GLint current=0;
    if(target==GL_TEXTURE_2D)glGetIntegerv(GL_TEXTURE_BINDING_2D,&current);
    else if(target==GL_TEXTURE_2D_ARRAY)glGetIntegerv(GL_TEXTURE_BINDING_2D_ARRAY,&current);
    else if(target==GL_TEXTURE_BUFFER)glGetIntegerv(GL_TEXTURE_BINDING_BUFFER,&current);
    else return;
    if(static_cast<unsigned>(current)!=texture)RenderProfile::Change(RenderProfile::State::Texture);
}
}
