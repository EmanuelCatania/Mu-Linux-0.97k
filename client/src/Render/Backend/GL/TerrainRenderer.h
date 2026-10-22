#pragma once
#include "Render/TerrainDraw.h"
namespace TerrainRendererGL {
TerrainDraw::Result Draw(const TerrainDraw::FrameData& frame,const std::vector<TerrainDraw::Tile>& tiles);
void Release();
}
