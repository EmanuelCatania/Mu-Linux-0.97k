// Sin PCH: las funciones modernas se resuelven con glad, no GL/gl.h.
#include "Platform/GL/glad.h"
#include "Render/Backend/GL/ProfileUploads.h"
#undef glTexImage2D
#define glTexImage2D(...) ::ProfileUploadGL::Image2D(glad_glTexImage2D,__VA_ARGS__)
#undef glTexSubImage2D
#define glTexSubImage2D(...) ::ProfileUploadGL::Sub2D(glad_glTexSubImage2D,__VA_ARGS__)
#undef glTexImage3D
#define glTexImage3D(...) ::ProfileUploadGL::Image3D(glad_glTexImage3D,__VA_ARGS__)
#undef glTexSubImage3D
#define glTexSubImage3D(...) ::ProfileUploadGL::Sub3D(glad_glTexSubImage3D,__VA_ARGS__)
#undef glBufferData
#define glBufferData(...) ::ProfileUploadGL::BufferData(glad_glBufferData,__VA_ARGS__)
#undef glBufferSubData
#define glBufferSubData(...) ::ProfileUploadGL::BufferSubData(glad_glBufferSubData,__VA_ARGS__)

#include "Platform/GLFunctions.h"
#include "Render/BufferedDraw.h"
#include "Render/Backend/GL/FragmentPipeline.h"
#include "Render/RenderProfile.h"
#include "Render/Backend/GL/ProfileQueries.h"
#include <cstddef>
#include <cstdio>
#include <algorithm>

extern "C" void DbgLogPublic(const char*);
namespace BufferedDraw {
Stream stream;
namespace {
GLuint program=0,buffer=0,array=0,quadBuffer=0;
std::size_t quadCapacity=0;
std::vector<Vertex> pending;
unsigned pendingMode=0;
bool bound=false;
void (*exitHook)()=nullptr;
GLuint boundProgram=0,boundArray=0,boundBuffer=0;
GLint savedProgram=0,savedArray=0,savedBuffer=0;
// DESVIACION: compartir la propiedad del pipeline entre geometría general
// y mallas indexadas. Sólo Suspend devuelve los bindings del caller.
void BindPipeline(GLuint nextProgram,GLuint nextArray,GLuint nextBuffer) {
    if (!bound) {
        glGetIntegerv(GL_CURRENT_PROGRAM,&savedProgram);glGetIntegerv(GL_VERTEX_ARRAY_BINDING,&savedArray);
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING,&savedBuffer);
        bound=true;boundProgram=boundArray=boundBuffer=0;
    }
    if (boundProgram!=nextProgram) { RenderProfileGL::ObserveProgram(nextProgram);glUseProgram(nextProgram);boundProgram=nextProgram; }
    if (boundArray!=nextArray) { glBindVertexArray(nextArray);boundArray=nextArray; }
    if (boundBuffer!=nextBuffer) { glBindBuffer(GL_ARRAY_BUFFER,nextBuffer);boundBuffer=nextBuffer; }
}
// DESVIACION: cada abanico de cuatro esquinas conserva los triángulos
// 0-1-2 y 0-2-3 sin duplicar sus vértices en el buffer de envío.
void EnsureQuadIndices(std::size_t count) {
    if (quadCapacity>=count) return;
    quadCapacity=std::max(count,std::max(std::size_t(256),quadCapacity*2));
    std::vector<unsigned> indices;indices.reserve(quadCapacity*6);
    for (std::size_t q=0;q<quadCapacity;++q) {
        const unsigned v=static_cast<unsigned>(q*4);
        indices.insert(indices.end(),{v,v+1,v+2,v,v+2,v+3});
    }
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,indices.size()*sizeof(unsigned),indices.data(),GL_STATIC_DRAW);
}
// DESVIACION R4b: la referencia conserva sus fragmentos compatibles. La
// variante EXPLICIT_FRAGMENT limita color primario en el vértice y compone
// textura/niebla/alpha en el backend, con respaldo para estados no admitidos.
const char* vertexSource=R"GLSL(#version 330 compatibility
void main() {
    gl_Position=ftransform();
#ifdef EXPLICIT_FRAGMENT
    gl_FrontColor=clamp(gl_Color,0,1);gl_BackColor=clamp(gl_Color,0,1);
#else
    gl_FrontColor=gl_Color;gl_BackColor=gl_Color;
#endif
    gl_TexCoord[0]=gl_TextureMatrix[0]*gl_MultiTexCoord0;
    vec4 eye=gl_ModelViewMatrix*gl_Vertex;
    gl_FogFragCoord=abs(eye.z);
    gl_ClipVertex=eye;
}
)GLSL";
void ShaderLog(GLuint object,bool link) {
    char text[2048]{};
    if (link) glGetProgramInfoLog(object,sizeof(text),nullptr,text);
    else glGetShaderInfoLog(object,sizeof(text),nullptr,text);
    DbgLogPublic(text);
}
}
bool Initialize() {
    if (program) return true;
    if (!Platform::GL33Ready()) return false;
    const GLuint shader=glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shader,1,&vertexSource,nullptr);glCompileShader(shader);
    GLint ok=0;glGetShaderiv(shader,GL_COMPILE_STATUS,&ok);
    if (!ok) { ShaderLog(shader,false);glDeleteShader(shader);return false; }
    program=glCreateProgram();glAttachShader(program,shader);glLinkProgram(program);
    glDeleteShader(shader);glGetProgramiv(program,GL_LINK_STATUS,&ok);
    if (!ok) { ShaderLog(program,true);Release();return false; }
    FragmentPipelineGL::Create(program,vertexSource);
    GLint oldArray=0,oldBuffer=0,oldClient=0;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING,&oldArray);glGetIntegerv(GL_ARRAY_BUFFER_BINDING,&oldBuffer);
    glGetIntegerv(GL_CLIENT_ACTIVE_TEXTURE,&oldClient);
    glGenBuffers(1,&buffer);glGenBuffers(1,&quadBuffer);glGenVertexArrays(1,&array);
    glBindVertexArray(array);glBindBuffer(GL_ARRAY_BUFFER,buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,quadBuffer);
    glEnableClientState(GL_VERTEX_ARRAY);glVertexPointer(3,GL_FLOAT,sizeof(Vertex),(void*)offsetof(Vertex,position));
    glEnableVertexAttribArray(0);glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,position));
    glEnableVertexAttribArray(1);glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,color));
    glEnableVertexAttribArray(2);glVertexAttribPointer(2,4,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,uv));
    glEnableClientState(GL_COLOR_ARRAY);glColorPointer(4,GL_FLOAT,sizeof(Vertex),(void*)offsetof(Vertex,color));
    glClientActiveTexture(GL_TEXTURE0);glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(4,GL_FLOAT,sizeof(Vertex),(void*)offsetof(Vertex,uv));
    glClientActiveTexture(oldClient);glBindVertexArray(oldArray);glBindBuffer(GL_ARRAY_BUFFER,oldBuffer);
    if (!buffer || !quadBuffer || !array || glGetError()!=GL_NO_ERROR) { Release();return false; }
    return true;
}
void Release() {
    Suspend();
    stream.active=false;stream.vertices.clear();pending.clear();
    if (array) glDeleteVertexArrays(1,&array);
    if (buffer) glDeleteBuffers(1,&buffer);
    if (quadBuffer) glDeleteBuffers(1,&quadBuffer);
    FragmentPipelineGL::Release(program);
    if (program) glDeleteProgram(program);
    array=buffer=program=quadBuffer=0;quadCapacity=0;
}
void NativeColor(float r,float g,float b,float a) { glColor4f(r,g,b,a); }
void NativeUV(float u,float v) { glTexCoord2f(u,v); }
void NativeVertex(float x,float y,float z) { glVertex3f(x,y,z); }
Vertex CaptureAttributes(bool color,bool uv){
    Vertex attributes{{},{1,1,1,1},{0,0,0,1}};
    if(color)glGetFloatv(GL_CURRENT_COLOR,attributes.color);
    if(uv){int active=0;glGetIntegerv(GL_ACTIVE_TEXTURE,&active);if(active!=GL_TEXTURE0)glActiveTexture(GL_TEXTURE0);glGetFloatv(GL_CURRENT_TEXTURE_COORDS,attributes.uv);if(active!=GL_TEXTURE0)glActiveTexture(active);}
    return attributes;
}
void Begin(Primitive primitive){
    const unsigned modes[]{GL_POINTS,GL_LINES,GL_TRIANGLES,GL_QUADS,GL_TRIANGLE_FAN};
    Begin(modes[static_cast<unsigned>(primitive)]);
}
void Begin(unsigned mode) {
    if (!program) { glBegin(mode);return; }
    stream.mode=mode;stream.vertices.clear();
    // Capturar una vez antes del posible envío: Flush restituye estos mismos
    // atributos, por lo que no necesita volver a leerlos del driver.
    glGetFloatv(GL_CURRENT_COLOR,stream.color);
    GLint active=0;glGetIntegerv(GL_ACTIVE_TEXTURE,&active);
    if (active!=GL_TEXTURE0) glActiveTexture(GL_TEXTURE0);
    glGetFloatv(GL_CURRENT_TEXTURE_COORDS,stream.uv);
    if (active!=GL_TEXTURE0) glActiveTexture(active);
    const Vertex current{{},{stream.color[0],stream.color[1],stream.color[2],stream.color[3]},
        {stream.uv[0],stream.uv[1],stream.uv[2],stream.uv[3]}};
    const bool direct=mode==GL_TRIANGLES || mode==GL_QUADS || mode==GL_LINES || mode==GL_POINTS;
    if (direct && !pending.empty() && pendingMode!=mode) Flush(&current);
    if (direct) pendingMode=mode;
    stream.destination=direct ? &pending : &stream.vertices;
    stream.first=stream.destination->size();
    stream.active=true;
}
// DESVIACION: el terreno ya entrega todos los atributos; evitar consultas
// de color/UV heredados y la captura individual de cada esquina.
void Quad(const Vertex* corners,bool fan) {
    const unsigned mode=fan ? GL_TRIANGLE_FAN : GL_QUADS;
    if (!program) {
        glBegin(mode);
        for (int i=0;i<4;++i) {
            glColor4fv(corners[i].color);glTexCoord4fv(corners[i].uv);glVertex3fv(corners[i].position);
        }
        glEnd();return;
    }
    if (!pending.empty() && pendingMode!=mode) Flush();
    pendingMode=mode;pending.insert(pending.end(),corners,corners+4);
    glColor4fv(corners[3].color);glTexCoord4fv(corners[3].uv);
}
void Flush(const Vertex* current) {
    if (pending.empty()) return;
    float color[4],uv[4];
    if(current) {
        std::copy(current->color,current->color+4,color);std::copy(current->uv,current->uv+4,uv);
    } else {
        GLint active=0;
        glGetFloatv(GL_CURRENT_COLOR,color);glGetIntegerv(GL_ACTIVE_TEXTURE,&active);
        if (active!=GL_TEXTURE0) glActiveTexture(GL_TEXTURE0);
        glGetFloatv(GL_CURRENT_TEXTURE_COORDS,uv);
        if (active!=GL_TEXTURE0) glActiveTexture(active);
    }
    FragmentPipelineGL::Draw material(program,&pending.front().color[3],pending.size(),sizeof(Vertex));
    BindPipeline(material.Program(),array,buffer);material.Apply();
    glBufferData(GL_ARRAY_BUFFER,pending.size()*sizeof(Vertex),pending.data(),GL_STREAM_DRAW);
    RenderProfile::Draw(pendingMode==GL_TRIANGLE_FAN ? pending.size()/4*6 : pending.size());
    if (pendingMode==GL_TRIANGLE_FAN) {
        EnsureQuadIndices(pending.size()/4);
        glDrawElements(GL_TRIANGLES,static_cast<GLsizei>(pending.size()/4*6),GL_UNSIGNED_INT,nullptr);
    } else glDrawArrays(pendingMode,0,(GLsizei)pending.size());
    // Los atributos actuales pueden quedar indefinidos tras DrawArrays.
    glColor4fv(color);glTexCoord4fv(uv);
    pending.clear();
}
void SetExitHook(void (*hook)()) { exitHook=hook; }
void Suspend() {
    Flush();FragmentPipelineGL::InvalidateBindings();
    if(exitHook) { auto hook=exitHook;exitHook=nullptr;hook(); }
    if (!bound) return;
    glBindVertexArray(savedArray);glBindBuffer(GL_ARRAY_BUFFER,savedBuffer);RenderProfileGL::ObserveProgram(savedProgram);glUseProgram(savedProgram);
    bound=false;boundProgram=boundArray=boundBuffer=0;
}
// DESVIACION: compartir la propiedad también con el shader de skinning.
void Activate(unsigned shader,unsigned nextArray,unsigned nextBuffer) {
    Flush();BindPipeline(shader,nextArray,nextBuffer);
}
void DrawIndexed(unsigned meshArray,unsigned meshBuffer,const Vertex* vertices,std::size_t count,std::size_t indices,const Vertex* current) {
    // DESVIACION: MeshDraw ya fijó estos atributos; repetir sus stores conserva
    // incluso las conversiones nativas sin volver a consultar el driver.
    Flush(current);
    float color[4],uv[4];
    if(current) {
        std::copy(current->color,current->color+4,color);std::copy(current->uv,current->uv+4,uv);
    } else {
        GLint active;
        glGetFloatv(GL_CURRENT_COLOR,color);glGetIntegerv(GL_ACTIVE_TEXTURE,&active);
        if(active!=GL_TEXTURE0) glActiveTexture(GL_TEXTURE0);
        glGetFloatv(GL_CURRENT_TEXTURE_COORDS,uv);
        if(active!=GL_TEXTURE0) glActiveTexture(active);
    }
    FragmentPipelineGL::Draw material(program,&vertices[0].color[3],count,sizeof(Vertex),true);
    BindPipeline(material.Program(),meshArray,meshBuffer);material.Apply();
    glBufferData(GL_ARRAY_BUFFER,count*sizeof(Vertex),vertices,GL_STREAM_DRAW);
    RenderProfile::Draw(indices);
    glDrawElements(GL_TRIANGLES,static_cast<GLsizei>(indices),GL_UNSIGNED_INT,nullptr);
    glColor4fv(color);glTexCoord4fv(uv);
}

void End() {
    if (!stream.active) { glEnd();return; }
    stream.active=false;
    const bool direct=stream.destination==&pending;
    const auto count=stream.destination->size()-stream.first;
    const bool fan=stream.mode==GL_TRIANGLE_FAN;
    const bool quad=fan && count==4;
    const unsigned mode=quad ? GL_TRIANGLE_FAN : fan ? GL_TRIANGLES : stream.mode;
    const unsigned group=mode==GL_TRIANGLES?3:mode==GL_QUADS?4:mode==GL_LINES?2:1;
    const bool independent=quad || mode==GL_TRIANGLES || mode==GL_QUADS || mode==GL_LINES || mode==GL_POINTS;
    if (!direct && !pending.empty() && (pendingMode!=mode || !independent)) Flush();
    pendingMode=mode;
    if (quad) {
        pending.insert(pending.end(),stream.vertices.begin(),stream.vertices.end());
    } else if (fan) {
        // Cada abanico conserva sus triángulos y su orden. No se conectan
        // abanicos de tiles distintos al reunirlos en un mismo envío.
        for (size_t i=1;i+1<count;++i) {
            pending.push_back(stream.vertices[0]);pending.push_back(stream.vertices[i]);pending.push_back(stream.vertices[i+1]);
        }
    } else if (direct) {
        pending.resize(pending.size()-count%group);
    } else {
        // OpenGL descarta los vértices incompletos al cerrar una primitiva.
        // Su color/UV actual sí se conserva aunque no formen una cara.
        const auto complete=independent ? count-count%group : count;
        pending.insert(pending.end(),stream.vertices.begin(),stream.vertices.begin()+complete);
    }
    glColor4fv(stream.color);glTexCoord4fv(stream.uv);
    // Strips/polígonos no se concatenan: conservar sus conexiones originales.
    if (!independent) {
        const Vertex current{{},{stream.color[0],stream.color[1],stream.color[2],stream.color[3]},
            {stream.uv[0],stream.uv[1],stream.uv[2],stream.uv[3]}};
        Flush(&current);
    }
}
}
