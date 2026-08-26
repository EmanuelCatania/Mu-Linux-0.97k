// Render_MatrixStack.cpp
// Extracted from stubs.cpp; IDA provenance comments retained.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"
#include "Net/MuEmu.h"
#include "Net/Net.h"
#include "Render/Camera.h"

// ── GL_PopMatrixAll ───────────────────────────────────────────────────────────
// BUG-FIX CRÍTICO: antes hacía un solo glPopMatrix() asumiendo modo actual.
// Pero FUN_005119b0 (GL_SetupView) pushea DOS matrices (PROJECTION + MODELVIEW),
// y Scene_Login sólo hace un glPopMatrix antes de Begin2D. Resultado: cada frame
// quedaba un push acumulado en PROJECTION → stack overflow tras 2 frames →
// matrices corruptas → UI 2D invisible. Aquí forzamos reset completo de ambos
// stacks a identidad. Los glGetError() limpian el GL_STACK_UNDERFLOW que
// generan los pops sobrantes (son inocuos, sólo setean el flag de error).
unsigned int __cdecl GL_PopMatrixAll(void) {
    glMatrixMode(GL_PROJECTION);
    for (int i = 0; i < 8; ++i) glPopMatrix();
    // Drain ALL pending error flags (multiple underflows can stack).
    // Single glGetError() solo limpia uno; el resto persiste y aparece
    // como 0x504 en cada FUN_005114f0 next frame.
    while (glGetError() != GL_NO_ERROR) {}
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    for (int i = 0; i < 8; ++i) glPopMatrix();
    while (glGetError() != GL_NO_ERROR) {}
    glLoadIdentity();
    return 0;
}

