// Sin PCH: captura de la pasada independiente de globals de juego y de GL.
#include "Render/TerrainDraw.h"
#include "Render/Backend/GL/TerrainRenderer.h"
#include "Render/RenderProfile.h"
#include "Platform/ITimer.h"
#include <cstdio>
extern "C" void DbgLogPublic(const char*);
namespace TerrainDraw {
namespace {
bool requested=false,collecting=false;
std::uint64_t epoch=1;FrameData frame;Reference reference=nullptr;
std::vector<Tile> tiles;
const char* status="Terreno: actual";
Platform::TimerStart start{};bool logging=false;int logMap=-1;
std::uint64_t frames=0,tileCount=0,draws=0,fallbacks=0,geometry=0,light=0,data=0;
void ResetLog() { logging=false;frames=tileCount=draws=fallbacks=geometry=light=data=0; }
}
void Toggle() { requested=!requested;RenderProfile::RestartCapture();status=requested?"Terreno experimental: esperando pasada":"Terreno: actual";ResetLog(); }
bool Requested() { return requested; }
bool Collecting() { return collecting; }
const char* Status() { return status; }
void InvalidateTextures() { ++epoch; }
void Begin(FrameData next,Reference replay) {
    collecting=requested;tiles.clear();frame=next;frame.textureEpoch=epoch;reference=replay;
}
void Submit(const Tile& tile) { if(collecting)tiles.push_back(tile); }
bool End() {
    if(!collecting)return true;collecting=false;
    const auto result=TerrainRendererGL::Draw(frame,tiles);
    if(!result.drawn && reference)for(const auto& tile:tiles)reference(tile,frame,true);
    if(result.drawn && !tiles.empty() && reference)reference(tiles.back(),frame,false);
    status=result.drawn?"Terreno experimental: activo":result.reason;
    if(RenderProfile::active) {
        if(logMap!=frame.map){ResetLog();logMap=frame.map;}
        if(!logging) { start=Platform::GetTimer().Start();logging=true; }
        ++frames;tileCount+=tiles.size();draws+=result.draws;fallbacks+=result.drawn?result.referenceTiles:tiles.size();
        geometry+=result.geometryBytes;light+=result.lightBytes;data+=result.dataBytes;
        if(Platform::GetTimer().Wait(start,0)>=2000) {
            char line[400];const auto n=static_cast<double>(frames);
            std::snprintf(line,sizeof(line),"PerfilR3 terreno: mapa=%d cuadros=%llu tiles=%.2f envios_nuevos=%.2f tiles_referencia=%.2f geometria_bytes=%.0f luz_bytes=%.0f datos_bytes=%.0f",
                frame.map,static_cast<unsigned long long>(frames),tileCount/n,draws/n,fallbacks/n,geometry/n,light/n,data/n);DbgLogPublic(line);ResetLog();
        }
    } else ResetLog();
    return result.drawn || reference!=nullptr;
}
void Release() { collecting=false;tiles.clear();TerrainRendererGL::Release();ResetLog(); }
}
