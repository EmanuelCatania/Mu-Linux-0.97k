// Render_MatrixStack.cpp

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "Net/MuEmu.h"
#include "Net/Net.h"
#include "Render/Camera.h"

// ── GL_PopMatrixAll ───────────────────────────────────────────────────────────
// Fuerza el reset completo de los stacks PROJECTION y MODELVIEW a identidad:
// GL_BeginViewport (GL_SetupView) pushea DOS matrices (PROJECTION + MODELVIEW)
// y Scene_Login sólo hace un glPopMatrix antes de Begin2D; con un solo pop
// quedaría un push acumulado por frame (overflow → matrices corruptas → UI 2D
// invisible). Los glGetError() limpian el GL_STACK_UNDERFLOW que generan los
// pops sobrantes (son inocuos, sólo setean el flag de error).
unsigned int __cdecl GL_PopMatrixAll(void) {
    glMatrixMode(GL_PROJECTION);
    for (int i = 0; i < 8; ++i) glPopMatrix();
    // Drain ALL pending error flags (multiple underflows can stack).
    // Single glGetError() solo limpia uno; el resto persiste y aparece
    // como 0x504 en cada GL_DisableDepthTest next frame.
    while (glGetError() != GL_NO_ERROR) {}
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    for (int i = 0; i < 8; ++i) glPopMatrix();
    while (glGetError() != GL_NO_ERROR) {}
    glLoadIdentity();
    return 0;
}

