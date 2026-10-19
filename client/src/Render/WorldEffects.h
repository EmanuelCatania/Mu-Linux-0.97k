#pragma once

// Muestra de sprites de objetos registrados de la escena, independiente del draw.
void WorldEffects_BeginStep();
void WorldEffects_Tick(int entity);
void WorldEffects_Draw(int entity);
int WorldEffects_AddSprite(unsigned short type, float *position, float scale,
    float *light, int owner, float rotation, int mode);
