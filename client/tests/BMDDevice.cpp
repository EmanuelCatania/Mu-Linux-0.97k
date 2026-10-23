// Verificación del backend BMD real en un contexto oculto, sin assets ni red.
#define NOMINMAX
#include "Platform/GL/glad.h"
// Cargar GL antes de los aliases ABI Win32 de la prueba POSIX.
#include "Platform/PlatformHeaders.h"
#include "Platform/GLFunctions.h"
#include "Platform/ITimer.h"
#include "Render/BMDDevice.h"
#include "Render/BufferedDraw.h"
#include "Render/FragmentStage.h"
#include "Render/MeshDraw.h"
#include "Render/RenderProfile.h"
#include "Render/ResourceTables.h"
#include "Render/Skinning.h"
#include "Render/Backend/GL/FragmentPipeline.h"
#include <SDL3/SDL.h>
#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

int DAT_083a412c=0;
char DAT_083a411c=0,DAT_083a411d=0,DAT_083a4125=0,DAT_083a411e=0,DAT_083a42ea=0;
DWORD DAT_083a42e8=0,DAT_00561574=0xffffffffu;
char* g_BitmapsRaw=ResourceTables::TextureBase();
static std::vector<std::string> diagnostics;
extern "C" void DbgLogPublic(const char* text){
    if(std::strstr(text,"PerfilR4 BMD:")){diagnostics.emplace_back(text);std::puts(text);}
}
// La ruta nueva no puede invocar estos respaldos de estado durante la sonda.
void GL_SetBlendAdditive(){std::abort();}
void GL_SetBlendSrcAlpha(){std::abort();}
void GL_SetBlendSrcOver(char){std::abort();}
void GL_ResetState(){std::abort();}
void GL_EnableLightMap(){std::abort();}
void GL_SetAlphaTest(char){std::abort();}
void GL_DisableDepthWrites(){std::abort();}
void GL_EnableDepthWrites(){std::abort();}
void GL_BindTextureSlot(int slot){assert(Materials::BindTexture({slot}));}

namespace TerrainDraw {void InvalidateBoundTexture(){} void InvalidateTexture(unsigned){}}
namespace Platform {
class ProbeTimer final:public ITimer {
    std::chrono::steady_clock::time_point origin=std::chrono::steady_clock::now();
public:
    double offset=0;
    double Now(){return offset+std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-origin).count();}
    int64_t CounterFrequency()override{return 1000000;}
    bool ReadCounter(int64_t& value)override{value=static_cast<int64_t>(Now()*1000);return true;}
    LocalDateTime LocalTime()override{return {};}
    uint32_t Ticks()override{return static_cast<uint32_t>(Now());}
    uint32_t MultimediaTicks()override{return Ticks();}
    void SleepMilliseconds(uint32_t)override{}
    TimerStart Start()override{return {static_cast<int64_t>(Now()*1000),Ticks()};}
    double Wait(const TimerStart& start,double)override{return Now()-double(start.counter)/1000;}
    void Shutdown()override{}
};
static ProbeTimer timer;
ITimer& GetTimer(){return timer;}
}
namespace {
Platform::GLProc Resolve(const char* name){return reinterpret_cast<Platform::GLProc>(SDL_GL_GetProcAddress(name));}
using Matrix=std::array<double,16>;
Matrix CurrentMatrix(){Matrix value{};glGetDoublev(GL_MODELVIEW_MATRIX,value.data());return value;}
void MatrixCases(){
    assert(!Materials::Enabled());glMatrixMode(GL_MODELVIEW);glLoadIdentity();glTranslated(.1,.2,.3);
    auto initial=CurrentMatrix();int depth=0;glGetIntegerv(GL_MODELVIEW_STACK_DEPTH,&depth);
    BMDDevice::BeginModel();int nested=0;glGetIntegerv(GL_MODELVIEW_STACK_DEPTH,&nested);assert(nested==depth+1);BMDDevice::EndModel();assert(initial==CurrentMatrix());
    Materials::Toggle();
    for(int sample=0;sample<32;++sample){
        initial=CurrentMatrix();BMDDevice::BeginModel();glGetIntegerv(GL_MODELVIEW_STACK_DEPTH,&nested);assert(nested==depth);
        BMDDevice::BeforeMatrixChange();glTranslatef(.01f*sample,.02f,.03f);auto parent=CurrentMatrix();
        BMDDevice::BeginModel();BMDDevice::BeforeMatrixChange();glRotatef(float(sample+1),0,0,1);BMDDevice::EndModel();assert(parent==CurrentMatrix());BMDDevice::EndModel();assert(initial==CurrentMatrix());
        glGetIntegerv(GL_MODELVIEW_STACK_DEPTH,&nested);assert(nested==depth);
    }
    for(int physical=0;physical<2;++physical)for(int cache=0;cache<2;++cache){
        if(physical)glEnable(GL_CULL_FACE);else glDisable(GL_CULL_FACE);DAT_083a411c=char(cache);
        {BMDDevice::MeshScope scope(true);BMDDevice::Select({BMDDevice::Pass::Color});assert(!glIsEnabled(GL_CULL_FACE)&&DAT_083a411c==0);{BMDDevice::MeshScope nestedScope(true);BMDDevice::Select({BMDDevice::Pass::Glow});assert(!glIsEnabled(GL_CULL_FACE));}}
        assert((glIsEnabled(GL_CULL_FACE)!=0)==(physical!=0)&&DAT_083a411c==physical);
    }
    Materials::Toggle();assert(glGetError()==GL_NO_ERROR);std::puts("BMD: pila propia y culling anidado correctos");
}
struct Image {std::array<unsigned char,128*128*4> color{};std::array<float,128*128> depth{};};
void NativeInput(int view,bool alpha,unsigned texture){
    BufferedDraw::Suspend();glUseProgram(0);glActiveTexture(GL_TEXTURE0);glDisable(GL_SCISSOR_TEST);glDisable(GL_STENCIL_TEST);glDisable(GL_FOG);glEnable(GL_DEPTH_TEST);glDepthFunc(GL_LEQUAL);glDepthMask(GL_TRUE);glCullFace(GL_BACK);glFrontFace(GL_CCW);
    if(alpha){glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glEnable(GL_ALPHA_TEST);glDisable(GL_CULL_FACE);}
    else{glDisable(GL_BLEND);glDisable(GL_ALPHA_TEST);glEnable(GL_CULL_FACE);}
    glAlphaFunc(GL_GREATER,.25f);glEnable(GL_TEXTURE_2D);glBindTexture(GL_TEXTURE_2D,texture);glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);glViewport(0,0,128,128);
    glMatrixMode(GL_PROJECTION);glLoadIdentity();if(view>=2)glFrustum(-1,1,-1,1,1,8);else glOrtho(-1,1,-1,1,-2,2);
    glMatrixMode(GL_MODELVIEW);glLoadIdentity();if(view>=2)glTranslatef(0,0,-2);if(view%2){glTranslatef(.1f,.05f,.03f);glRotatef(17,0,0,1);}
    glMatrixMode(GL_TEXTURE);glLoadIdentity();if(view%2){glTranslatef(.07f,.12f,0);glScalef(.8f,.7f,1);}glMatrixMode(GL_MODELVIEW);
    glClearColor(.1f,.2f,.3f,.4f);glClearDepth(1);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    DAT_083a412c=alpha?2:0;DAT_083a411c=alpha?0:1;DAT_083a411d=alpha?1:0;DAT_083a4125=1;DAT_083a411e=1;DAT_083a42e8=1;DAT_083a42ea=0;
    FragmentPipelineGL::InvalidateBindings();FragmentPipelineGL::InvalidateTextures();
}
void CheckCoreProgram(){
    int program=0;glGetIntegerv(GL_CURRENT_PROGRAM,&program);assert(program);unsigned shaders[2]{};int count=0;glGetAttachedShaders(unsigned(program),2,&count,shaders);assert(count==2);
    for(int i=0;i<count;++i){char source[8192]{};glGetShaderSource(shaders[i],sizeof(source),nullptr,source);std::string text(source);assert(text.find("#version 330 core")!=std::string::npos);for(const char* legacy:{"gl_Vertex","gl_Color","gl_ModelView","gl_TextureMatrix","gl_TexCoord","gl_FogFragCoord"})assert(text.find(legacy)==std::string::npos);}
}
Image DrawCase(bool modern,int view,bool alpha,int path){
    assert(Materials::Enabled()==modern);
    unsigned texture=0;glGenTextures(1,&texture);glBindTexture(GL_TEXTURE_2D,texture);const unsigned char pixels[16]{200,150,100,20,100,200,150,80,150,100,200,180,220,180,120,250};glTexImage2D(GL_TEXTURE_2D,0,alpha?GL_RGBA8:GL_RGB8,2,2,0,GL_RGBA,GL_UNSIGNED_BYTE,pixels);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    NativeInput(view,alpha,texture);ResourceTables::EnsureTextures(36);std::memcpy(g_BitmapsRaw+35*0x38+0x2c,&texture,4);g_BitmapsRaw[35*0x38+0x28]=alpha?4:3;DAT_00561574=0xffffffffu;
    unsigned char face[36]{},raw[48]{},normal[60]{};face[0]=3;float local[9]{-.7f,-.7f,.1f,.7f,-.7f,-.1f,0,.7f,.2f};float world[9];std::memcpy(world,local,sizeof(world));float colors[9]{.8f,.6f,.4f,.4f,.8f,.6f,.6f,.4f,.8f},uv[6]{0,0,1,0,.5f,1},body[3]{1,1,1},lighting[3]{1,1,1};
    for(int i=0;i<3;++i){short index=short(i);for(int offset:{2,10,18})std::memcpy(face+offset+i*2,&index,2);std::memcpy(raw+i*16+4,local+i*3,12);float n[3]{0,0,1};std::memcpy(normal+i*20+4,n,12);}
    int model=0;float bones[12]{1,0,0,0,0,1,0,0,0,0,1,0},direction[3]{0,0,1},coefficients[3]{.5f,.5f,.25f},origin[3]{};
    if(path){Skinning::Source source{face,raw,normal,world,lighting,3,3};Skinning::Capture(&model,bones,1,1,1,origin,direction,coefficients,&source,1);if(path==2)world[0]+=.125f;}
    else Skinning::Invalidate();
    unsigned foreignTF=0;
    if(view==3){glGenBuffers(1,&foreignTF);glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER,foreignTF);glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER,256,nullptr,GL_STATIC_DRAW);glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER,0,foreignTF,16,64);glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER,0);}
    Image image;
    {
        RenderProfile::Frame frame(modern,7);
        BMDDevice::BeginModel();
        {
            BMDDevice::MeshScope scope(false);
            if(modern)BMDDevice::Select({BMDDevice::Pass::Surface,0,35,alpha});
            auto attributes=BufferedDraw::CaptureAttributes();
            MeshDraw::Input input{face,face,1,3,3,3,world,colors,uv,uv,2,true,false,.75f,0,0,body,path?&model:nullptr,&attributes};assert(MeshDraw::Draw(input));
            if(modern){if(BMDDevice::Enabled())CheckCoreProgram();BMDDevice::Select({BMDDevice::Pass::Exit});}
        }
        BMDDevice::EndModel();
        glReadPixels(0,0,128,128,GL_RGBA,GL_UNSIGNED_BYTE,image.color.data());glReadPixels(0,0,128,128,GL_DEPTH_COMPONENT,GL_FLOAT,image.depth.data());
        // Sólo fuerza el volcado del diagnóstico; estos tiempos no son un benchmark.
        if(modern)Platform::timer.offset+=2101;
    }
    if(foreignTF){int binding=0;GLint64 start=0,size=0;glGetIntegeri_v(GL_TRANSFORM_FEEDBACK_BUFFER_BINDING,0,&binding);assert(unsigned(binding)==foreignTF);glGetInteger64i_v(GL_TRANSFORM_FEEDBACK_BUFFER_START,0,&start);glGetInteger64i_v(GL_TRANSFORM_FEEDBACK_BUFFER_SIZE,0,&size);assert(start==16&&size==64);glGetIntegerv(GL_TRANSFORM_FEEDBACK_BUFFER_BINDING,&binding);assert(binding==0);glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER,0,0);glDeleteBuffers(1,&foreignTF);}
    BufferedDraw::Suspend();glDeleteTextures(1,&texture);FragmentPipelineGL::InvalidateTextures();DAT_00561574=0xffffffffu;assert(glGetError()==GL_NO_ERROR);return image;
}
// Diagnóstico sólo ante fallo: recuperar la matriz de la referencia mediante
// transform feedback permite distinguir composición de matrices y rasterizado.
void DumpNativeMatrix(){
    float view[16],projection[16],native[16]{};glGetFloatv(GL_MODELVIEW_MATRIX,view);glGetFloatv(GL_PROJECTION_MATRIX,projection);
    const char* source="#version 330 compatibility\nout vec4 captured;void main(){captured=gl_ModelViewProjectionMatrix[gl_VertexID];gl_Position=vec4(0,0,0,1);}";
    unsigned shader=glCreateShader(GL_VERTEX_SHADER);glShaderSource(shader,1,&source,nullptr);glCompileShader(shader);unsigned program=glCreateProgram();glAttachShader(program,shader);const char* name="captured";glTransformFeedbackVaryings(program,1,&name,GL_INTERLEAVED_ATTRIBS);glLinkProgram(program);glUseProgram(program);
    unsigned buffer=0,array=0;glGenBuffers(1,&buffer);glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER,buffer);glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER,sizeof(native),nullptr,GL_STREAM_READ);glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER,0,buffer);glGenVertexArrays(1,&array);glBindVertexArray(array);glEnable(GL_RASTERIZER_DISCARD);glBeginTransformFeedback(GL_POINTS);glDrawArrays(GL_POINTS,0,4);glEndTransformFeedback();glDisable(GL_RASTERIZER_DISCARD);glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER,0,sizeof(native),native);
    for(int column=0;column<4;++column)for(int row=0;row<4;++row){float value=0;double wide=0;for(int k=0;k<4;++k){value+=projection[k*4+row]*view[column*4+k];wide+=double(projection[k*4+row])*double(view[column*4+k]);}std::fprintf(stderr,"MVP[%d]: native=%.9g float=%.9g double=%.9g\n",column*4+row,native[column*4+row],value,float(wide));}
    glUseProgram(0);glBindVertexArray(0);glDeleteVertexArrays(1,&array);glDeleteBuffers(1,&buffer);glDeleteProgram(program);glDeleteShader(shader);
}
void Images(){
    assert(!Materials::Enabled());FragmentStage::Toggle();RenderProfile::Toggle();unsigned cases=0,maxCoverage=0,maxColor=0;
    for(int view=0;view<4;++view)for(int alpha=0;alpha<2;++alpha)for(int path=0;path<3;++path){
        auto reference=DrawCase(false,view,alpha,path);Materials::Toggle();auto modern=DrawCase(true,view,alpha,path);
        BMDDevice::Toggle();auto previousStage=DrawCase(true,view,alpha,path);assert(reference.color==previousStage.color&&reference.depth==previousStage.depth);BMDDevice::Toggle();Materials::Toggle();unsigned coverage=0,color=0;
        for(std::size_t i=0;i<reference.depth.size();++i){bool a=reference.depth[i]<1,b=modern.depth[i]<1;if(a!=b){++coverage;continue;}if(reference.depth[i]!=modern.depth[i]){std::fprintf(stderr,"Profundidad: vista=%d alpha=%d ruta=%d pixel=%zu referencia=%.9g nueva=%.9g\n",view,alpha,path,i,reference.depth[i],modern.depth[i]);DumpNativeMatrix();assert(false);}for(int c=0;c<4;++c){unsigned delta=unsigned(std::abs(int(reference.color[i*4+c])-int(modern.color[i*4+c])));if(delta)++color;maxColor=std::max(maxColor,delta);}}
        if(!alpha)assert(coverage==0);else assert(coverage<=64);assert(color==0);maxCoverage=std::max(maxCoverage,coverage);++cases;
    }
    RenderProfile::Toggle();FragmentStage::Toggle();assert(!diagnostics.empty());bool checked=false;
    for(const auto& line:diagnostics){assert(line.find("error_ambito_matriz")==std::string::npos);if(line.find("errores_estado=")!=std::string::npos){assert(line.find("errores_estado=0 ")!=std::string::npos);assert(line.find("ambitos_abiertos=0")!=std::string::npos);checked=true;}}
    assert(checked);std::printf("BMD: %u escenas CPU/GPU/deformacion; opaco exacto; alpha max %u/16384 pixeles; color compartido max %u/255; validador sin errores\n",cases,maxCoverage,maxColor);
}
int Unavailable(const char* reason){std::fprintf(stderr,"Contexto GL de prueba no disponible: %s\n",reason);return std::getenv("MU_REQUIRE_GL_TEST")?1:77;}
}
int main(){
    std::setvbuf(stdout,nullptr,_IONBF,0);
    if(!SDL_Init(SDL_INIT_VIDEO))return Unavailable(SDL_GetError());
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,3);SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,3);SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);SDL_GL_SetAttribute(SDL_GL_RED_SIZE,8);SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,8);SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,8);SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,8);SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,24);
    SDL_Window* window=SDL_CreateWindow("BMD backend",128,128,SDL_WINDOW_OPENGL|SDL_WINDOW_HIDDEN);if(!window)return Unavailable(SDL_GetError());auto context=SDL_GL_CreateContext(window);if(!context)return Unavailable(SDL_GetError());if(!Platform::LoadGL33(Resolve))return Unavailable(Platform::GL33Error());
    std::printf("Driver: %s | %s\n",glGetString(GL_RENDERER),glGetString(GL_VERSION));
    for(int restart=0;restart<3;++restart){assert(BufferedDraw::Initialize());MatrixCases();Images();RenderProfile::ResetContext();MeshDraw::Release();BufferedDraw::Release();assert(glGetError()==GL_NO_ERROR);}
    SDL_GL_DestroyContext(context);SDL_DestroyWindow(window);SDL_Quit();return 0;
}
