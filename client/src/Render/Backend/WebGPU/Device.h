#pragma once

// Dispositivo WebGPU (Dawn): instancia, adaptador, dispositivo y la superficie
// de la ventana. No conoce el estado de render del juego; las capas de dibujo
// toman de acá el dispositivo, la cola y la textura del cuadro actual.
#include <webgpu/webgpu.h>

namespace WebGPU {

// API nativa que Dawn usa por debajo. Auto deja elegir a Dawn.
enum class Api { Auto, D3D12, D3D11, Vulkan };

struct Surface {
    void* window = nullptr;    // HWND en Windows; SDL_Window* en Linux
    void* instance = nullptr;  // HINSTANCE en Windows; sin uso en Linux
};

bool Initialize(Api api, const Surface& surface, unsigned width, unsigned height, bool vsync);
void Release();
bool IsReady();
// Reconfigura el swapchain (cambio de resolución o de vsync).
bool Configure(unsigned width, unsigned height, bool vsync);

// Toma la textura del cuadro (una vez por cuadro: si ya se tomó, la conserva).
// Devuelve false si la superficie no está disponible (ventana minimizada o
// recién cambiada): ese cuadro se saltea.
// Las pasadas de dibujo las graba y envía Renderer.cpp.
bool BeginFrame();
// Presenta la textura del cuadro (la toma BeginFrame).
void Present();
// Tamaño configurado de la superficie.
unsigned Width();
unsigned Height();

WGPUDevice GetDevice();
WGPUQueue GetQueue();
WGPUTextureFormat SurfaceFormat();
// Textura y vista del cuadro actual; nulas fuera de BeginFrame/Present.
WGPUTexture FrameTexture();
WGPUTextureView FrameView();
// Espera a que termine una operación asíncrona (lectura de un buffer).
bool WaitFuture(WGPUFuture future);
// "D3D12 | NVIDIA GeForce ..." para el log y el menú de opciones.
const char* Description();

} // namespace WebGPU
