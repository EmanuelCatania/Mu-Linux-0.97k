#include "Render/RenderProfile.h"
// Sonda sin contexto: sólo la instrumentación consulta estos bindings.
static int fakeTexture=17;
static unsigned bindingReads=0;
void glGetIntegerv(unsigned query,int* value){++bindingReads;*value=query==0x88EF?0:fakeTexture;}
#include "Render/Backend/GL/ProfileUploads.h"
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
struct AuditCall {RenderProfile::Pass pass;unsigned occurrence;};
std::vector<AuditCall> audits;
void Audit(RenderProfile::Pass pass,unsigned occurrence){CHECK(flushes>0);audits.push_back({pass,occurrence});}
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
 SetPassAudit(Audit);
 for(int i=0;i<20;++i){Frame f(true,7);Scope s(Pass::Terrain);Draw(6);Change(State::Blend);TextScope text(TextWork::Measure);InterpolationScope interpolation(InterpolationWork::Capture);TextureUpload(17,20);BufferUpload(10);}
 CHECK(clockCalls==0 && flushes==0 && gpuBegins==0 && gpuChanges==0 && logs.empty() && audits.empty());
 const auto image=[](unsigned,int,int,int,int,int,unsigned,unsigned,const void*){};
 const auto beforeReads=bindingReads;
 ProfileUploadGL::Image2D(image,0xDE1,0,4,2,3,0,0x1908,0x1401,(void*)1);
 CHECK(bindingReads==beforeReads);
 CHECK(ProfileUploadGL::PixelBytes(0x1907,0x1406)==12);
 CHECK(ProfileUploadGL::PixelBytes(0x1908,0x8033)==2);
 Toggle();
 {Frame invalid(true,-1);Scope s(Pass::Terrain);}
 CHECK(clockCalls==0 && flushes==0 && gpuBegins==0 && logs.empty());
 {
 Frame f(true,7);
 {TextScope text(TextWork::Measure);now+=1;}
 {TextScope text(TextWork::Raster);now+=2;}
 {InterpolationScope interpolation(InterpolationWork::Capture);now+=1;}
 TextString(true);TextString(false);
 ProfileUploadGL::Image2D(image,0xDE1,0,4,2,3,0,0x1908,0x1401,(void*)1);
 ProfileUploadGL::Image2D(image,0xDE1,0,4,2,3,0,0x1908,0x1401,(void*)1);
 ProfileUploadGL::Image2D(image,0xDE1,0,4,2,3,0,0x1908,0x1401,nullptr);
 BufferUpload(100,200);
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
  if(line.find("PerfilR1 frame:")!=std::string::npos){frame=true;CHECK(line.find("cpu_prom=2013.000")!=std::string::npos);}
 }
 bool text=false,uploads=false,slow=false,interpolation=false,textureId=false;
 for(const auto& line:logs){
  if(line.find("etapa=medir")!=std::string::npos){text=true;CHECK(line.find("cpu_prom=1.000")!=std::string::npos);}
  if(line.find("PerfilR1 subidas:")!=std::string::npos){uploads=true;CHECK(line.find("textura_bytes_prom=48")!=std::string::npos);CHECK(line.find("buffer_bytes_prom=100")!=std::string::npos);CHECK(line.find("repetidas_prom=1.00")!=std::string::npos);CHECK(line.find("cadenas_prom=2.00")!=std::string::npos);}
  if(line.find("PerfilR1 interpolacion:")!=std::string::npos && line.find("etapa=capturar")!=std::string::npos){interpolation=true;CHECK(line.find("cpu_prom=1.000")!=std::string::npos);}
  if(line.find("PerfilR1 textura:")!=std::string::npos){textureId=true;CHECK(line.find("nombre=17 subidas=2 bytes=48 max_por_frame=2")!=std::string::npos);}
  if(line.find("PerfilR1 lento:")!=std::string::npos){slow=true;CHECK(line.find("terreno=3.000")!=std::string::npos);}
 }
 CHECK(terrain && effects && frame && text && uploads && slow && interpolation && textureId);
 CHECK(audits.size()==4);
 CHECK(audits[0].pass==Pass::Terrain && audits[0].occurrence==0);
 CHECK(audits[1].pass==Pass::Effects && audits[1].occurrence==0);
 CHECK(audits[2].pass==Pass::Effects && audits[2].occurrence==1);
 // Reanudar la pasada anterior no simula una entrada nueva; el ordinal se
 // reinicia en el cuadro siguiente, aunque la ventana de perfil siga abierta.
 {Frame f(true,7);Scope s(Pass::Terrain);}
 CHECK(audits.size()==5 && audits.back().pass==Pass::Terrain && audits.back().occurrence==0);
 Toggle();{Frame f(true,7);} // Limpieza única al apagar.
 const auto calls=clockCalls,drawFlushes=flushes,begins=gpuBegins,changes=gpuChanges,deleted=releases;
 const auto count=logs.size(),auditCount=audits.size();
 for(int i=0;i<20;++i){Frame f(true,7);Scope s(Pass::Terrain);Draw(6);Change(State::Blend);TextScope text(TextWork::Measure);InterpolationScope interpolation(InterpolationWork::Capture);TextureUpload(17,20);BufferUpload(10);}
 CHECK(clockCalls==calls && flushes==drawFlushes && gpuBegins==begins && gpuChanges==changes && releases==deleted && logs.size()==count && audits.size()==auditCount);
 Toggle();
 {Frame f(true,7);TextureUpload(17,4);ForgetTexture(17);TextureUpload(17,4);now+=2001;}
 CHECK(logs[logs.size()-2].find("R1")!=std::string::npos);
 for(auto it=logs.rbegin();it!=logs.rend();++it)if(it->find("PerfilR1 subidas:")!=std::string::npos){CHECK(it->find("repetidas_prom=0.00")!=std::string::npos);break;}
 {Frame f(true,7);{Scope s(Pass::Scene);ResetContext();}CHECK(!active);}
 CHECK(Requested());Toggle();{Frame f(false,7);}
 puts("OK R1: apagado sin reloj/GL/flush/log, fases repetidas, contadores, mapa, resumen y apagar.");
}
