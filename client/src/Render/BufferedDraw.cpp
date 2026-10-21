// Sin PCH: las funciones modernas se resuelven con glad, no GL/gl.h.
#include "Platform/GL/glad.h"
#include "Platform/GLFunctions.h"
#include "Render/BufferedDraw.h"
#include <cstddef>
#include <cstdio>

extern "C" void DbgLogPublic(const char*);
namespace BufferedDraw {
Stream stream;
namespace {
GLuint program=0,buffer=0,array=0;
std::vector<Vertex> pending;
unsigned pendingMode=0;
bool bound=false;
GLint savedProgram=0,savedArray=0,savedBuffer=0;
// ftransform conserva la transformación del pipeline compatible. La etapa
// fragmentaria sigue siendo la compatible: textura, alpha test y niebla no
// se sustituyen por aproximaciones ni cambian sus setters/cache heredados.
const char* vertexSource=R"GLSL(#version 330 compatibility
void main() {
    gl_Position=ftransform();
    gl_FrontColor=gl_Color;
    gl_BackColor=gl_Color;
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
    GLint oldArray=0,oldBuffer=0,oldClient=0;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING,&oldArray);glGetIntegerv(GL_ARRAY_BUFFER_BINDING,&oldBuffer);
    glGetIntegerv(GL_CLIENT_ACTIVE_TEXTURE,&oldClient);
    glGenBuffers(1,&buffer);glGenVertexArrays(1,&array);
    glBindVertexArray(array);glBindBuffer(GL_ARRAY_BUFFER,buffer);
    glEnableClientState(GL_VERTEX_ARRAY);glVertexPointer(3,GL_FLOAT,sizeof(Vertex),(void*)offsetof(Vertex,position));
    glEnableClientState(GL_COLOR_ARRAY);glColorPointer(4,GL_FLOAT,sizeof(Vertex),(void*)offsetof(Vertex,color));
    glClientActiveTexture(GL_TEXTURE0);glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(4,GL_FLOAT,sizeof(Vertex),(void*)offsetof(Vertex,uv));
    glClientActiveTexture(oldClient);glBindVertexArray(oldArray);glBindBuffer(GL_ARRAY_BUFFER,oldBuffer);
    if (!buffer || !array || glGetError()!=GL_NO_ERROR) { Release();return false; }
    return true;
}
void Release() {
    Suspend();
    stream.active=false;stream.vertices.clear();pending.clear();
    if (array) glDeleteVertexArrays(1,&array);
    if (buffer) glDeleteBuffers(1,&buffer);
    if (program) glDeleteProgram(program);
    array=buffer=program=0;
}
void NativeColor(float r,float g,float b,float a) { glColor4f(r,g,b,a); }
void NativeUV(float u,float v) { glTexCoord2f(u,v); }
void NativeVertex(float x,float y,float z) { glVertex3f(x,y,z); }
void Begin(unsigned mode) {
    if (!program) { glBegin(mode);return; }
    stream.mode=mode;stream.vertices.clear();
    const bool direct=mode==GL_TRIANGLES || mode==GL_QUADS || mode==GL_LINES || mode==GL_POINTS;
    if (direct && !pending.empty() && pendingMode!=mode) Flush();
    if (direct) pendingMode=mode;
    stream.destination=direct ? &pending : &stream.vertices;
    stream.first=stream.destination->size();
    glGetFloatv(GL_CURRENT_COLOR,stream.color);
    GLint active=0;glGetIntegerv(GL_ACTIVE_TEXTURE,&active);
    if (active!=GL_TEXTURE0) glActiveTexture(GL_TEXTURE0);
    glGetFloatv(GL_CURRENT_TEXTURE_COORDS,stream.uv);
    if (active!=GL_TEXTURE0) glActiveTexture(active);
    stream.active=true;
}
void Flush() {
    if (pending.empty()) return;
    GLint active=0;
    float color[4],uv[4];
    glGetFloatv(GL_CURRENT_COLOR,color);glGetIntegerv(GL_ACTIVE_TEXTURE,&active);
    if (active!=GL_TEXTURE0) glActiveTexture(GL_TEXTURE0);
    glGetFloatv(GL_CURRENT_TEXTURE_COORDS,uv);
    if (active!=GL_TEXTURE0) glActiveTexture(active);
    // DESVIACION: mantener el pipeline durante envíos consecutivos. Cambiar
    // texturas/matrices no exige volver al programa fijo entre cada primitiva.
    if (!bound) {
        glGetIntegerv(GL_CURRENT_PROGRAM,&savedProgram);glGetIntegerv(GL_VERTEX_ARRAY_BINDING,&savedArray);
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING,&savedBuffer);
        glUseProgram(program);glBindVertexArray(array);glBindBuffer(GL_ARRAY_BUFFER,buffer);bound=true;
    }
    glBufferData(GL_ARRAY_BUFFER,pending.size()*sizeof(Vertex),pending.data(),GL_STREAM_DRAW);
    glDrawArrays(pendingMode,0,(GLsizei)pending.size());
    // Los atributos actuales pueden quedar indefinidos tras DrawArrays.
    glColor4fv(color);glTexCoord4fv(uv);
    pending.clear();
}
void Suspend() {
    Flush();
    if (!bound) return;
    glBindVertexArray(savedArray);glBindBuffer(GL_ARRAY_BUFFER,savedBuffer);glUseProgram(savedProgram);
    bound=false;
}
void DrawIndexed(unsigned meshArray,unsigned meshBuffer,const Vertex* vertices,std::size_t count,std::size_t indices) {
    Suspend();
    GLint oldProgram,oldArray,oldBuffer,active;
    float color[4],uv[4];glGetFloatv(GL_CURRENT_COLOR,color);glGetIntegerv(GL_ACTIVE_TEXTURE,&active);
    glActiveTexture(GL_TEXTURE0);glGetFloatv(GL_CURRENT_TEXTURE_COORDS,uv);glActiveTexture(active);
    glGetIntegerv(GL_CURRENT_PROGRAM,&oldProgram);glGetIntegerv(GL_VERTEX_ARRAY_BINDING,&oldArray);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING,&oldBuffer);
    glUseProgram(program);glBindVertexArray(meshArray);glBindBuffer(GL_ARRAY_BUFFER,meshBuffer);
    glBufferData(GL_ARRAY_BUFFER,count*sizeof(Vertex),vertices,GL_STREAM_DRAW);
    glDrawElements(GL_TRIANGLES,static_cast<GLsizei>(indices),GL_UNSIGNED_INT,nullptr);
    glBindVertexArray(oldArray);glBindBuffer(GL_ARRAY_BUFFER,oldBuffer);glUseProgram(oldProgram);
    glColor4fv(color);glTexCoord4fv(uv);
}
void End() {
    if (!stream.active) { glEnd();return; }
    stream.active=false;
    const bool direct=stream.destination==&pending;
    const auto count=stream.destination->size()-stream.first;
    const bool fan=stream.mode==GL_TRIANGLE_FAN;
    const unsigned mode=fan ? GL_TRIANGLES : stream.mode;
    const unsigned group=mode==GL_TRIANGLES?3:mode==GL_QUADS?4:mode==GL_LINES?2:1;
    const bool independent=mode==GL_TRIANGLES || mode==GL_QUADS || mode==GL_LINES || mode==GL_POINTS;
    if (!direct && !pending.empty() && (pendingMode!=mode || !independent)) Flush();
    pendingMode=mode;
    if (fan) {
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
    if (!independent) Flush();
}
}
