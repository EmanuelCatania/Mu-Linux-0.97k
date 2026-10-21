// Sin stdafx: glad debe preceder a cualquier header OpenGL del sistema.
#define GLAD_GL_IMPLEMENTATION
#include "Platform/GL/glad.h"
#include "Platform/GLFunctions.h"

namespace Platform {
namespace {
bool ready=false;
const char* error="OpenGL 3.3 compatibility no inicializado";
}
void ResetGL33() { ready=false; }
bool GL33Ready() { return ready; }
const char* GL33Error() { return error; }
bool LoadGL33(GLResolver resolver) {
    ResetGL33();
    error="No se pudo cargar OpenGL 3.3";
    if (!resolver || !gladLoadGL(resolver) || !GLAD_GL_VERSION_3_3 || !glGetIntegerv) return false;
    GLint profile=0,flags=0;
    glGetIntegerv(GL_CONTEXT_PROFILE_MASK,&profile);
    glGetIntegerv(GL_CONTEXT_FLAGS,&flags);
    error="Se requiere OpenGL 3.3 compatibility sin forward-compatible";
    if (!(profile & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT) || (flags & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT)) return false;
    // También validar las entradas que necesitarán buffers, VAO y shaders;
    // una versión publicada no sustituye a un puntero disponible del driver.
    error="El controlador no ofrece las funciones OpenGL requeridas";
    if (!glBegin || !glEnd || !glMatrixMode || !glGenLists || !glGetError ||
        !glGenBuffers || !glBindBuffer || !glBufferData || !glBufferSubData || !glDeleteBuffers ||
        !glGenVertexArrays || !glBindVertexArray || !glDeleteVertexArrays ||
        !glCreateShader || !glShaderSource || !glCompileShader || !glGetShaderiv || !glGetShaderInfoLog || !glDeleteShader ||
        !glCreateProgram || !glAttachShader || !glLinkProgram || !glGetProgramiv || !glGetProgramInfoLog || !glUseProgram || !glDeleteProgram ||
        !glGetUniformLocation || !glUniformMatrix4fv || !glUniform1i ||
        !glEnableVertexAttribArray || !glVertexAttribPointer || !glDrawElements || !glDrawArrays) return false;
    ready=true; error="";
    return true;
}
}
