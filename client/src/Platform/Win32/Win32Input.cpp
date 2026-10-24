#include "stdafx.h"
#include "Text/TextCore.h"
#include "Platform/IInput.h"
#include "Platform/Win32/Win32Window.h"
#include "Input/Input.h"
#include <windowsx.h>
namespace Platform {
class Win32Input final : public IInput {
public:
    bool HasImeModeIndicator() const override { return true; }
    int16_t KeyState(int key) override { return GetAsyncKeyState(key); }
    void CaptureMouse(bool capture) override {
        if (capture) SetCapture(GetWin32Window().GetHwnd());
        else ReleaseCapture();
    }
    // IDA: CheckIME_Status (0x0047EDC0), misma adquisición/consulta/cierre del contexto.
    ImeState ReadImeState(bool clearActive) override {
        const HWND window = GetWin32Window().GetHwnd();
        HIMC context = ImmGetContext(window);
        DWORD conversion = 0, sentence = 0;
        ImmGetConversionStatus(context, &conversion, &sentence);
        if (clearActive && (conversion || sentence)) ImmSetConversionStatus(context, 0, 0);
        ImmReleaseContext(window, context);
        return {conversion, sentence};
    }
    // IDA: SetPositionIME_Wnd (0x0047ECAF).
    void PositionIme(int x, int y, int width, int height) override {
        COMPOSITIONFORM form = {};
        form.dwStyle = CFS_POINT;
        form.ptCurrentPos = {x, y};
        SetRect(&form.rcArea, 0, 0, width, height);
        SendMessageA(ImmGetDefaultIMEWnd(GetWin32Window().GetHwnd()), WM_IME_CONTROL, IMC_SETCOMPOSITIONWINDOW, (LPARAM)&form);
    }
};
IInput& GetInput() { static Win32Input input; return input; }
} // namespace Platform

// IDA: WndProc (0x004149D0). Traducción síncrona: no bombea ni difiere mensajes.
void Input_OnWindowMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    Platform::InputEvent event{};
    switch (message) {
    case WM_MOUSEMOVE: event.type = Platform::InputEventType::MouseMove; break;
    case WM_LBUTTONDOWN: event.type = Platform::InputEventType::LeftDown; break;
    case WM_LBUTTONUP: event.type = Platform::InputEventType::LeftUp; break;
    case WM_LBUTTONDBLCLK: event.type = Platform::InputEventType::LeftDoubleClick; break;
    case WM_RBUTTONDOWN: event.type = Platform::InputEventType::RightDown; break;
    case WM_RBUTTONUP: event.type = Platform::InputEventType::RightUp; break;
    case WM_MOUSEWHEEL: event.type = Platform::InputEventType::Wheel; break;
    case WM_MBUTTONDOWN: event.type = Platform::InputEventType::MiddleDown; break;
    case WM_MBUTTONUP: event.type = Platform::InputEventType::MiddleUp; break;
    case WM_IME_STARTCOMPOSITION: event.type = Platform::InputEventType::ImeStart; break;
    case WM_IME_ENDCOMPOSITION: event.type = Platform::InputEventType::ImeEnd; break;
    case WM_CHAR: event.type = Platform::InputEventType::Text; break;
    default: return;
    }
    event.x = GET_X_LPARAM(lParam);
    event.y = GET_Y_LPARAM(lParam);
    event.wheel = GET_WHEEL_DELTA_WPARAM(wParam);
    // La ventana es ANSI: WM_CHAR trae un byte 1252; el campo de texto
    // recibe el code point (el texto del cliente es UTF-8, Fase 4).
    event.character = message == WM_CHAR && wParam < 256
        ? (uint32_t)TextEngine::DecodeWindows1252((unsigned char)wParam) : (uint32_t)wParam;
    Input_OnEvent(event);
}
