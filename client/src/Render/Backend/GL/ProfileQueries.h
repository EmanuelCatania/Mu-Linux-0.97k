#pragma once
#include "Render/RenderProfile.h"
namespace RenderProfileGL {
void BeginFrame(int map);
void SetPass(RenderProfile::Pass pass);
void EndFrame();
void Release();
std::uint64_t SkippedFrames();
std::uint64_t SkippedSegments();
bool Supported();
// Sólo se usa al perfilar; consulta el binding previo, no mantiene otro caché.
void ObserveProgram(unsigned program);
void ObserveTexture(unsigned target,unsigned texture);
}
