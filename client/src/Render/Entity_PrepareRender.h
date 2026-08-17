#pragma once
// Entity_PrepareRender.h - Entity preparation for rendering declarations

#include "stdafx.h"

// Prepares an entity for rendering by checking if it's renderable and setting up
// the appropriate OpenGL state. Called for each entity during the terrain render pass.
// @ 0x004fc030
void Entity_PrepareRender(void *entity, int slot, int flag, char mode);