#ifdef _WIN32
#include "Platform/NativeBridge.h"
#include "Platform/SDL3/SDLPlatform.h"
#include "Platform/IWindow.h"
#include <GL/gl.h>
void Input_OnEvent(const Platform::InputEvent& event);
namespace Platform {
namespace {
HINSTANCE instance = nullptr;
WNDPROC gameProcedure = nullptr;
// TEMPORAL: WSAAsyncSelect y timers hasta extraer el poll. No subclass de HWND.
bool SDLCALL MessageHook(void*, MSG* message) {
    if (message->hwnd != NativeWindow()) return true;
    switch (message->message) {
    case WM_USER: case WM_USER + 1: case WM_TIMER:
        gameProcedure(message->hwnd, message->message, message->wParam, message->lParam);
        return false;
    case WM_CLOSE:
        GetWindow().RequestCloseApplication();
        return false;
    default: return true;
    }
}
void Close() { if (gameProcedure && NativeWindow()) gameProcedure(NativeWindow(), WM_DESTROY, 0, 0); }
}
void BindNativeDispatch(HINSTANCE value, WNDPROC procedure) {
    instance = value; gameProcedure = procedure;
    BindSDLCallbacks(Input_OnEvent, Close);
    SDL_SetWindowsMessageHook(MessageHook, nullptr);
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
