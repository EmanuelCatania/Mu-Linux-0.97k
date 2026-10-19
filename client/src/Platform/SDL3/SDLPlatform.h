#pragma once
#include "Platform/IInput.h"
#include <SDL3/SDL.h>
namespace Platform {
using InputSink = void (*)(const InputEvent&);
using CloseSink = void (*)();
// El host decide cómo despachar al juego; el backend no depende de sus globales.
void BindSDLCallbacks(InputSink input, CloseSink close);
SDL_Window* SDLWindow();
void InstallSDLInput();
void RemoveSDLInput();
// Bombeo de input; IWindow::ServiceEvents también sondea el transporte.
void ServiceSDLEvents(bool pumpNative);
}
