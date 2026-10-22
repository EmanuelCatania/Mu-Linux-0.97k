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
// Sólo se usa al perfilar; consulta el estado previo, no mantiene otro caché.
void BlendFunction(unsigned source,unsigned destination);
void DepthWrite(bool enabled);
void DepthFunction(unsigned function);
void ObserveProgram(unsigned program);
void ObserveTexture(unsigned target,unsigned texture);
void SetEnabled(unsigned cap,bool enabled);
void PopAttributes();
}
