// Debug_Forge.cpp
// Diagnostic-only render trace, moved from stubs_mouse_hover.cpp.

// stubs_mouse_hover.cpp
//
// 2026-05-07 B3 refactor — moved from stubs.cpp lines 1180-2577 (1398 lines).
//
// "Mouse hover helpers" section + neutralized stubs:
//   FUN_004f8480 — Terrain_TilePick
//   FUN_004afa40 — ItemOnGround_HoverTest (NEUTRALIZED, returns -1)
//   FUN_004b0240 — SpecialObject_HoverTest (NEUTRALIZED)
//   FUN_004afdc0 — Entity_SelectNearest
//   ... and additional misc/legacy stubs through line 2577.

#include "stdafx.h"
#include "globals.h"
#include "functions.h"

extern "C" void DbgLogPublic(const char* msg);
extern void __cdecl FUN_0054158c(void* ptr);

// ── [DIAG FORGE] TEMPORAL — identificar quién dibuja el cubo cyan (Lorencia) ──
// Loguea draw/spawn calls cuyo centro (pos mundial) esté dentro de ~6 tiles del
// héroe. Rate-limited a 250 líneas/segundo. NO modifica lógica del cliente.
// REMOVER cuando se identifique el renderable.
//   fn    = punto del pipeline (RenderObject/RenderMesh/RenderSprite/...)
//   type  = entity type / particle-bitmap type (-1 si N/A)
//   model = model index (-1 si N/A)
//   bmp   = bitmap id solicitado (-1 si N/A)
//   glTex = GL texture handle (-1 si N/A)
//   mesh  = mesh index (-1 si N/A)
//   blend = flags/Components de blend (-1 si N/A)
// [DIAG FORGE] — trazado selectivo del pipeline de habilidades.  No modifica
// pools ni estado GL: sólo registra los tipos visuales de combate, limitado
// por segundo, para poder contrastar spawn → bind → draw contra IDA.
extern "C" void DbgForge(const char* fn, int type, int, int, int,
                         int, int, float wx, float wy, float wz,
                         float, float, float, float)
{
    // Las habilidades 9/10/14 crean primero efectos lógicos 190..240 y éstos
    // luego emiten partículas 1180..1300.  Mantener ambos rangos permite
    // comprobar la cadena completa sin registrar el mundo entero.
    if (!fn || !((type >= 190 && type <= 240) ||
                 (type >= 1180 && type <= 1300))) return;

    static DWORD lastWindow = 0;
    static unsigned count = 0;
    DWORD now = GetTickCount();
    if (now - lastWindow >= 1000) {
        lastWindow = now;
        count = 0;
    }
    if (count++ >= 80) return;

    char line[192];
    _snprintf_s(line, sizeof(line), _TRUNCATE,
                "VIS %s type=%d pos=(%.0f,%.0f,%.0f)",
                fn, type, wx, wy, wz);
    DbgLogPublic(line);
}

