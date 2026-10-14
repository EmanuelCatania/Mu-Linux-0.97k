#pragma once
#include <cstdint>

namespace Platform {
struct WindowSettings {
    uint32_t width;
    uint32_t height;
    bool windowMode;
    bool borderless;
};

// Ventana y modo de video. Input, bombeo de eventos y contexto GL se extraen aparte.
class IWindow {
public:
    virtual ~IWindow() = default;
    virtual void ServiceEvents() {}
    virtual bool UsesPlatformPump() const { return false; }
    virtual bool CloseRequested() const { return false; }
    virtual void Shutdown() {}
    virtual bool Create(const WindowSettings& settings) = 0;
    virtual bool IsCreated() const = 0;
    virtual void ChangeMode(const WindowSettings& settings) = 0;
    virtual void RestoreDisplay() = 0;
    virtual void SetTitle(const char* title) = 0;
    virtual void ShowAndFocus() = 0;
    virtual void RequestCloseApplication() = 0;
};
IWindow& GetWindow();
} // namespace Platform
