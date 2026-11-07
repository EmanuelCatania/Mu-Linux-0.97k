#pragma once
#include <cstddef>

// Pools binarios originales; no incluye sprites efímeros (Capture/ReplayStep).
enum class RenderFxPool { Effects, Joints, Particles, Leaves, Weather, Bugs,
    Ambient, Trails, Fades, Flares, Count };
struct RenderFxView {
    const unsigned char *previous=nullptr, *current=nullptr;
    std::size_t bytes=0;
    bool continuous=false;
};
// Vistas de sólo lectura, válidas hasta la próxima captura/alta/reset.
// Los bytes conservan tails y campos dependientes del tipo sin reinterpretar
// payload no inicializado. Nunca aplicar el bloque completo sobre un pool:
// sólo se interpolarán campos de presentación, con sus reglas por dominio.
void RenderFxSnapshots_BeginStep();
void RenderFxSnapshots_EndStep();
void RenderFxSnapshots_New(const void *slot);
void RenderFxSnapshots_Reset();
bool RenderFxSnapshots_Read(RenderFxPool pool, int slot, RenderFxView& out);
int RenderFxSnapshots_Count(RenderFxPool pool);
bool RenderFxSnapshots_Owner(const void *owner, RenderFxView& out);

// Guarda sólo slots vivos; las altas invalidan su copia antes de reutilizarlos.
void RenderFxSnapshots_SaveRenderState();
