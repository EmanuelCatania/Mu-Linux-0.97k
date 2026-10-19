#pragma once

// IDA: Entity_SpawnEffects (0x004FC070).
// Emisores SIM; las mallas se dibujan mediante Entity_PrepareRender.
float *Entity_SpawnEffects(int entity);

void MoltSilhouette_BeginStep();
void MoltSilhouette_Invalidate(const void *entity);
void MoltSilhouette_Tick(int entity, void *model, float *bones, char translate);
