#pragma once

// Dispositivo WebGPU (Dawn): instancia, adaptador, dispositivo y la superficie
// de la ventana. No conoce el estado de render del juego; las capas de dibujo
// toman de acá el dispositivo, la cola y la textura del cuadro actual.
#include <webgpu/webgpu.h>

namespace WebGPU {

// API nativa que Dawn usa por debajo. Auto deja elegir a Dawn.
enum class Api { Auto, D3D12, D3D11, Vulkan };

struct Surface {
    void* window = nullptr;    // HWND en Windows
    void* instance = nullptr;  // HINSTANCE en Windows
};

bool Initialize(Api api, const Surface& surface, unsigned width, unsigned height, bool vsync);
void Release();
bool IsReady();
// Reconfigura el swapchain (cambio de resolución o de vsync).
bool Configure(unsigned width, unsigned height, bool vsync);

// Toma la textura del cuadro. Devuelve false si la superficie no está
// disponible (ventana minimizada o recién cambiada): ese cuadro se saltea.
bool BeginFrame();
// Limpia el cuadro actual con el color dado (y la profundidad a 1).
void Clear(float r, float g, float b, float a);
void Present();

WGPUDevice GetDevice();
WGPUQueue GetQueue();
WGPUTextureFormat SurfaceFormat();
// Vista de la textura del cuadro actual; nula fuera de BeginFrame/Present.
WGPUTextureView FrameView();
// "D3D12 | NVIDIA GeForce ..." para el log y el menú de opciones.
const char* Description();

} // namespace WebGPU
