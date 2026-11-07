#pragma once
// DESVIACION: ventana de presentación, siempre dentro del guard de render.
void RenderInterpolation_Begin(int remainder);
void RenderInterpolation_End();
float RenderInterpolation_Alpha();
bool RenderInterpolation_OwnerContinuous(const void *owner);
// Cuatro tiles por paso: sólo corta presentación, nunca posiciones de SIM.
constexpr float RenderInterpolation_MaxDisplacement=400.0f;

bool RenderInterpolation_OwnerOffset(const void *owner, float *offset);
