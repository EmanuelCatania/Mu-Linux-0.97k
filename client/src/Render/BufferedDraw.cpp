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
    stream.active=false;stream.vertices.clear();
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
    glGetFloatv(GL_CURRENT_COLOR,stream.color);
    GLint active=0;glGetIntegerv(GL_ACTIVE_TEXTURE,&active);
    if (active!=GL_TEXTURE0) glActiveTexture(GL_TEXTURE0);
    glGetFloatv(GL_CURRENT_TEXTURE_COORDS,stream.uv);
    if (active!=GL_TEXTURE0) glActiveTexture(active);
    stream.active=true;
}
void End() {
    if (!stream.active) { glEnd();return; }
    stream.active=false;
    if (!stream.vertices.empty()) {
        GLint oldProgram=0,oldArray=0,oldBuffer=0;
        glGetIntegerv(GL_CURRENT_PROGRAM,&oldProgram);glGetIntegerv(GL_VERTEX_ARRAY_BINDING,&oldArray);
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING,&oldBuffer);
        glUseProgram(program);glBindVertexArray(array);glBindBuffer(GL_ARRAY_BUFFER,buffer);
        glBufferData(GL_ARRAY_BUFFER,stream.vertices.size()*sizeof(Vertex),stream.vertices.data(),GL_STREAM_DRAW);
        glDrawArrays(stream.mode,0,(GLsizei)stream.vertices.size());
        glBindVertexArray(oldArray);glBindBuffer(GL_ARRAY_BUFFER,oldBuffer);glUseProgram(oldProgram);
    }
    // DrawArrays deja atributos actuales indefinidos si sus arrays están
    // activos. Reponer los últimos emitidos conserva los draws y textos que
    // heredan color/UV sin volver a establecerlos.
    glColor4fv(stream.color);glTexCoord4fv(stream.uv);
}
}
