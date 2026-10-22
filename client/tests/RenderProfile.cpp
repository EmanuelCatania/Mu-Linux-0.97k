#define GLAD_GL_IMPLEMENTATION
#include "Platform/GL/glad.h"
#include "Render/Backend/GL/ProfileQueries.h"
#include "Render/ProfileStats.h"
#include <array>
#include <cstdio>
#include <cstdlib>
#define CHECK(x) do {if(!(x)){std::fprintf(stderr,"Fallo %d: %s\n",__LINE__,#x);std::exit(1);}}while(0)
namespace {
unsigned nextQuery=1,current=0,reads=0,results=0,timerBits=64;
bool ready=false;std::array<bool,1024> pending{};
unsigned stateChanges[RenderProfile::StateCount]{},stateReads=0;
void GLAD_API_PTR StateValue(GLenum name,GLint* value){
 ++stateReads;
 if(name==GL_BLEND_SRC_RGB)*value=GL_ONE;
 else if(name==GL_BLEND_DST_RGB)*value=GL_ZERO;
 else if(name==GL_BLEND_SRC_ALPHA)*value=GL_SRC_ALPHA;
 else if(name==GL_BLEND_DST_ALPHA)*value=GL_ONE_MINUS_SRC_ALPHA;
 else *value=0;
}
const GLubyte* GLAD_API_PTR String(GLenum){return reinterpret_cast<const GLubyte*>("mock");}
void GLAD_API_PTR Query(GLenum,GLenum,GLint* value){*value=static_cast<GLint>(timerBits);}
void GLAD_API_PTR Gen(GLsizei n,GLuint* q){while(n--)*q++=nextQuery++;}
void GLAD_API_PTR Delete(GLsizei n,const GLuint* q){while(n--)pending[*q++]=false;}
void GLAD_API_PTR Begin(GLenum type,GLuint q){CHECK(type==GL_TIME_ELAPSED && current==0 && !pending[q]);current=q;}
void GLAD_API_PTR End(GLenum type){CHECK(type==GL_TIME_ELAPSED && current);pending[current]=true;current=0;}
void GLAD_API_PTR Available(GLuint q,GLenum name,GLint* value){CHECK(name==GL_QUERY_RESULT_AVAILABLE && pending[q]);*value=ready?1:0;}
void GLAD_API_PTR Result(GLuint q,GLenum name,GLuint64* value){CHECK(name==GL_QUERY_RESULT && ready && pending[q]);*value=1000000;pending[q]=false;++reads;}
}
extern "C" void DbgLogPublic(const char*){}
namespace Platform {bool GL33Ready(){return true;}}
namespace RenderProfile {
bool active=true;void CountState(State s){++stateChanges[static_cast<unsigned>(s)];}
void GpuResult(int map,const double* time,const bool* present){CHECK(map==7);CHECK(present[static_cast<unsigned>(Pass::Terrain)]);CHECK(time[static_cast<unsigned>(Pass::Terrain)]==2);CHECK(present[static_cast<unsigned>(Pass::Simulation)] && time[static_cast<unsigned>(Pass::Simulation)]==1);++results;}
}
int main(){
 using namespace RenderProfile;
 auto empty=Summarize({});CHECK(empty.count==0);
 auto one=Summarize({7});CHECK(one.average==7 && one.p95==7 && one.maximum==7);
 std::vector<double> values;for(int i=20;i>0;--i)values.push_back(i);
 auto d=Summarize(values);CHECK(d.count==20 && d.average==10.5 && d.p95==19 && d.maximum==20);
 glad_glGetIntegerv=StateValue;
 RenderProfileGL::BlendFunction(GL_ONE,GL_ZERO);CHECK(stateChanges[static_cast<unsigned>(State::Blend)]==1);
 active=false;const auto before=stateReads;RenderProfileGL::ObserveProgram(1);RenderProfileGL::ObserveTexture(GL_TEXTURE_2D,1);CHECK(stateReads==before);active=true;
 glad_glGetString=String;glad_glGetQueryiv=Query;glad_glGenQueries=Gen;glad_glDeleteQueries=Delete;glad_glBeginQuery=Begin;glad_glEndQuery=End;
 glad_glGetQueryObjectiv=Available;glad_glGetQueryObjectui64v=Result;
 for(int i=0;i<8;++i){RenderProfileGL::BeginFrame(7);RenderProfileGL::SetPass(Pass::Terrain);RenderProfileGL::SetPass(Pass::Simulation);RenderProfileGL::SetPass(Pass::Terrain);RenderProfileGL::EndFrame();}
 CHECK(reads==0 && results==0);
 RenderProfileGL::BeginFrame(7);RenderProfileGL::SetPass(Pass::Terrain);RenderProfileGL::EndFrame();CHECK(RenderProfileGL::SkippedFrames()==1 && reads==0);
 ready=true;RenderProfileGL::BeginFrame(7);CHECK(results==8 && reads==24);
 // Sin anidar queries, incluso en la misma pasada. Exceso explícito, sin bloqueo.
 for(int i=0;i<65;++i)RenderProfileGL::SetPass(Pass::Terrain);
 CHECK(RenderProfileGL::SkippedSegments()==1);RenderProfileGL::EndFrame();
 RenderProfileGL::BeginFrame(7);CHECK(results==8);RenderProfileGL::EndFrame();RenderProfileGL::Release();
 CHECK(current==0 && RenderProfileGL::SkippedFrames()==0);
 for(bool p:pending)CHECK(!p);
 timerBits=0;RenderProfileGL::BeginFrame(7);CHECK(!RenderProfileGL::Supported());RenderProfileGL::EndFrame();RenderProfileGL::Release();
 puts("OK R1: percentiles, queries no anidadas, disponibilidad, ring lleno, agregacion, desborde y liberacion.");
}
