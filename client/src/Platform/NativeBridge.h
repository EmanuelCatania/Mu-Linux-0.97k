#pragma once
#include <windows.h>
namespace Platform {
// Puente temporal mientras GDI y WSAAsyncSelect siguen en el cliente Windows.
void BindNativeDispatch(HINSTANCE instance, WNDPROC procedure);
void DispatchNativeMessage(const MSG& message);
HWND NativeWindow();
HINSTANCE NativeInstance();
HDC NativeDC();
HGLRC NativeGLContext();
}
