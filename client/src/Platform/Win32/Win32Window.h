#pragma once
#include "Platform/IWindow.h"
#include <windows.h>

namespace Platform {
// Puente nativo transitorio para WndProc, GDI, audio y red aún no abstraídos.
class Win32Window final : public IWindow {
public:
    void BindDispatch(HINSTANCE instance, WNDPROC procedure);
    void ShowError(const char* text, const char* title, bool attach, bool errorIcon) override {
        MessageBoxA(attach ? m_hWnd : nullptr, text, title, MB_OK | (errorIcon ? MB_ICONERROR : 0));
    }
    bool Create(const WindowSettings& settings) override;
    bool IsCreated() const override { return m_hWnd != nullptr; }
    void ChangeMode(const WindowSettings& settings) override;
    void RestoreDisplay() override;
    void SetTitle(const char* title) override;
    void ShowAndFocus() override;
    void RequestCloseApplication() override;
    HWND GetHwnd() const { return m_hWnd; }
    HINSTANCE GetInstance() const { return m_hInst; }
private:
    void ApplySettings(const WindowSettings& settings);
    void ApplyFullscreen();
    // IDA: g_hWnd (0x055C9FFC), g_hInst (0x055CA000).
    HWND m_hWnd = nullptr;
    HINSTANCE m_hInst = nullptr;
    WNDPROC m_WndProc = nullptr;
    DWORD m_Width = 640;
    DWORD m_Height = 480;
    bool m_WindowMode = true;
    bool m_Borderless = false;
    bool m_DisplayModeChanged = false;
};
Win32Window& GetWin32Window();
} // namespace Platform
