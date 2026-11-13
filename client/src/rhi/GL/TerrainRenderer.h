#pragma once
#include "render/TerrainDraw.h"
namespace TerrainRendererGL {
TerrainDraw::Result Draw(const TerrainDraw::FrameData& frame,const std::vector<TerrainDraw::Tile>& tiles);
bool BoundSourceTexture();
bool SourceTexture(std::uint32_t handle);
void Release();
}
