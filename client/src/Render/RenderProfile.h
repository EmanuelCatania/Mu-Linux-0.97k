#pragma once
#include <cstddef>
#include <cstdint>
#include "Platform/ITimer.h"

// DESVIACION: diagnóstico R1, independiente de la simulación. Sin reloj,
// queries ni asignaciones por frame cuando está apagado (sólo el guard).
namespace RenderProfile {
enum class Pass { Simulation, Prepare, Restore, Terrain, Objects, Characters, Effects, UI, Scene, Other, Swap, Wait, Count };
enum class State { Blend, Depth, Texture, Program, Count };
constexpr std::size_t PassCount=static_cast<std::size_t>(Pass::Count);
constexpr std::size_t StateCount=static_cast<std::size_t>(State::Count);
extern bool active;
void Toggle();
void RestartCapture();
bool Requested();
class Frame {
public:
    Frame(bool world,int map);
    ~Frame();
private: bool measured;
};
class Scope {
public:
    explicit Scope(Pass pass);
    ~Scope();
private: Pass previous;bool measured;
};
// Subcostos CPU: no cambian de pasada ni fuerzan envíos. Las etapas de texto
// se miden por separado, sin incluir una dentro de otra.
enum class TextWork { Measure, Raster, Compose, Upload, Draw, Total, Count };
class TextScope {
public:
    explicit TextScope(TextWork work);
    ~TextScope();
private:
    TextWork work; bool measured; Platform::TimerStart start{};
};
enum class InterpolationWork { Capture, Apply, Restore, Count };
class InterpolationScope {
public:
    explicit InterpolationScope(InterpolationWork work);
    ~InterpolationScope();
private:
    InterpolationWork work; bool measured; Platform::TimerStart start{};
};
void TextString(bool bitmap);
void TextureUpload(unsigned texture,std::uint64_t bytes,std::uint64_t allocated=0);
void ForgetTexture(unsigned texture);
void BufferUpload(std::uint64_t bytes,std::uint64_t allocated=0);
void CountDraw(std::uint64_t vertices,std::uint64_t instances=1);
void CountState(State state);
void CountLegacy();
void FragmentDraw(bool modern,bool alphaFallback);
void SetBackendReport(void (*report)(bool reset));
void SetPassAudit(void (*audit)(Pass pass,unsigned occurrence));
void Configuration(int width,int height,int limit,bool vsync,bool decoupled,bool terrain=false,bool composition=true,bool materials=false,bool fragments=false);
inline void Legacy() { if(active)CountLegacy(); }
inline void Draw(std::uint64_t vertices,std::uint64_t instances=1) { if(active)CountDraw(vertices,instances); }
inline void Change(State state) { if(active)CountState(state); }
// El backend entrega sólo cuadros completos, disponibles, de la misma escena.
void GpuResult(int map,const double* times,const bool* present);
const char* Summary();
void ResetContext();
}
