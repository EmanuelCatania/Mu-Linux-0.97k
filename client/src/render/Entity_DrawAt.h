#pragma once
// Entity_DrawAt.h - Entity sprite rendering with state declarations

#include "stdafx.h"

// Renders an entity as a sprite with state. This function is responsible for drawing
// individual sprite frames with proper positioning, rotation, and state handling.
// @ 0x00505a10 (126 instructions)
void Entity_DrawAt(void *entity, int entity_class, int slot, float *angle, float rot,
                   int state_flags, byte param1, byte param2, byte param3,
                   byte param4, byte param5, byte param6);