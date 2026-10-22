// Sin PCH: el perfil no depende de globals ni del estado de juego.
#include "Render/RenderProfile.h"
#include "Render/ProfileStats.h"
#include "Render/Backend/GL/ProfileQueries.h"
#include "Render/BufferedDraw.h"
#include "Platform/ITimer.h"
#include <array>
#include <cstdio>
#include <vector>
extern "C" void DbgLogPublic(const char*);
namespace RenderProfile {
bool active=false;
namespace {
bool requested=false,resetPending=false,configurationPending=true;
int lastWidth=0,lastHeight=0,lastLimit=0;bool lastVsync=false,lastDecoupled=false,lastTerrain=false,lastComposition=true,lastMaterials=false;
int world=-1;
Pass current=Pass::Other;
Platform::TimerStart frameStart{},phaseStart{},windowStart{};
struct Counters { std::uint64_t draws=0,vertices=0,instances=0,legacy=0,states[StateCount]{}; };
struct Window {
    std::array<std::vector<double>,PassCount> cpu,gpu;
    std::array<Counters,PassCount> counters{};
    std::vector<double> frames,gpuFrames;
    std::uint64_t gpuSkippedStart=0,segmentsSkippedStart=0;
} window;
double frameTimes[PassCount]{};
char summary[160]="R1: recopilando CPU/GPU...";
const char* names[PassCount]={"sim","preparar","restaurar","terreno","objetos","personajes","efectos","ui","otros_dibujo","otros","swap","espera"};
void ResetWindow() {
    for(auto& values:window.cpu)values.clear();for(auto& values:window.gpu)values.clear();
    window.frames.clear();window.gpuFrames.clear();window.counters={};
    windowStart=Platform::GetTimer().Start();
    window.gpuSkippedStart=RenderProfileGL::SkippedFrames();window.segmentsSkippedStart=RenderProfileGL::SkippedSegments();
}
void Switch(Pass next) {
    // Sólo durante el perfil: cerrar la geometría pendiente antes de cambiar
    // de query. Evita atribuir un lote a la pasada siguiente; medir su costo.
    BufferedDraw::Flush();
    auto& timer=Platform::GetTimer();
    frameTimes[static_cast<std::size_t>(current)]+=timer.Wait(phaseStart,0);
    RenderProfileGL::SetPass(next);current=next;phaseStart=timer.Start();
}
void Report() {
    const auto frame=Summarize(window.frames),gpu=Summarize(window.gpuFrames);
    char line[600];
    std::snprintf(line,sizeof(line),"PerfilR1 frame: revision=2 mapa=%d cuadros=%zu cpu_prom=%.3f cpu_p95=%.3f cpu_max=%.3f gpu_cuadros=%zu gpu_pasadas_prom=%.3f gpu_pasadas_p95=%.3f gpu_pasadas_max=%.3f gpu_soportada=%d gpu_omitidos=%llu segmentos_omitidos=%llu",
        world,frame.count,frame.average,frame.p95,frame.maximum,gpu.count,gpu.average,gpu.p95,gpu.maximum,RenderProfileGL::Supported()?1:0,
        static_cast<unsigned long long>(RenderProfileGL::SkippedFrames()-window.gpuSkippedStart),static_cast<unsigned long long>(RenderProfileGL::SkippedSegments()-window.segmentsSkippedStart));
    DbgLogPublic(line);
    for(std::size_t p=0;p<PassCount;++p) {
        const auto cpu=Summarize(window.cpu[p]),g=Summarize(window.gpu[p]);const auto& c=window.counters[p];
        const double n=static_cast<double>(frame.count);
        std::snprintf(line,sizeof(line),"PerfilR1 pasada: mapa=%d pasada=%s cuadros=%zu cpu_prom=%.3f cpu_p95=%.3f cpu_max=%.3f gpu_muestras=%zu gpu_prom=%.3f gpu_p95=%.3f gpu_max=%.3f envios=%.3f vertices=%.3f instancias=%.3f blend=%.3f depth=%.3f textura=%.3f programa=%.3f legado=%.3f",
            world,names[p],frame.count,cpu.average,cpu.p95,cpu.maximum,g.count,g.average,g.p95,g.maximum,
            c.draws/n,c.vertices/n,c.instances/n,c.states[0]/n,c.states[1]/n,c.states[2]/n,c.states[3]/n,c.legacy/n);
        DbgLogPublic(line);
    }
    if(gpu.count)std::snprintf(summary,sizeof(summary),"R1 prom/p95/max CPU %.1f/%.1f/%.1f | GPU %.1f/%.1f/%.1f ms",frame.average,frame.p95,frame.maximum,gpu.average,gpu.p95,gpu.maximum);
    else std::snprintf(summary,sizeof(summary),"R1 CPU %.1f p95 %.1f | GPU pendiente/no disponible",frame.average,frame.p95);
    ResetWindow();
}
}
void RestartCapture() { resetPending=true; }
void Toggle() { requested=!requested;resetPending=true; }
bool Requested() { return requested; }
Frame::Frame(bool inWorld,int map):measured(requested && inWorld && map>=0) {
    if(resetPending) { ResetContext();resetPending=false; }
    active=measured;
    if(!measured) { if(world!=-1)resetPending=true;world=-1;return; }
    if(world!=map) { RenderProfileGL::Release();world=map;configurationPending=true;ResetWindow();std::snprintf(summary,sizeof(summary),"R1: recopilando CPU/GPU..."); }
    frameStart=phaseStart=Platform::GetTimer().Start();current=Pass::Other;
    for(auto& value:frameTimes)value=0;
    RenderProfileGL::BeginFrame(map);
}
Frame::~Frame() {
    if(!measured || !active)return;
    Switch(Pass::Other);RenderProfileGL::EndFrame();
    const auto frame=Platform::GetTimer().Wait(frameStart,0);
    frameTimes[static_cast<std::size_t>(Pass::Other)]+=Platform::GetTimer().Wait(phaseStart,0);
    for(std::size_t p=0;p<PassCount;++p)window.cpu[p].push_back(frameTimes[p]);
    double accounted=0;for(double value:frameTimes)accounted+=value;
    // El costo del propio diagnóstico también pertenece al frame.
    if(frame>accounted)window.cpu[static_cast<std::size_t>(Pass::Other)].back()+=frame-accounted;
    window.frames.push_back(frame);active=false;
    if(Platform::GetTimer().Wait(windowStart,0)>=2000)Report();
}
Scope::Scope(Pass pass):previous(current),measured(active) { if(measured)Switch(pass); }
Scope::~Scope() { if(measured && active)Switch(previous); }
void CountDraw(std::uint64_t vertices,std::uint64_t instances) {
    auto& c=window.counters[static_cast<std::size_t>(current)];++c.draws;c.vertices+=vertices;c.instances+=instances;
}
void CountLegacy() { ++window.counters[static_cast<std::size_t>(current)].legacy; }
void Configuration(int width,int height,int limit,bool vsync,bool decoupled,bool terrain,bool composition,bool materials) {
    if(!configurationPending && width==lastWidth && height==lastHeight && limit==lastLimit && vsync==lastVsync && decoupled==lastDecoupled && terrain==lastTerrain && composition==lastComposition && materials==lastMaterials)return;
    configurationPending=false;lastWidth=width;lastHeight=height;lastLimit=limit;lastVsync=vsync;lastDecoupled=decoupled;lastTerrain=terrain;lastComposition=composition;lastMaterials=materials;
    char line[240];std::snprintf(line,sizeof(line),"PerfilR1 configuracion: mapa=%d ancho=%d alto=%d limite=%d vsync_solicitado=%d desacople=%d terreno_experimental=%d composicion=%d materiales=%d intervalo_ms=2000",world,width,height,limit,vsync?1:0,decoupled?1:0,terrain?1:0,composition?1:0,materials?1:0);DbgLogPublic(line);
}
void CountState(State state) { ++window.counters[static_cast<std::size_t>(current)].states[static_cast<std::size_t>(state)]; }
void GpuResult(int map,const double* times,const bool* present) {
    if(map!=world)return;
    double total=0;bool any=false;
    for(std::size_t p=0;p<PassCount;++p)if(present[p]) { window.gpu[p].push_back(times[p]);total+=times[p];any=true; }
    if(any)window.gpuFrames.push_back(total);
}
const char* Summary() { return summary; }
void ResetContext() { RenderProfileGL::Release();world=-1;active=false; }
}
