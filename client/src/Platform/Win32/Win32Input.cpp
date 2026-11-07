#include "stdafx.h"
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
        SendMessageW(ImmGetDefaultIMEWnd(GetWin32Window().GetHwnd()), WM_IME_CONTROL, IMC_SETCOMPOSITIONWINDOW, (LPARAM)&form);
    }
    std::string ClipboardText() override {
        std::string result;
        if (!OpenClipboard(GetWin32Window().GetHwnd())) return result;
        if (HANDLE data = GetClipboardData(CF_UNICODETEXT)) {
            if (const wchar_t* text = static_cast<const wchar_t*>(GlobalLock(data))) {
                const int size = WideCharToMultiByte(CP_UTF8, 0, text, -1, nullptr, 0, nullptr, nullptr);
                if (size > 1) {
                    result.resize((size_t)size - 1);
                    WideCharToMultiByte(CP_UTF8, 0, text, -1, &result[0], size, nullptr, nullptr);
                }
                GlobalUnlock(data);
            }
        }
        CloseClipboard();
        return result;
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
    // DESVIACION (Fase 4, texto Unicode): la ventana es Unicode, así que
    // WM_CHAR trae UTF-16 y los caracteres fuera del BMP llegan como dos
    // mensajes (par surrogate). Al campo de texto le llega el code point.
    if (message == WM_CHAR) {
        static wchar_t high = 0;
        const wchar_t unit = (wchar_t)wParam;
        if (unit >= 0xD800 && unit <= 0xDBFF) { high = unit; return; }
        if (unit >= 0xDC00 && unit <= 0xDFFF) {
            if (!high) return;
            event.character = 0x10000u + (((uint32_t)high - 0xD800u) << 10) + ((uint32_t)unit - 0xDC00u);
            high = 0;
        } else {
            high = 0;
            event.character = unit;
        }
    } else {
        event.character = (uint32_t)wParam;
    }
    Input_OnEvent(event);
}
