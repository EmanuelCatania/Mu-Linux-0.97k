#pragma once
void ClothPresentation_BeginStep();
void ClothPresentation_Forget(const void *cloth);
void ClothPresentation_Before(void *cloth);
void ClothPresentation_After(void *cloth);
void ClothPresentation_Vertex(const void *cloth, int index, float *position);
