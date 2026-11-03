// Verificación del backend BMD real en un contexto oculto, sin assets ni red.
#define NOMINMAX
#include "Platform/GL/glad.h"
// Cargar GL antes de los aliases ABI Win32 de la prueba POSIX.
#include "Platform/PlatformHeaders.h"
#include "Platform/GLFunctions.h"
#include "Platform/ITimer.h"
#include "Render/BMDDevice.h"
#include "Render/BufferedDraw.h"
#include "Render/Gfx.h"
#include "Render/MeshDraw.h"
#include "Render/RenderProfile.h"
#include "Render/RenderState.h"
#include "Render/ResourceTables.h"
#include "Render/Skinning.h"
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
// La ruta de materiales no puede invocar estos respaldos de estado.
void GL_SetBlendAdditive(){std::abort();}
void GL_SetBlendSrcAlpha(){std::abort();}
void GL_SetBlendSrcOver(char){std::abort();}
void GL_ResetState(){std::abort();}
void GL_EnableLightMap(){std::abort();}
void GL_SetAlphaTest(char){std::abort();}
void GL_DisableDepthWrites(){std::abort();}
void GL_EnableDepthWrites(){std::abort();}
void GL_BindTextureSlot(int slot){assert(Materials::BindTexture({slot}));}

namespace TerrainDraw {void InvalidateBoundTexture(){} void InvalidateTexture(std::uint32_t){}}
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
using Matrix=std::array<float,16>;
Matrix CurrentMatrix(){Matrix value{};Gfx::GetMatrix(Gfx::Matrix::ModelView,value.data());return value;}
void MatrixCases(){
    if(!Materials::Enabled())Materials::Toggle();
    Gfx::MatrixMode(Gfx::Matrix::ModelView);Gfx::LoadIdentity();Gfx::Translate(.1f,.2f,.3f);
    for(int sample=0;sample<32;++sample){
        const auto initial=CurrentMatrix();BMDDevice::BeginModel();
        Gfx::Translate(.01f*sample,.02f,.03f);const auto parent=CurrentMatrix();
        BMDDevice::BeginModel();Gfx::Rotate(float(sample+1),0,0,1);BMDDevice::EndModel();assert(parent==CurrentMatrix());
        BMDDevice::EndModel();assert(initial==CurrentMatrix());
    }
    for(int physical=0;physical<2;++physical)for(int cache=0;cache<2;++cache){
        Gfx::SetEnabled(Gfx::Cap::CullFace,physical!=0);DAT_083a411c=char(cache);
        {BMDDevice::MeshScope scope(true);BMDDevice::Select({BMDDevice::Pass::Color});assert(!Gfx::IsEnabled(Gfx::Cap::CullFace)&&DAT_083a411c==0);{BMDDevice::MeshScope nestedScope(true);BMDDevice::Select({BMDDevice::Pass::Glow});assert(!Gfx::IsEnabled(Gfx::Cap::CullFace));}}
        assert(Gfx::IsEnabled(Gfx::Cap::CullFace)==(physical!=0)&&DAT_083a411c==physical);
        assert((glIsEnabled(GL_CULL_FACE)!=0)==(physical!=0));
    }
    assert(glGetError()==GL_NO_ERROR);std::puts("BMD: pila de matrices en CPU y culling anidado correctos");
}
struct Image {std::array<unsigned char,128*128*4> color{};std::array<float,128*128> depth{};};
void SetupView(int view,bool alpha,unsigned texture){
    using C=Gfx::Cap;
    BufferedDraw::Suspend();
    Gfx::Disable(C::ScissorTest);Gfx::Disable(C::StencilTest);Gfx::Disable(C::Fog);Gfx::Enable(C::DepthTest);
    Gfx::DepthFunc(Gfx::Compare::LessEqual);Gfx::DepthMask(true);
    if(alpha){Gfx::Enable(C::Blend);Gfx::BlendFunc(Gfx::Factor::SrcAlpha,Gfx::Factor::OneMinusSrcAlpha);Gfx::Enable(C::AlphaTest);Gfx::Disable(C::CullFace);}
    else{Gfx::Disable(C::Blend);Gfx::Disable(C::AlphaTest);Gfx::Enable(C::CullFace);}
    Gfx::AlphaFunc(Gfx::Compare::Greater,.25f);Gfx::Enable(C::Texture2D);Gfx::BindTexture(texture);Gfx::TexEnvMode(Gfx::TexEnv::Modulate);Gfx::Viewport(0,0,128,128);
    Gfx::MatrixMode(Gfx::Matrix::Projection);Gfx::LoadIdentity();
    if(view>=2)Gfx::Perspective(90,1,1,8);else RenderState::Ortho(-1,1,-1,1,-2,2);
    Gfx::MatrixMode(Gfx::Matrix::ModelView);Gfx::LoadIdentity();if(view>=2)Gfx::Translate(0,0,-2);if(view%2){Gfx::Translate(.1f,.05f,.03f);Gfx::Rotate(17,0,0,1);}
    Gfx::MatrixMode(Gfx::Matrix::Texture);Gfx::LoadIdentity();
    if(view%2){Gfx::Translate(.07f,.12f,0);const float scale[16]{.8f,0,0,0, 0,.7f,0,0, 0,0,1,0, 0,0,0,1};RenderState::MultiplyMatrix(scale);}
    Gfx::MatrixMode(Gfx::Matrix::ModelView);
    Gfx::ClearColor(.1f,.2f,.3f,.4f);Gfx::Clear(Gfx::ClearColorBuffer|Gfx::ClearDepthBuffer);
    DAT_083a412c=alpha?2:0;DAT_083a411c=alpha?0:1;DAT_083a411d=alpha?1:0;DAT_083a4125=1;DAT_083a411e=1;DAT_083a42e8=1;DAT_083a42ea=0;
}
void CheckCoreProgram(){
    int program=0;glGetIntegerv(GL_CURRENT_PROGRAM,&program);assert(program);unsigned shaders[2]{};int count=0;glGetAttachedShaders(unsigned(program),2,&count,shaders);assert(count==2);
    for(int i=0;i<count;++i){char source[8192]{};glGetShaderSource(shaders[i],sizeof(source),nullptr,source);std::string text(source);assert(text.find("#version 330 core")!=std::string::npos);for(const char* legacy:{"gl_Vertex","gl_Color","gl_ModelView","gl_TextureMatrix","gl_TexCoord","gl_FogFragCoord","gl_Fog"})assert(text.find(legacy)==std::string::npos);}
}
Image DrawCase(int view,bool alpha,int path){
    const unsigned texture=Gfx::CreateTexture();Gfx::BindTexture(texture);
    const unsigned char pixels[16]{200,150,100,20,100,200,150,80,150,100,200,180,220,180,120,250};
    glTexImage2D(GL_TEXTURE_2D,0,alpha?GL_RGBA8:GL_RGB8,2,2,0,GL_RGBA,GL_UNSIGNED_BYTE,pixels);
    Gfx::TexParameter(Gfx::TexParam::MinFilter,Gfx::TexValue::Linear);Gfx::TexParameter(Gfx::TexParam::MagFilter,Gfx::TexValue::Linear);
    SetupView(view,alpha,texture);ResourceTables::EnsureTextures(36);std::memcpy(g_BitmapsRaw+35*0x38+0x2c,&texture,4);g_BitmapsRaw[35*0x38+0x28]=alpha?4:3;DAT_00561574=0xffffffffu;
    unsigned char face[36]{},raw[48]{},normal[60]{};face[0]=3;float local[9]{-.7f,-.7f,.1f,.7f,-.7f,-.1f,0,.7f,.2f};float world[9];std::memcpy(world,local,sizeof(world));float colors[9]{.8f,.6f,.4f,.4f,.8f,.6f,.6f,.4f,.8f},uv[6]{0,0,1,0,.5f,1},body[3]{1,1,1},lighting[3]{1,1,1};
    for(int i=0;i<3;++i){short index=short(i);for(int offset:{2,10,18})std::memcpy(face+offset+i*2,&index,2);std::memcpy(raw+i*16+4,local+i*3,12);float n[3]{0,0,1};std::memcpy(normal+i*20+4,n,12);}
    int model=0;float bones[12]{1,0,0,0,0,1,0,0,0,0,1,0},direction[3]{0,0,1},coefficients[3]{.5f,.5f,.25f},origin[3]{};
    if(path){Skinning::Source source{face,raw,normal,world,lighting,3,3};Skinning::Capture(&model,bones,1,1,1,origin,direction,coefficients,&source,1);if(path==2)world[0]+=.125f;}
    else Skinning::Invalidate();
    Image image;
    {
        RenderProfile::Frame frame(true,7);
        BMDDevice::BeginModel();
        {
            BMDDevice::MeshScope scope(false);
            BMDDevice::Select({BMDDevice::Pass::Surface,0,35,alpha});
            auto attributes=BufferedDraw::CaptureAttributes();
            MeshDraw::Input input{face,face,1,3,3,3,world,colors,uv,uv,2,true,false,.75f,0,0,body,path?&model:nullptr,&attributes};assert(MeshDraw::Draw(input));
            CheckCoreProgram();BMDDevice::Select({BMDDevice::Pass::Exit});
        }
        BMDDevice::EndModel();
        BufferedDraw::Flush();
        glReadPixels(0,0,128,128,GL_RGBA,GL_UNSIGNED_BYTE,image.color.data());glReadPixels(0,0,128,128,GL_DEPTH_COMPONENT,GL_FLOAT,image.depth.data());
        Platform::timer.offset+=2101;
    }
    BufferedDraw::Suspend();Gfx::DeleteTexture(texture);DAT_00561574=0xffffffffu;assert(glGetError()==GL_NO_ERROR);return image;
}
void Images(){
    if(!Materials::Enabled())Materials::Toggle();
    RenderProfile::Toggle();unsigned cases=0;
    for(int view=0;view<4;++view)for(int alpha=0;alpha<2;++alpha){
        const auto cpu=DrawCase(view,alpha,0);
        unsigned covered=0;for(float d:cpu.depth)if(d<1)++covered;
        assert(covered>0);
        // La pose en GPU y la deformada (que vuelve a la CPU) deben dar la misma imagen.
        for(int path=1;path<3;++path){
            const auto other=DrawCase(view,alpha,path);
            for(std::size_t i=0;i<cpu.depth.size();++i){
                if(cpu.depth[i]!=other.depth[i]&&path==1){std::fprintf(stderr,"Profundidad: vista=%d alpha=%d ruta=%d pixel=%zu cpu=%.9g gpu=%.9g\n",view,alpha,path,i,cpu.depth[i],other.depth[i]);assert(false);}
            }
            if(path==1)assert(cpu.color==other.color);
            ++cases;
        }
    }
    RenderProfile::Toggle();
    for(const auto& line:diagnostics)assert(line.find("error_ambito_matriz")==std::string::npos);
    std::printf("BMD: %u escenas CPU/GPU/deformacion con programas 3.3 core\n",cases);
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
