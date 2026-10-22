// Sin PCH: GL y traducción del estado permanecen en el backend.
#include "Platform/GL/glad.h"
#include "Render/FragmentStage.h"
#include "Render/Backend/GL/FragmentPipeline.h"
#include "Render/BufferedDraw.h"
#include "Render/RenderProfile.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <cstdio>
#include <map>
#include <string>
extern "C" void DbgLogPublic(const char*);
namespace {
bool requested=false,layoutChecked=false,targetChecked=false,targetSupported=false;
const char* status="Fragmentos: referencia";
struct Pipeline { unsigned program=0;int texture=-1,textured=-1,fog=-1,values=-1,color=-1,reject=-1,combine=-1;bool valid=false,lastTextured=false,lastReject=false;int lastFog=0,lastCombine=0;float lastValues[3]{},lastColor[4]{}; };
std::map<unsigned,Pipeline> pipelines;
struct TextureInfo{bool supported=false,opaque=false;int combine=0;};
std::map<unsigned,TextureInfo> textureInfo;
unsigned long long selected=0,fallback=0,alphaFallback=0,nativeAlpha=0;
const char* lastReason="";
const char* fragment=R"GLSL(#version 330 compatibility
uniform sampler2D materialTexture;
uniform bool materialTextured,materialReject;
uniform int materialFog,materialCombine;
uniform vec3 materialFogValues;
uniform vec4 materialFogColor;
out vec4 resultColor;
void main(){
    if(materialReject)discard;
    vec4 primary=gl_Color;
    vec4 c=materialTextured?textureProj(materialTexture,gl_TexCoord[0])*primary:primary;
    if(materialTextured&&materialCombine==1)c.a=primary.a;
    if(materialTextured&&materialCombine==2)c.rgb=primary.rgb;
    if(materialFog!=0){
        float d=gl_FogFragCoord,f=1;
        if(materialFog==1)f=(materialFogValues.y-d)/(materialFogValues.y-materialFogValues.x);
        if(materialFog==2)f=exp(-materialFogValues.z*d);
        if(materialFog==3){float v=materialFogValues.z*d;f=exp(-v*v);}
        c.rgb=mix(materialFogColor.rgb,c.rgb,clamp(f,0,1));
    }
    resultColor=c;
}
)GLSL";
void Report(bool reset){
    if(!reset&&requested){char line[320];std::snprintf(line,sizeof(line),"PerfilR4b fragmentos: nuevos=%llu referencia=%llu referencia_alpha=%llu alpha_nativo=%llu ultimo_motivo=\"%s\"",selected,fallback,alphaFallback,nativeAlpha,lastReason);DbgLogPublic(line);}
    selected=fallback=alphaFallback=nativeAlpha=0;lastReason="";
}
void Count(bool modern,const char* reason="",bool alpha=false){
    status=modern?"Fragmentos: shader activo":reason;
    if(!RenderProfile::active)return;
    RenderProfile::FragmentDraw(modern,alpha);
    if(modern)++selected;else{++fallback;if(alpha)++alphaFallback;lastReason=reason;}
}
TextureInfo InspectTexture(unsigned texture){
    const auto found=textureInfo.find(texture);if(found!=textureInfo.end())return found->second;
    int format=0,swizzle=0,width=0,height=0,filter=0;
    glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_INTERNAL_FORMAT,&format);
    glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_WIDTH,&width);glGetTexLevelParameteriv(GL_TEXTURE_2D,0,GL_TEXTURE_HEIGHT,&height);
    glGetTexParameteriv(GL_TEXTURE_2D,GL_TEXTURE_SWIZZLE_A,&swizzle);glGetTexParameteriv(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,&filter);
    TextureInfo info;info.opaque=swizzle==GL_ONE;
    switch(format){
    case 3:case GL_RGB:case GL_RGB8:case GL_RGB4:case GL_RGB5:case GL_R3_G3_B2:case GL_RGB10:case GL_RGB12:case GL_RGB16:
    case 1:case GL_LUMINANCE:case GL_LUMINANCE8:case GL_LUMINANCE12:case GL_LUMINANCE16:
        info.supported=true;info.opaque=info.opaque||swizzle==GL_ALPHA;info.combine=1;break;
    case GL_ALPHA:case GL_ALPHA8:case GL_ALPHA4:case GL_ALPHA12:case GL_ALPHA16:
        info.supported=true;info.combine=2;break;
    case 2:case 4:case GL_RGBA:case GL_RGBA8:case GL_RGBA4:case GL_RGB5_A1:case GL_RGB10_A2:case GL_RGBA12:case GL_RGBA16:
    case GL_LUMINANCE_ALPHA:case GL_LUMINANCE8_ALPHA8:case GL_LUMINANCE4_ALPHA4:case GL_LUMINANCE6_ALPHA2:case GL_LUMINANCE12_ALPHA4:case GL_LUMINANCE12_ALPHA12:case GL_LUMINANCE16_ALPHA16:
    case GL_INTENSITY:case GL_INTENSITY8:case GL_INTENSITY4:case GL_INTENSITY12:case GL_INTENSITY16:
        info.supported=true;break;
    default:break;
    }
    info.supported=info.supported&&(swizzle==GL_ALPHA||swizzle==GL_ONE)&&width>0&&height>0&&(filter==GL_NEAREST||filter==GL_LINEAR);
    textureInfo[texture]=info;return info;
}
// No aproximar el comparador nativo cerca del umbral. Una cobertura variable
// conserva el test nativo; las coberturas constantes se resuelven sin test fijo.
int Coverage(int function,float reference,const float* alpha,std::size_t count,std::size_t stride,bool opaque){
    if(function==GL_ALWAYS)return 1;if(function==GL_NEVER)return -1;
    float lo=1,hi=0;
    for(std::size_t i=0;i<count;++i){float value=*reinterpret_cast<const float*>(reinterpret_cast<const unsigned char*>(alpha)+i*stride);if(!std::isfinite(value))return 0;value=std::clamp(value,0.f,1.f);lo=std::min(lo,value);hi=std::max(hi,value);}
    if(!opaque)lo=0;
    constexpr float margin=2.f/255.f;
    const bool below=hi<reference-margin,above=lo>reference+margin;
    switch(function){
    case GL_LESS:case GL_LEQUAL:return below?1:above?-1:0;
    case GL_GREATER:case GL_GEQUAL:return above?1:below?-1:0;
    case GL_EQUAL:return below||above?-1:0;
    case GL_NOTEQUAL:return below||above?1:0;
    default:return 0;
    }
}
unsigned Compile(unsigned type,const char* source){unsigned shader=glCreateShader(type);glShaderSource(shader,1,&source,nullptr);glCompileShader(shader);int ok=0;glGetShaderiv(shader,GL_COMPILE_STATUS,&ok);if(ok)return shader;char log[2048]{};glGetShaderInfoLog(shader,sizeof(log),nullptr,log);DbgLogPublic(log);glDeleteShader(shader);return 0;}
}
namespace FragmentStage {
bool Requested(){return requested;}
const char* Status(){return status;}
void Toggle(){BufferedDraw::Suspend();requested=!requested;Report(true);RenderProfile::RestartCapture();status=requested?"Fragmentos: esperando dibujo":"Fragmentos: referencia";DbgLogPublic(requested?"Fragmentos R4b: activo":"Fragmentos R4b: referencia");}
}
namespace FragmentPipelineGL {
void CapabilityChanged(unsigned cap){if(cap==GL_TEXTURE_1D||cap==GL_TEXTURE_2D||cap==GL_TEXTURE_3D||cap==GL_TEXTURE_CUBE_MAP||cap==GL_TEXTURE_RECTANGLE)layoutChecked=false;}
void InvalidateBindings(){layoutChecked=false;targetChecked=false;}
void InvalidateTextures(){textureInfo.clear();layoutChecked=false;}
void Create(unsigned reference,const char* source){
    RenderProfile::SetBackendReport(Report);
    if(pipelines.count(reference))return;
    Pipeline p;std::string vertex(source);const auto end=vertex.find('\n');if(end==std::string::npos)return;
    vertex.insert(end+1,"#define EXPLICIT_FRAGMENT\n");
    unsigned vs=Compile(GL_VERTEX_SHADER,vertex.c_str()),fs=Compile(GL_FRAGMENT_SHADER,fragment);
    if(vs&&fs){p.program=glCreateProgram();glAttachShader(p.program,vs);glAttachShader(p.program,fs);glLinkProgram(p.program);int ok=0;glGetProgramiv(p.program,GL_LINK_STATUS,&ok);if(!ok){char log[2048]{};glGetProgramInfoLog(p.program,sizeof(log),nullptr,log);DbgLogPublic(log);glDeleteProgram(p.program);p.program=0;}}
    if(vs)glDeleteShader(vs);if(fs)glDeleteShader(fs);
    if(p.program){p.texture=glGetUniformLocation(p.program,"materialTexture");p.textured=glGetUniformLocation(p.program,"materialTextured");p.fog=glGetUniformLocation(p.program,"materialFog");p.values=glGetUniformLocation(p.program,"materialFogValues");p.color=glGetUniformLocation(p.program,"materialFogColor");p.reject=glGetUniformLocation(p.program,"materialReject");p.combine=glGetUniformLocation(p.program,"materialCombine");}
    pipelines[reference]=p;
}
void Release(unsigned reference){const auto p=pipelines.find(reference);if(p!=pipelines.end()){if(p->second.program)glDeleteProgram(p->second.program);pipelines.erase(p);}textureInfo.clear();}
Draw::Draw(unsigned reference,const float* alpha,std::size_t count,std::size_t stride):program(reference),reference(reference){
    if(!requested)return;
    const auto found=pipelines.find(reference);if(found==pipelines.end()||!found->second.program){Count(false,"Fragmentos: referencia (programa)");return;}
    if(!targetChecked){int r=0,g=0,b=0,a=0;glGetIntegerv(GL_RED_BITS,&r);glGetIntegerv(GL_GREEN_BITS,&g);glGetIntegerv(GL_BLUE_BITS,&b);glGetIntegerv(GL_ALPHA_BITS,&a);targetSupported=r==8&&g==8&&b==8&&(a==0||a==8);targetChecked=true;}
    if(!targetSupported){Count(false,"Fragmentos: referencia (framebuffer)");return;}
    int active=0;glGetIntegerv(GL_ACTIVE_TEXTURE,&active);if(active!=GL_TEXTURE0){Count(false,"Fragmentos: referencia (unidad activa)");return;}
    int shade=0,clampVertex=0;glGetIntegerv(GL_SHADE_MODEL,&shade);glGetIntegerv(GL_CLAMP_VERTEX_COLOR,&clampVertex);
    if(shade!=GL_SMOOTH||clampVertex==GL_FALSE||glIsEnabled(GL_POINT_SPRITE)){Count(false,"Fragmentos: referencia (interpolacion)");return;}
    if(!layoutChecked){int units=0;glGetIntegerv(GL_MAX_TEXTURE_UNITS,&units);
    for(int i=0;i<units;++i){glActiveTexture(GL_TEXTURE0+i);const bool other=glIsEnabled(GL_TEXTURE_1D)||glIsEnabled(GL_TEXTURE_3D)||glIsEnabled(GL_TEXTURE_CUBE_MAP)||glIsEnabled(GL_TEXTURE_RECTANGLE)||(i&&glIsEnabled(GL_TEXTURE_2D));if(other){glActiveTexture(GL_TEXTURE0);Count(false,"Fragmentos: referencia (multitextura)");return;}}
    glActiveTexture(GL_TEXTURE0);layoutChecked=true;}
    if(glIsEnabled(GL_COLOR_SUM)||glIsEnabled(GL_POLYGON_STIPPLE)){Count(false,"Fragmentos: referencia (estado especial)");return;}
    textured=glIsEnabled(GL_TEXTURE_2D)!=0;
    int environment=0;glGetTexEnviv(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,&environment);
    if(textured&&environment!=GL_MODULATE){Count(false,"Fragmentos: referencia (texture env)");return;}
    bool opaque=!textured;
    if(textured){int texture=0,sampler=0;glGetIntegerv(GL_TEXTURE_BINDING_2D,&texture);glGetIntegerv(GL_SAMPLER_BINDING,&sampler);
        const auto info=texture?InspectTexture(texture):TextureInfo{};
        if(!info.supported||sampler){Count(false,"Fragmentos: referencia (textura o sampler)");return;}opaque=info.opaque;combine=info.combine;}
    alphaEnabled=glIsEnabled(GL_ALPHA_TEST)!=0;
    int coverage=1;bool keepNativeAlpha=false;
    if(alphaEnabled){int function=0;float referenceValue=0;glGetIntegerv(GL_ALPHA_TEST_FUNC,&function);glGetFloatv(GL_ALPHA_TEST_REF,&referenceValue);
        coverage=Coverage(function,referenceValue,alpha,count,stride,opaque);
        // Cobertura variable: conservar el comparador nativo después del shader.
        // Así no alternamos de programa ni aproximamos el umbral alpha.
        if(coverage==0){alphaEnabled=false;coverage=1;keepNativeAlpha=true;}
    }
    if(glIsEnabled(GL_FOG)){int mode=0;glGetIntegerv(GL_FOG_MODE,&mode);fog=mode==GL_LINEAR?1:mode==GL_EXP?2:3;glGetFloatv(GL_FOG_START,fogValues);glGetFloatv(GL_FOG_END,fogValues+1);glGetFloatv(GL_FOG_DENSITY,fogValues+2);glGetFloatv(GL_FOG_COLOR,fogColor);
        if(fog==1&&fogValues[0]==fogValues[1]){alphaEnabled=false;Count(false,"Fragmentos: referencia (niebla degenerada)");return;}}
    program=found->second.program;modern=true;Count(true);if(keepNativeAlpha&&RenderProfile::active)++nativeAlpha;reject=coverage<0;
}
Draw::~Draw(){if(modern&&alphaEnabled)glEnable(GL_ALPHA_TEST);}
void Draw::Apply(){
    if(!modern)return;auto& p=pipelines.find(reference)->second;
    if(!p.valid)glUniform1i(p.texture,0);
    if(!p.valid||p.lastTextured!=textured)glUniform1i(p.textured,textured);
    if(!p.valid||p.lastReject!=reject)glUniform1i(p.reject,reject);
    if(!p.valid||p.lastFog!=fog)glUniform1i(p.fog,fog);
    if(!p.valid||p.lastCombine!=combine)glUniform1i(p.combine,combine);
    if(!p.valid||std::memcmp(p.lastValues,fogValues,sizeof(fogValues)))glUniform3fv(p.values,1,fogValues);
    if(!p.valid||std::memcmp(p.lastColor,fogColor,sizeof(fogColor)))glUniform4fv(p.color,1,fogColor);
    p.valid=true;p.lastTextured=textured;p.lastReject=reject;p.lastFog=fog;p.lastCombine=combine;
    std::copy(fogValues,fogValues+3,p.lastValues);std::copy(fogColor,fogColor+4,p.lastColor);
    if(alphaEnabled)glDisable(GL_ALPHA_TEST);
}
}
