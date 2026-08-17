#pragma once
// Entity_Render_3D.h - Entity 3D geometry rendering declarations

#include "stdafx.h"

// Renders entity geometry via OpenGL. This is the main 3D geometry draw function
// called during the terrain rendering pass for each entity.
// @ 0x004fc070
void Entity_Render_3D(void *entity);