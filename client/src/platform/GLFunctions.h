#pragma once

namespace Platform {
using GLProc = void (*)();
using GLResolver = GLProc (*)(const char*);
// DESVIACION: funciones modernas para el renderer; requiere contexto actual.
// No modifica matrices, texturas, blend ni ningún otro estado de dibujo.
bool LoadGL33(GLResolver resolver);
void ResetGL33();
bool GL33Ready();
const char* GL33Error();
}
