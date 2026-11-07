#pragma once
// Sprite_Draw.h - Sprite rendering declarations

#include "stdafx.h"

// Main sprite drawing function that renders 2D sprites in the 3D world.
// This function handles the actual OpenGL calls to draw textured quads for sprites.
// @ 0x00440060
void Sprite_Draw(void *model, int flags, float rot, float *pos, float *scale,
                 void *anim_ptr, void *dir_ptr, byte param1, byte param2);