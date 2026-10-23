// Sin PCH: el perfil no depende de globals ni del estado de juego.
#include "Render/RenderProfile.h"
#include "Render/ProfileStats.h"
#include "Render/Backend/GL/ProfileQueries.h"
#include "Render/BufferedDraw.h"
#include "Platform/ITimer.h"
#include <array>
#include <cstdio>
#include <vector>
#include <unordered_map>
#include <algorithm>
extern "C" void DbgLogPublic(const char*);
namespace RenderProfile {
bool active=false;
namespace {
bool requested=false,resetPending=false,configurationPending=true;
void (*backendReport)(bool)=nullptr;
void (*passAudit)(Pass,unsigned)=nullptr;
unsigned entryCounts[PassCount]{};
int lastWidth=0,lastHeight=0,lastLimit=0;bool lastVsync=false,lastDecoupled=false,lastTerrain=false,lastComposition=true,lastMaterials=false,lastFragments=false;
int world=-1;
Pass current=Pass::Other;
Platform::TimerStart frameStart{},phaseStart{},windowStart{};
struct Counters { std::uint64_t draws=0,vertices=0,instances=0,legacy=0,fragmentModern=0,fragmentReference=0,fragmentAlpha=0,states[StateCount]{}; };
struct Window {
    std::array<std::vector<double>,PassCount> cpu,gpu;
    std::array<Counters,PassCount> counters{};
    std::vector<double> frames,gpuFrames;
    std::uint64_t gpuSkippedStart=0,segmentsSkippedStart=0;
} window;
constexpr std::size_t TextCount=static_cast<std::size_t>(TextWork::Count);
double frameTimes[PassCount]{},textTimes[TextCount]{},interpolationTimes[3]{};
std::array<std::vector<double>,3> interpolationSamples;
struct Uploads { std::uint64_t textureBytes=0,bufferBytes=0,allocated=0,textures=0,buffers=0,repeated=0,bitmapStrings=0,atlasStrings=0; } uploads;
std::unordered_map<unsigned,unsigned> textureUploads;
struct TextureInfo { std::uint64_t bytes=0,calls=0;unsigned maxPerFrame=0; };
std::unordered_map<unsigned,TextureInfo> windowTextures;
std::array<std::vector<double>,TextCount> textSamples;
unsigned textDepth[TextCount]{};
std::vector<double> textureSamples,bufferSamples,repeatSamples,stringSamples;
Uploads windowUploads;
unsigned slowFrames=0,slowLogged=0;
double worstSlow=0;
char worstLine[1600]{};
char summary[256]="R1: recopilando CPU/GPU...";
const char* names[PassCount]={"sim","preparar","restaurar","terreno","objetos","personajes","efectos","ui","otros_dibujo","otros","swap","espera"};
void ResetWindow() {
    if(backendReport)backendReport(true);
    for(auto& values:window.cpu)values.clear();for(auto& values:window.gpu)values.clear();
    window.frames.clear();window.gpuFrames.clear();window.counters={};
    for(auto& values:textSamples)values.clear();
    for(auto& values:interpolationSamples)values.clear();
    textureSamples.clear();bufferSamples.clear();repeatSamples.clear();stringSamples.clear();
    windowTextures.clear();windowUploads={};slowFrames=slowLogged=0;worstSlow=0;worstLine[0]=0;
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
    char line[1600];
    std::snprintf(line,sizeof(line),"PerfilR1 frame: revision=3 mapa=%d cuadros=%zu cpu_prom=%.3f cpu_p95=%.3f cpu_max=%.3f gpu_cuadros=%zu gpu_pasadas_prom=%.3f gpu_pasadas_p95=%.3f gpu_pasadas_max=%.3f gpu_soportada=%d gpu_omitidos=%llu segmentos_omitidos=%llu",
        world,frame.count,frame.average,frame.p95,frame.maximum,gpu.count,gpu.average,gpu.p95,gpu.maximum,RenderProfileGL::Supported()?1:0,
        static_cast<unsigned long long>(RenderProfileGL::SkippedFrames()-window.gpuSkippedStart),static_cast<unsigned long long>(RenderProfileGL::SkippedSegments()-window.segmentsSkippedStart));
    DbgLogPublic(line);
    for(std::size_t p=0;p<PassCount;++p) {
        const auto cpu=Summarize(window.cpu[p]),g=Summarize(window.gpu[p]);const auto& c=window.counters[p];
        const double n=static_cast<double>(frame.count);
        std::snprintf(line,sizeof(line),"PerfilR1 pasada: mapa=%d pasada=%s cuadros=%zu cpu_prom=%.3f cpu_p95=%.3f cpu_max=%.3f gpu_muestras=%zu gpu_prom=%.3f gpu_p95=%.3f gpu_max=%.3f envios=%.3f vertices=%.3f instancias=%.3f blend=%.3f depth=%.3f textura=%.3f programa=%.3f legado=%.3f fragmentos_nuevos=%.3f fragmentos_referencia=%.3f fragmentos_alpha=%.3f",
            world,names[p],frame.count,cpu.average,cpu.p95,cpu.maximum,g.count,g.average,g.p95,g.maximum,
            c.draws/n,c.vertices/n,c.instances/n,c.states[0]/n,c.states[1]/n,c.states[2]/n,c.states[3]/n,c.legacy/n,c.fragmentModern/n,c.fragmentReference/n,c.fragmentAlpha/n);
        DbgLogPublic(line);
    }
    const char* textNames[TextCount]={"medir","rasterizar","componer","subir","dibujar","emision_total"};
    for(std::size_t i=0;i<TextCount;++i) {
        const auto t=Summarize(textSamples[i]);
        std::snprintf(line,sizeof(line),"PerfilR1 texto: mapa=%d etapa=%s cuadros=%zu cpu_prom=%.3f cpu_p95=%.3f cpu_max=%.3f",world,textNames[i],frame.count,t.average,t.p95,t.maximum);DbgLogPublic(line);
    }
    const char* interpolationNames[]{"capturar","aplicar","restaurar"};
    for(std::size_t i=0;i<3;++i){const auto t=Summarize(interpolationSamples[i]);
        std::snprintf(line,sizeof(line),"PerfilR1 interpolacion: mapa=%d etapa=%s cuadros=%zu cpu_prom=%.3f cpu_p95=%.3f cpu_max=%.3f",world,interpolationNames[i],frame.count,t.average,t.p95,t.maximum);DbgLogPublic(line);}
    const double n=static_cast<double>(frame.count);
    const auto tb=Summarize(textureSamples),bb=Summarize(bufferSamples),ss=Summarize(stringSamples),rs=Summarize(repeatSamples);
    std::snprintf(line,sizeof(line),"PerfilR1 subidas: mapa=%d cuadros=%zu textura_bytes_prom=%.0f textura_bytes_p95=%.0f textura_bytes_max=%.0f buffer_bytes_prom=%.0f buffer_bytes_p95=%.0f buffer_bytes_max=%.0f reserva_bytes_prom=%.0f texturas_prom=%.2f buffers_prom=%.2f repetidas_prom=%.2f repetidas_max=%.0f cadenas_prom=%.2f cadenas_p95=%.0f cadenas_max=%.0f bitmap_prom=%.2f atlas_listas_prom=%.2f lentos_12ms=%u detalle_omitido=%u",
        world,frame.count,tb.average,tb.p95,tb.maximum,bb.average,bb.p95,bb.maximum,windowUploads.allocated/n,windowUploads.textures/n,windowUploads.buffers/n,rs.average,rs.maximum,ss.average,ss.p95,ss.maximum,windowUploads.bitmapStrings/n,windowUploads.atlasStrings/n,slowFrames,slowFrames-slowLogged);
    DbgLogPublic(line);
    std::vector<std::pair<unsigned,TextureInfo>> busiest(windowTextures.begin(),windowTextures.end());
    std::sort(busiest.begin(),busiest.end(),[](const auto& a,const auto& b){return a.second.calls!=b.second.calls?a.second.calls>b.second.calls:a.first<b.first;});
    for(std::size_t i=0;i<(std::min)(busiest.size(),std::size_t(8));++i){const auto& t=busiest[i];
        std::snprintf(line,sizeof(line),"PerfilR1 textura: mapa=%d nombre=%u subidas=%llu bytes=%llu max_por_frame=%u",world,t.first,static_cast<unsigned long long>(t.second.calls),static_cast<unsigned long long>(t.second.bytes),t.second.maxPerFrame);DbgLogPublic(line);}
    if(worstLine[0])DbgLogPublic(worstLine);
    if(backendReport)backendReport(false);
    if(gpu.count)std::snprintf(summary,sizeof(summary),"R1 CPU %.1f p95 %.1f | GPU %.1f | emitir %.1f ms | %.0f cadenas | %.1f MB",frame.average,frame.p95,gpu.average,Summarize(textSamples[5]).average,ss.average,(tb.average+bb.average)/1048576.0);
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
    for(auto& value:frameTimes)value=0;for(auto& value:entryCounts)value=0;
    for(auto& value:textTimes)value=0;
    for(auto& value:interpolationTimes)value=0;
    uploads={};textureUploads.clear();
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
    window.frames.push_back(frame);
    for(std::size_t i=0;i<TextCount;++i)textSamples[i].push_back(textTimes[i]);
    for(std::size_t i=0;i<3;++i)interpolationSamples[i].push_back(interpolationTimes[i]);
    textureSamples.push_back(static_cast<double>(uploads.textureBytes));bufferSamples.push_back(static_cast<double>(uploads.bufferBytes));
    repeatSamples.push_back(static_cast<double>(uploads.repeated));stringSamples.push_back(static_cast<double>(uploads.bitmapStrings+uploads.atlasStrings));
    windowUploads.textureBytes+=uploads.textureBytes;windowUploads.bufferBytes+=uploads.bufferBytes;windowUploads.allocated+=uploads.allocated;
    windowUploads.textures+=uploads.textures;windowUploads.buffers+=uploads.buffers;windowUploads.repeated+=uploads.repeated;
    windowUploads.bitmapStrings+=uploads.bitmapStrings;windowUploads.atlasStrings+=uploads.atlasStrings;
    if(frame>12) {
        ++slowFrames;
        char detail[1600];
        std::snprintf(detail,sizeof(detail),"PerfilR1 lento: mapa=%d cpu=%.3f sim=%.3f preparar=%.3f restaurar=%.3f terreno=%.3f objetos=%.3f personajes=%.3f efectos=%.3f ui=%.3f escena=%.3f otros=%.3f swap=%.3f espera=%.3f capturar=%.3f aplicar=%.3f recuperar=%.3f texto_medir=%.3f texto_raster=%.3f texto_componer=%.3f texto_subir=%.3f texto_dibujar=%.3f texto_emitir=%.3f cadenas=%llu textura_bytes=%llu buffer_bytes=%llu repetidas=%llu",
            world,frame,frameTimes[0],frameTimes[1],frameTimes[2],frameTimes[3],frameTimes[4],frameTimes[5],frameTimes[6],frameTimes[7],frameTimes[8],frameTimes[9],frameTimes[10],frameTimes[11],interpolationTimes[0],interpolationTimes[1],interpolationTimes[2],textTimes[0],textTimes[1],textTimes[2],textTimes[3],textTimes[4],textTimes[5],
            static_cast<unsigned long long>(uploads.bitmapStrings+uploads.atlasStrings),static_cast<unsigned long long>(uploads.textureBytes),static_cast<unsigned long long>(uploads.bufferBytes),static_cast<unsigned long long>(uploads.repeated));
        // Limitar escritura: primeros ocho por ventana y siempre el peor.
        if(slowLogged<8){DbgLogPublic(detail);++slowLogged;}
        if(frame>worstSlow){worstSlow=frame;std::snprintf(worstLine,sizeof(worstLine),"%s peor_ventana=1",detail);}
    }
    active=false;
    if(Platform::GetTimer().Wait(windowStart,0)>=2000)Report();
}
Scope::Scope(Pass pass):previous(current),measured(active) { if(measured){Switch(pass);if(passAudit)passAudit(pass,entryCounts[static_cast<std::size_t>(pass)]++);} }
Scope::~Scope() { if(measured && active)Switch(previous); }
TextScope::TextScope(TextWork value):work(value),measured(active) {if(measured && textDepth[static_cast<std::size_t>(work)]++==0)start=Platform::GetTimer().Start();}
TextScope::~TextScope(){if(measured && --textDepth[static_cast<std::size_t>(work)]==0 && active)textTimes[static_cast<std::size_t>(work)]+=Platform::GetTimer().Wait(start,0);}
InterpolationScope::InterpolationScope(InterpolationWork value):work(value),measured(active){if(measured)start=Platform::GetTimer().Start();}
InterpolationScope::~InterpolationScope(){if(measured && active)interpolationTimes[static_cast<std::size_t>(work)]+=Platform::GetTimer().Wait(start,0);}
void TextString(bool bitmap){if(!active)return;if(bitmap)++uploads.bitmapStrings;else ++uploads.atlasStrings;}
void TextureUpload(unsigned texture,std::uint64_t bytes,std::uint64_t allocated){
    if(!active)return;uploads.textureBytes+=bytes;uploads.allocated+=allocated;++uploads.textures;
    if(bytes){const auto count=++textureUploads[texture];if(count>1)++uploads.repeated;
        auto& info=windowTextures[texture];info.bytes+=bytes;++info.calls;info.maxPerFrame=(std::max)(info.maxPerFrame,count);}
}
void ForgetTexture(unsigned texture){if(active)textureUploads.erase(texture);}
void BufferUpload(std::uint64_t bytes,std::uint64_t allocated){if(!active)return;uploads.bufferBytes+=bytes;uploads.allocated+=allocated;++uploads.buffers;}
void CountDraw(std::uint64_t vertices,std::uint64_t instances) {
    auto& c=window.counters[static_cast<std::size_t>(current)];++c.draws;c.vertices+=vertices;c.instances+=instances;
}
void FragmentDraw(bool modern,bool alphaFallback){auto& c=window.counters[static_cast<std::size_t>(current)];if(modern)++c.fragmentModern;else ++c.fragmentReference;if(alphaFallback)++c.fragmentAlpha;}
void SetBackendReport(void (*report)(bool)){backendReport=report;}
void SetPassAudit(void (*audit)(Pass,unsigned)){passAudit=audit;}
void CountLegacy() { ++window.counters[static_cast<std::size_t>(current)].legacy; }
void Configuration(int width,int height,int limit,bool vsync,bool decoupled,bool terrain,bool composition,bool materials,bool fragments) {
    if(!configurationPending && width==lastWidth && height==lastHeight && limit==lastLimit && vsync==lastVsync && decoupled==lastDecoupled && terrain==lastTerrain && composition==lastComposition && materials==lastMaterials && fragments==lastFragments)return;
    configurationPending=false;lastWidth=width;lastHeight=height;lastLimit=limit;lastVsync=vsync;lastDecoupled=decoupled;lastTerrain=terrain;lastComposition=composition;lastMaterials=materials;lastFragments=fragments;
    char line[320];std::snprintf(line,sizeof(line),"PerfilR1 configuracion: mapa=%d ancho=%d alto=%d limite=%d vsync_solicitado=%d desacople=%d terreno_experimental=%d composicion=%d materiales=%d fragmentos=%d intervalo_ms=2000",world,width,height,limit,vsync?1:0,decoupled?1:0,terrain?1:0,composition?1:0,materials?1:0,fragments?1:0);DbgLogPublic(line);
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
