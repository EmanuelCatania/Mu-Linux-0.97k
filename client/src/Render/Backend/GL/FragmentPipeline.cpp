// Sin PCH: GL y traducción del estado permanecen en el backend.
#include "Platform/GL/glad.h"
#include "Render/Backend/GL/FragmentPipeline.h"
#include "Render/RenderState.h"
#include <string>
#include <vector>
extern "C" void DbgLogPublic(const char*);

namespace FragmentPipelineGL {
const char* const VertexPrologue=R"GLSL(#version 330 core
invariant gl_Position;
uniform mat4 uModelViewProjection,uModelView,uTextureMatrix;
out vec4 vColor,vUV;
out float vFogDistance;
)GLSL";
namespace {
// DESVIACION: el pipeline fijo se reproduce en el shader. Texture env
// Blend usa la composición Modulate (el cliente no fija color de entorno).
const char* fragment=R"GLSL(#version 330 core
uniform sampler2D uTexture;
uniform bool uTextured;
uniform int uTexEnv;
uniform int uAlphaFunction;
uniform float uAlphaReference;
uniform int uFog;
uniform vec3 uFogValues;
uniform vec4 uFogColor;
uniform int uBicubic;
in vec4 vColor,vUV;
in float vFogDistance;
out vec4 fragmentColor;
// DESVIACION: filtrado bicúbico Catmull-Rom (opción de calidad de texturas).
// Lee los 16 texels con texelFetch: el 0.97k carga casi todo con filtro
// nearest y el sampler no sirve para interpolar. uBicubic: bit 1 = repetir en
// S, bit 2 = repetir en T (si no, se limita al borde).
vec4 catmullRom(vec2 uv){
    ivec2 size=textureSize(uTexture,0);
    vec2 p=uv*vec2(size)-0.5,i=floor(p),f=p-i;
    vec4 wx=vec4(f.x*(-0.5+f.x*(1.0-0.5*f.x)),1.0+f.x*f.x*(-2.5+1.5*f.x),f.x*(0.5+f.x*(2.0-1.5*f.x)),f.x*f.x*(-0.5+0.5*f.x));
    vec4 wy=vec4(f.y*(-0.5+f.y*(1.0-0.5*f.y)),1.0+f.y*f.y*(-2.5+1.5*f.y),f.y*(0.5+f.y*(2.0-1.5*f.y)),f.y*f.y*(-0.5+0.5*f.y));
    vec4 r=vec4(0.0);
    for(int y=0;y<4;++y){
        int ty=int(i.y)+y-1;
        ty=(uBicubic&4)!=0?((ty%size.y)+size.y)%size.y:clamp(ty,0,size.y-1);
        vec4 row=vec4(0.0);
        for(int x=0;x<4;++x){
            int tx=int(i.x)+x-1;
            tx=(uBicubic&2)!=0?((tx%size.x)+size.x)%size.x:clamp(tx,0,size.x-1);
            row+=texelFetch(uTexture,ivec2(tx,ty),0)*wx[x];
        }
        r+=row*wy[y];
    }
    return clamp(r,0.0,1.0);
}
void main(){
    vec4 c=vColor;
    if(uTextured){
        vec2 uv=vUV.xy/vUV.w;
        vec4 t=texture(uTexture,uv);
        if(uBicubic!=0){
            // Sólo al ampliar: al reducir, el muestreo normal (con mipmaps) es mejor.
            // Texels por píxel en el eje mayor (como el LOD del hardware, no fwidth).
            vec2 st=uv*vec2(textureSize(uTexture,0));
            if(max(length(dFdx(st)),length(dFdy(st)))<1.0)t=catmullRom(uv);
        }
        if(uTexEnv==1)c=t;
        else if(uTexEnv==2)c=vec4(mix(vColor.rgb,t.rgb,t.a),vColor.a);
        else if(uTexEnv==4)c=vec4(vColor.rgb+t.rgb,vColor.a*t.a);
        else c=t*vColor;
    }
    if(uAlphaFunction!=0){
        float a=c.a,r=uAlphaReference;int f=uAlphaFunction;
        bool pass=f==1?false:f==2?a<r:f==3?a==r:f==4?a<=r:f==5?a>r:f==6?a!=r:f==7?a>=r:true;
        if(!pass)discard;
    }
    if(uFog!=0){
        float d=vFogDistance,f=1;
        if(uFog==1)f=uFogValues.y==uFogValues.x?(d<=uFogValues.y?1.0:0.0):(uFogValues.y-d)/(uFogValues.y-uFogValues.x);
        if(uFog==2)f=exp(-uFogValues.z*d);
        if(uFog==3){float v=uFogValues.z*d;f=exp(-v*v);}
        c.rgb=mix(uFogColor.rgb,c.rgb,clamp(f,0.0,1.0));
    }
    fragmentColor=c;
}
)GLSL";
struct Program {
    GLuint name=0;
    GLint mvp=-1,view=-1,texture=-1,sampler=-1,textured=-1,environment=-1,alphaFunction=-1,alphaReference=-1,fog=-1,fogValues=-1,fogColor=-1,bicubic=-1;
    unsigned long long matrices=0,material=0;
    bool sampler0=false;
};
std::vector<Program> programs;
Program* Find(unsigned name) { for(auto& p:programs)if(p.name==name)return &p;return nullptr; }
GLuint Compile(GLenum type,const std::string& source) {
    const GLuint shader=glCreateShader(type);const char* text=source.c_str();
    glShaderSource(shader,1,&text,nullptr);glCompileShader(shader);
    GLint ok=0;glGetShaderiv(shader,GL_COMPILE_STATUS,&ok);if(ok)return shader;
    char log[2048]{};glGetShaderInfoLog(shader,sizeof(log),nullptr,log);DbgLogPublic(log);glDeleteShader(shader);return 0;
}
int AlphaCode(Gfx::Compare c) {
    switch(c){
    case Gfx::Compare::Never:return 1;case Gfx::Compare::Less:return 2;case Gfx::Compare::Equal:return 3;
    case Gfx::Compare::LessEqual:return 4;case Gfx::Compare::Greater:return 5;case Gfx::Compare::NotEqual:return 6;
    case Gfx::Compare::GreaterEqual:return 7;case Gfx::Compare::Always:return 0;
    }
    return 0;
}
int EnvCode(Gfx::TexEnv e) {
    switch(e){case Gfx::TexEnv::Replace:return 1;case Gfx::TexEnv::Decal:return 2;case Gfx::TexEnv::Add:return 4;default:return 0;}
}
}
unsigned Create(const char* vertexBody) {
    const GLuint vs=Compile(GL_VERTEX_SHADER,std::string(VertexPrologue)+vertexBody),fs=Compile(GL_FRAGMENT_SHADER,fragment);
    GLuint name=0;
    if(vs&&fs) {
        name=glCreateProgram();glAttachShader(name,vs);glAttachShader(name,fs);
        glBindAttribLocation(name,0,"aPosition");glBindAttribLocation(name,1,"aColor");glBindAttribLocation(name,2,"aUV");
        glLinkProgram(name);GLint ok=0;glGetProgramiv(name,GL_LINK_STATUS,&ok);
        if(!ok){char log[2048]{};glGetProgramInfoLog(name,sizeof(log),nullptr,log);DbgLogPublic(log);glDeleteProgram(name);name=0;}
    }
    if(vs)glDeleteShader(vs);if(fs)glDeleteShader(fs);
    if(!name)return 0;
    Program p;p.name=name;
    p.mvp=glGetUniformLocation(name,"uModelViewProjection");p.view=glGetUniformLocation(name,"uModelView");p.texture=glGetUniformLocation(name,"uTextureMatrix");
    p.sampler=glGetUniformLocation(name,"uTexture");p.textured=glGetUniformLocation(name,"uTextured");p.environment=glGetUniformLocation(name,"uTexEnv");
    p.alphaFunction=glGetUniformLocation(name,"uAlphaFunction");p.alphaReference=glGetUniformLocation(name,"uAlphaReference");
    p.fog=glGetUniformLocation(name,"uFog");p.fogValues=glGetUniformLocation(name,"uFogValues");p.fogColor=glGetUniformLocation(name,"uFogColor");
    p.bicubic=glGetUniformLocation(name,"uBicubic");
    programs.push_back(p);return name;
}
void Release(unsigned name) {
    for(auto it=programs.begin();it!=programs.end();++it)if(it->name==name){programs.erase(it);break;}
    if(!name)return;
    GLint current=0;glGetIntegerv(GL_CURRENT_PROGRAM,&current);
    if(static_cast<unsigned>(current)==name)glUseProgram(0);
    glDeleteProgram(name);
}
int Uniform(unsigned program,const char* name) { return glGetUniformLocation(program,name); }
// Samplers trilineales con ampliación lineal, por combinación de wrap.
GLuint SmoothSampler(bool repeatS,bool repeatT) {
    static GLuint samplers[4]{};
    GLuint& sampler=samplers[(repeatS?1:0)|(repeatT?2:0)];
    if(!sampler) {
        glGenSamplers(1,&sampler);
        glSamplerParameteri(sampler,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
        glSamplerParameteri(sampler,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glSamplerParameteri(sampler,GL_TEXTURE_WRAP_S,repeatS?GL_REPEAT:GL_CLAMP_TO_EDGE);
        glSamplerParameteri(sampler,GL_TEXTURE_WRAP_T,repeatT?GL_REPEAT:GL_CLAMP_TO_EDGE);
        GLfloat anisotropy=0;glGetFloatv(0x84FF,&anisotropy);   // GL_MAX_TEXTURE_MAX_ANISOTROPY
        while(glGetError()!=GL_NO_ERROR){}
        if(anisotropy>1)glSamplerParameterf(sampler,0x84FE,anisotropy<16?anisotropy:16);
    }
    return sampler;
}
void Apply(unsigned name) {
    Program* p=Find(name);if(!p)return;
    if(!p->sampler0) { glUniform1i(p->sampler,0);p->sampler0=true; }
    if(p->matrices!=RenderState::MatrixRevision()) {
        glUniformMatrix4fv(p->mvp,1,GL_FALSE,RenderState::ModelViewProjection());
        glUniformMatrix4fv(p->view,1,GL_FALSE,RenderState::Top(Gfx::Matrix::ModelView));
        glUniformMatrix4fv(p->texture,1,GL_FALSE,RenderState::Top(Gfx::Matrix::Texture));
        p->matrices=RenderState::MatrixRevision();
    }
    if(p->material!=RenderState::MaterialRevision()) {
        const auto& s=RenderState::Get();
        glUniform1i(p->textured,s.caps[RenderState::CapIndex(Gfx::Cap::Texture2D)]?1:0);
        // Sólo lo 3D (depth test): el texto y la interfaz se dibujan sin él.
        const bool depth=s.caps[RenderState::CapIndex(Gfx::Cap::DepthTest)];
        const bool bicubic=Gfx::TextureBicubic() && depth;
        glUniform1i(p->bicubic,bicubic?1|(s.textureRepeat[0]?2:0)|(s.textureRepeat[1]?4:0):0);
        // Con mipmaps, lo 3D amplía con filtro lineal aunque la textura sea
        // nítida (sampler propio); la interfaz conserva el filtro original.
        glBindSampler(0,Gfx::TextureMipmaps() && depth?SmoothSampler(s.textureRepeat[0],s.textureRepeat[1]):0);
        glUniform1i(p->environment,EnvCode(s.texEnv));
        glUniform1i(p->alphaFunction,s.caps[RenderState::CapIndex(Gfx::Cap::AlphaTest)]?AlphaCode(s.alphaFunction):0);
        glUniform1f(p->alphaReference,s.alphaReference);
        const int fog=!s.caps[RenderState::CapIndex(Gfx::Cap::Fog)]?0:s.fogMode==Gfx::FogMode::Linear?1:s.fogMode==Gfx::FogMode::Exp?2:3;
        glUniform1i(p->fog,fog);
        glUniform3f(p->fogValues,s.fogStart,s.fogEnd,s.fogDensity);glUniform4fv(p->fogColor,1,s.fogColor);
        p->material=RenderState::MaterialRevision();
    }
}
}
