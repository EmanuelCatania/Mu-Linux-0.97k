// La captura no debe duplicar la pasada ni alterar los atributos finales.
#include "Render/TerrainDraw.h"
#include "Render/Backend/GL/TerrainRenderer.h"
#include "Render/RenderProfile.h"
#include "Platform/ITimer.h"
#include <cstdio>
#include <cstdlib>
#define CHECK(x) do {if(!(x)){std::fprintf(stderr,"Fallo %d: %s\n",__LINE__,#x);std::exit(1);}}while(0)
namespace {
int starts=0,backendCalls=0,releases=0,replays=0,finals=0,lastTile=-1;
std::uint64_t seenEpoch=0;
TerrainDraw::Result next;bool bound=false;
void Reference(const TerrainDraw::Tile& t,const TerrainDraw::FrameData&,bool draw) {
    lastTile=t.x;if(draw)++replays;else ++finals;
}
class Clock final:public Platform::ITimer {
public:
 std::int64_t CounterFrequency()override{return 1000;}
 bool ReadCounter(std::int64_t& c)override{c=0;return true;}
 Platform::LocalDateTime LocalTime()override{return {};}
 std::uint32_t Ticks()override{return 0;}
 std::uint32_t MultimediaTicks()override{return 0;}
 void SleepMilliseconds(std::uint32_t)override{}
 Platform::TimerStart Start()override{return {};}
 double Wait(const Platform::TimerStart&,double)override{return 0;}
 void Shutdown()override{}
};
}
namespace Platform {ITimer& GetTimer(){static Clock c;return c;}}
namespace RenderProfile {bool active=false;void RestartCapture(){++starts;}}
extern "C" void DbgLogPublic(const char*){}
namespace TerrainRendererGL {
TerrainDraw::Result Draw(const TerrainDraw::FrameData& f,const std::vector<TerrainDraw::Tile>& tiles) {
    ++backendCalls;seenEpoch=f.textureEpoch;CHECK(tiles.size()==2);CHECK(tiles[0].x==7&&tiles[1].x==8);return next;
}
bool SourceTexture(std::uint32_t handle){return handle==42;}
bool BoundSourceTexture(){return bound;}
void Release(){++releases;}
}
int main() {
    TerrainDraw::FrameData frame;TerrainDraw::Tile a,b;a.x=7;b.x=8;
    CHECK(!TerrainDraw::Requested());TerrainDraw::Begin(frame,Reference);CHECK(!TerrainDraw::Collecting());TerrainDraw::Submit(a);CHECK(TerrainDraw::End());CHECK(backendCalls==0);
    TerrainDraw::Toggle();CHECK(starts==1&&TerrainDraw::Requested());next.drawn=true;
    TerrainDraw::Begin(frame,Reference);CHECK(TerrainDraw::Collecting());TerrainDraw::Submit(a);TerrainDraw::Submit(b);CHECK(TerrainDraw::End());
    CHECK(backendCalls==1&&replays==0&&finals==1&&lastTile==8&&!TerrainDraw::Collecting());
    const auto oldEpoch=seenEpoch;TerrainDraw::InvalidateBoundTexture();TerrainDraw::InvalidateTexture(17);
    next.drawn=true;TerrainDraw::Begin(frame,Reference);TerrainDraw::Submit(a);TerrainDraw::Submit(b);CHECK(TerrainDraw::End());CHECK(seenEpoch==oldEpoch);
    TerrainDraw::InvalidateTexture(42);bound=true;TerrainDraw::InvalidateBoundTexture();bound=false;next.drawn=false;
    TerrainDraw::Begin(frame,Reference);TerrainDraw::Submit(a);TerrainDraw::Submit(b);CHECK(TerrainDraw::End());
    CHECK(seenEpoch==oldEpoch+2&&backendCalls==3&&replays==2&&finals==2&&lastTile==8);
    CHECK(TerrainDraw::End());CHECK(backendCalls==3&&replays==2);
    TerrainDraw::Begin(frame,Reference);TerrainDraw::Release();CHECK(!TerrainDraw::Collecting()&&releases==1);
    TerrainDraw::Toggle();CHECK(!TerrainDraw::Requested()&&starts==2);
    puts("OK: captura apagada sin backend, fallback una vez, atributos finales, invalidacion y liberacion");
}
