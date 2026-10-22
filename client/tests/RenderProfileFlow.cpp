#include "Render/RenderProfile.h"
#include "Render/Backend/GL/ProfileQueries.h"
#include "Platform/ITimer.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#define CHECK(x) do {if(!(x)){std::fprintf(stderr,"Fallo %d: %s\n",__LINE__,#x);std::exit(1);}}while(0)
namespace {
long long now=0;unsigned clockCalls=0,flushes=0,gpuBegins=0,gpuChanges=0,releases=0;
std::vector<std::string> logs;
class Clock final:public Platform::ITimer {
public:
 std::int64_t CounterFrequency()override{return 1000;}
 bool ReadCounter(std::int64_t& c)override{c=now;return true;}
 Platform::LocalDateTime LocalTime()override{return {};}
 std::uint32_t Ticks()override{return static_cast<std::uint32_t>(now);}
 std::uint32_t MultimediaTicks()override{return Ticks();}
 void SleepMilliseconds(std::uint32_t n)override{now+=n;}
 Platform::TimerStart Start()override{++clockCalls;return {now,0};}
 double Wait(const Platform::TimerStart& s,double)override{++clockCalls;return static_cast<double>(now-s.counter);}
 void Shutdown()override{}
};
}
extern "C" void DbgLogPublic(const char* line){logs.emplace_back(line);}
namespace Platform {ITimer& GetTimer(){static Clock c;return c;}}
namespace BufferedDraw {void Flush(const struct Vertex*){++flushes;}}
namespace RenderProfileGL {
void BeginFrame(int){++gpuBegins;}
void SetPass(RenderProfile::Pass){++gpuChanges;}
void EndFrame(){}
void Release(){++releases;}
std::uint64_t SkippedFrames(){return 0;}
std::uint64_t SkippedSegments(){return 0;}
bool Supported(){return true;}
}
int main(){
 using namespace RenderProfile;
 for(int i=0;i<20;++i){Frame f(true,7);Scope s(Pass::Terrain);Draw(6);Change(State::Blend);}
 CHECK(clockCalls==0 && flushes==0 && gpuBegins==0 && gpuChanges==0 && logs.empty());
 Toggle();
 {
 Frame f(true,7);
 {Scope s(Pass::Terrain);now+=3;Draw(6);Change(State::Blend);}
 {Scope s(Pass::Effects);now+=2;Draw(12,2);}
 {Scope s(Pass::Effects);now+=4;Draw(18,3);}
 {Scope s(Pass::Wait);now+=2000;}
 double times[PassCount]{};bool present[PassCount]{};
 times[static_cast<unsigned>(Pass::Terrain)]=1.5;present[static_cast<unsigned>(Pass::Terrain)]=true;
 GpuResult(2,times,present); // Mapa anterior: no contaminar.
 GpuResult(7,times,present);
 }
 CHECK(gpuBegins==1 && !active);
 bool terrain=false,effects=false,frame=false;
 for(const auto& line:logs){
  if(line.find("pasada=terreno")!=std::string::npos){terrain=true;CHECK(line.find("cpu_prom=3.000")!=std::string::npos);CHECK(line.find("gpu_muestras=1 gpu_prom=1.500")!=std::string::npos);}
  if(line.find("pasada=efectos")!=std::string::npos){effects=true;CHECK(line.find("cpu_prom=6.000")!=std::string::npos);CHECK(line.find("envios=2.000 vertices=30.000 instancias=5.000")!=std::string::npos);}
  if(line.find("PerfilR1 frame:")!=std::string::npos){frame=true;CHECK(line.find("cpu_prom=2009.000")!=std::string::npos);}
 }
 CHECK(terrain && effects && frame);
 Toggle();{Frame f(true,7);} // Limpieza única al apagar.
 const auto calls=clockCalls,drawFlushes=flushes,begins=gpuBegins,changes=gpuChanges,deleted=releases,count=logs.size();
 for(int i=0;i<20;++i){Frame f(true,7);Scope s(Pass::Terrain);Draw(6);Change(State::Blend);}
 CHECK(clockCalls==calls && flushes==drawFlushes && gpuBegins==begins && gpuChanges==changes && releases==deleted && logs.size()==count);
 Toggle();
 {Frame f(true,7);{Scope s(Pass::Scene);ResetContext();}CHECK(!active);}
 CHECK(Requested());Toggle();{Frame f(false,7);}
 puts("OK R1: apagado sin reloj/GL/flush/log, fases repetidas, contadores, mapa, resumen y apagar.");
}
