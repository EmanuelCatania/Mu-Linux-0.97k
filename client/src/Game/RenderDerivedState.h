#pragma once

// Aísla poses/cajas, materiales, huesos propios y contexto BMD del dibujo.
void RenderDerivedState_Begin();
void RenderDerivedState_End();
// Los productores invalidan una dirección si cambia de dueño durante un draw.
void RenderDerivedState_Forget(const void *owner);
