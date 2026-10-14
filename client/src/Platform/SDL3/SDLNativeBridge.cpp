#ifdef _WIN32
#include "Platform/NativeBridge.h"
#include "Platform/SDL3/SDLPlatform.h"
#include "Platform/IWindow.h"
#include "Platform/ISocket.h"
#include <GL/gl.h>
void Input_OnEvent(const Platform::InputEvent& event);
namespace Platform {
namespace {
HINSTANCE instance = nullptr;
WNDPROC gameProcedure = nullptr;
// El poll portable entrega los mismos eventos al dispatcher del cliente.
void SocketEvent(uint16_t event, uint16_t error) {
    if (gameProcedure && NativeWindow()) gameProcedure(NativeWindow(), WM_USER, 0, MAKELPARAM(event, error));
}
void Close() { if (gameProcedure && NativeWindow()) gameProcedure(NativeWindow(), WM_DESTROY, 0, 0); }
}
void BindNativeDispatch(HINSTANCE value, WNDPROC procedure) {
    instance = value; gameProcedure = procedure;
    BindSDLCallbacks(Input_OnEvent, Close);
    GetSocket().BindEvents(SocketEvent);
}
// TEMPORAL: GDI y consumidores heredados acceden sólo por la fachada CWindow.
HWND NativeWindow() {
    if (!SDLWindow()) return nullptr;
    return static_cast<HWND>(SDL_GetPointerProperty(SDL_GetWindowProperties(SDLWindow()), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));
}
HINSTANCE NativeInstance() { return instance; }
HDC NativeDC() {
    if (!SDLWindow()) return nullptr;
    return static_cast<HDC>(SDL_GetPointerProperty(SDL_GetWindowProperties(SDLWindow()), SDL_PROP_WINDOW_WIN32_HDC_POINTER, nullptr));
}
HGLRC NativeGLContext() { return wglGetCurrentContext(); }
void DispatchNativeMessage(const MSG& message) { DispatchMessageW(&message); }
}
// SDL entrega mouse/texto/IME; no duplicarlos por la ruta de WM_CHAR heredada.
void Input_OnWindowMessage(UINT, WPARAM, LPARAM) {}
#endif
