#pragma once
// Particle_Draw.h - Particle rendering declarations

#include "stdafx.h"

// Actual OpenGL particle drawing function. This function handles the rendering
// of individual particles with various types, positions, colors, and alpha values.
// @ 0x004f8bb0
void Particle_Draw(int type, float x, float y, float w, float h,
                   float *color, float z, float alpha);