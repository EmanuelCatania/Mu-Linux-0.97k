#pragma once
#include "Entity/EntityTypes.h"

// IDA: ala 790 de RenderPartObjectEffect (0x00504B50).
void LooseWingVisuals_BeginStep();
void LooseWingVisuals_ExpectPreview(unsigned int level);
void LooseWingVisuals_TickPreviews();
void LooseWingVisuals_TickGround(int entity);
void LooseWingVisuals_BeginRender();
void LooseWingVisuals_EndRender();
void LooseWingVisuals_SubmitPreview(const OBJECT &object, const float *angle,
    const float *headAngle, float bodyHeight, unsigned int level, bool spin);
