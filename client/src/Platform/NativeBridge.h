#pragma once
#include "Platform/PlatformHeaders.h"
namespace Platform {
// Puente temporal mientras GDI y WSAAsyncSelect siguen en el cliente Windows.
void BindNativeDispatch(HINSTANCE instance, WNDPROC procedure);
#ifdef _WIN32
void DispatchNativeMessage(const MSG& message);
#endif
HWND NativeWindow();
HINSTANCE NativeInstance();
HDC NativeDC();
HGLRC NativeGLContext();
}
