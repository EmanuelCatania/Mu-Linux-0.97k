#pragma once
#include <cstddef>

// Aísla poses/cajas, materiales, huesos propios y contexto BMD del dibujo.
void RenderDerivedState_Begin();
void RenderDerivedState_End();
// Los productores invalidan una dirección si cambia de dueño durante un draw.
void RenderDerivedState_Forget(const void *owner);

// Colaboración con pools FX: no aplica snapshots ni modifica sus altas.
void RenderDerivedState_SaveFx(void *slot, std::size_t size);
// Guardar un objeto del mapa antes de que el dibujo o la interpolación lo
// modifiquen. Sólo se copian los que se tocan, no los miles del mapa.
void RenderDerivedState_SaveObject(void *node);
void RenderDerivedState_Cancel();
