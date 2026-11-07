#pragma once
float RenderBlend(float previous, float current, float alpha);
float RenderBlendAngle(float previous, float current, float alpha);
// Rotación por quaternion; escala y traslación por componentes. Extremos exactos.
void RenderBlendMatrix(const float *previous, const float *current, float alpha, float *out);

bool RenderInverseMatrix(const float *matrix, float *inverse);
