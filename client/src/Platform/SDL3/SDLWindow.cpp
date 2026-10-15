#include "Platform/SDL3/SDLPlatform.h"
#include "Platform/IWindow.h"
#include "Platform/ISocket.h"
#include <string>
#ifdef _WIN32
#include <windows.h>
#endif
namespace Platform {
namespace {
// El título del cliente conserva Windows-1252; SDL recibe UTF-8 en ambos sistemas.
std::string TitleUTF8(const char* text) {
    const unsigned short extra[32] = {0x20ac,0x81,0x201a,0x192,0x201e,0x2026,0x2020,0x2021,0x2c6,0x2030,0x160,0x2039,0x152,0x8d,0x17d,0x8f,0x90,0x2018,0x2019,0x201c,0x201d,0x2022,0x2013,0x2014,0x2dc,0x2122,0x161,0x203a,0x153,0x9d,0x17e,0x178};
    std::string result;
    if (!text) return result;
    for (auto p = reinterpret_cast<const unsigned char*>(text); *p; ++p) {
        unsigned code = *p >= 128 && *p < 160 ? extra[*p - 128] : *p;
        if (code < 128) result += char(code);
        else {
            if (code >= 2048) { result += char(0xe0 | (code >> 12)); result += char(0x80 | ((code >> 6) & 63)); }
            else result += char(0xc0 | (code >> 6));
            result += char(0x80 | (code & 63));
        }
    }
    return result;
}
class SDLWindowBackend final : public IWindow {
public:
    SDL_Window* window = nullptr;
    bool closing = false;
    void ShowError(const char* text, const char* title, bool attach, bool errorIcon) override {
#ifdef _WIN32
        auto hwnd = window ? static_cast<HWND>(SDL_GetPointerProperty(SDL_GetWindowProperties(window),
            SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr)) : nullptr;
        MessageBoxA(attach ? hwnd : nullptr, text, title, MB_OK | (errorIcon ? MB_ICONERROR : 0));
#else
        const auto caption = TitleUTF8(title ? title : "Error");
        const auto body = TitleUTF8(text);
        if (!SDL_ShowSimpleMessageBox(errorIcon ? SDL_MESSAGEBOX_ERROR : 0,
            caption.c_str(), body.c_str(), attach ? window : nullptr))
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error al mostrar diálogo: %s; %s", SDL_GetError(), body.c_str());
#endif
    }
    bool Create(const WindowSettings& settings) override {
        if (window) return true;
        closing = false;
        if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) return false;
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
        // IDA: OpenGL_Init (0x0041DE30), mínimos de color/profundidad de 16 bits.
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
        SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 16);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
        window = SDL_CreateWindow("Mu Online", settings.width, settings.height,
            SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
        if (!window) { SDL_QuitSubSystem(SDL_INIT_VIDEO); return false; }
        InstallSDLInput();
        ChangeMode(settings);
        SDL_HideCursor();
        SDL_StartTextInput(window);
        return true;
    }
    void ServiceEvents() override {
        ServiceSDLEvents(true);
        if (!closing) GetSocket().Poll();
    }
    bool UsesPlatformPump() const override { return true; }
    bool CloseRequested() const override { return closing; }
    void Shutdown() override {
        if (!window) return;
        RemoveSDLInput();
        SDL_StopTextInput(window);
        SDL_DestroyWindow(window); window = nullptr;
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
    }
    bool IsCreated() const override { return window != nullptr; }
    void ChangeMode(const WindowSettings& settings) override {
        if (!window) return;
        SDL_SetWindowFullscreen(window, false);
        SDL_SetWindowBordered(window, !settings.borderless);
        SDL_SetWindowSize(window, settings.width, settings.height);
        if (!settings.windowMode) {
            SDL_DisplayMode mode{};
            if (SDL_GetClosestFullscreenDisplayMode(SDL_GetDisplayForWindow(window),
                    settings.width, settings.height, 0, false, &mode))
                SDL_SetWindowFullscreenMode(window, &mode);
            SDL_SetWindowFullscreen(window, true);
        } else {
            SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        }
        SDL_SyncWindow(window);
    }
    void RestoreDisplay() override { if (window) SDL_SetWindowFullscreen(window, false); }
    void SetTitle(const char* title) override { if (window) SDL_SetWindowTitle(window, TitleUTF8(title).c_str()); }
    void ShowAndFocus() override { if (window) { SDL_ShowWindow(window); SDL_RaiseWindow(window); } }
    void RequestCloseApplication() override;
};
SDLWindowBackend& Window() { static auto* instance = new SDLWindowBackend; return *instance; }
InputSink inputSink = nullptr;
CloseSink closeSink = nullptr;
}
void DeliverSDLInput(const InputEvent& event) { if (inputSink) inputSink(event); }
void DeliverSDLClose() { if (closeSink) closeSink(); }
void BindSDLCallbacks(InputSink input, CloseSink close) { inputSink = input; closeSink = close; }
void SDLWindowBackend::RequestCloseApplication() {
    if (closing) return;
    closing = true;
    DeliverSDLClose();
}
SDL_Window* SDLWindow() { return Window().window; }
IWindow& GetWindow() { return Window(); }
}
