#include "stdafx.h"
#include "Platform/NativeBridge.h"
#include "Platform/Win32/Win32Window.h"
#include "Platform/Win32/Win32GLContext.h"
namespace Platform {
void BindNativeDispatch(HINSTANCE instance, WNDPROC procedure) { GetWin32Window().BindDispatch(instance, procedure); }
void DispatchNativeMessage(const MSG& message) {
    if (message.message != WM_SYSKEYDOWN && message.message != WM_SYSKEYUP) DispatchMessageA(&message);
}
HWND NativeWindow() { return GetWin32Window().GetHwnd(); }
HINSTANCE NativeInstance() { return GetWin32Window().GetInstance(); }
HDC NativeDC() { return GetWin32GLContext().GetHdc(); }
HGLRC NativeGLContext() { return GetWin32GLContext().GetHandle(); }
}
