#include "Platform/SDL3/SDLPlatform.h"
#include "Platform/IWindow.h"
#include "Platform/ISocket.h"
#include <string>
#include <cstring>
#include "MuIconData.h"
#ifndef _WIN32
#include <SDL3/SDL_opengl.h>
#endif
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
#ifndef _WIN32
// Crea una ventana oculta con contexto GL y devuelve si el renderer es por
// software (llvmpipe/softpipe/swrast). Caso real: NVIDIA no publica la
// plataforma EGL Wayland de 32 bits (egl-wayland es sólo x64), así que un
// cliente i386 en Wayland cae a Mesa llvmpipe aunque haya GPU. Por GLX/XWayland
// sí usa el driver propietario.
bool ProbeSoftwareGL() {
    SDL_Window* probe = SDL_CreateWindow("probe", 16, 16, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    if (!probe) return true;
    bool software = true;
    if (SDL_GLContext ctx = SDL_GL_CreateContext(probe)) {
        const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
        software = renderer && (SDL_strcasestr(renderer, "llvmpipe") || SDL_strcasestr(renderer, "softpipe") ||
                                SDL_strcasestr(renderer, "swrast"));
        if (software) SDL_Log("Renderer por software en %s: %s", SDL_GetCurrentVideoDriver(), renderer);
        SDL_GL_DestroyContext(ctx);
    }
    SDL_DestroyWindow(probe);
    return software;
}
#endif
// Decodifica src/icon.ico (un DIB paletizado de 8 bpp con máscara AND, el
// mismo que usa el recurso Win32) a una superficie RGBA para SDL_SetWindowIcon.
// Devuelve nullptr si el formato no es el esperado.
SDL_Surface* LoadWindowIcon() {
    const unsigned char* ico = kMuIconIco;
    const size_t size = sizeof(kMuIconIco);
    auto u16 = [&](size_t o) { return unsigned(ico[o] | ico[o + 1] << 8); };
    auto u32 = [&](size_t o) { return u16(o) | u16(o + 2) << 16; };
    if (size < 22 || u16(2) != 1 || u16(4) < 1) return nullptr;
    const size_t dib = u32(18);
    if (dib + 40 > size || u32(dib) != 40) return nullptr;
    const int width = int(u32(dib + 4)), height = int(u32(dib + 8)) / 2;
    const unsigned bpp = u16(dib + 14);
    unsigned colors = u32(dib + 32);
    if (bpp != 8 || width <= 0 || height <= 0) return nullptr;
    if (colors == 0) colors = 256;
    const size_t palette = dib + 40, xorStride = (size_t(width) + 3) & ~size_t(3);
    const size_t xorBits = palette + colors * 4, andStride = ((size_t(width) + 31) / 32) * 4;
    const size_t andBits = xorBits + xorStride * height;
    if (andBits + andStride * height > size) return nullptr;
    SDL_Surface* surface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGBA32);
    if (!surface) return nullptr;
    for (int y = 0; y < height; ++y) {
        const size_t src = size_t(height - 1 - y);   // el DIB es bottom-up
        auto* row = static_cast<unsigned char*>(surface->pixels) + size_t(y) * surface->pitch;
        for (int x = 0; x < width; ++x) {
            const unsigned index = ico[xorBits + src * xorStride + x];
            const unsigned char* bgra = ico + palette + (index < colors ? index : 0) * 4;
            const bool transparent = ico[andBits + src * andStride + x / 8] & (0x80 >> (x % 8));
            row[x * 4 + 0] = bgra[2]; row[x * 4 + 1] = bgra[1]; row[x * 4 + 2] = bgra[0];
            row[x * 4 + 3] = transparent ? 0 : 255;
        }
    }
    return surface;
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
#ifndef _WIN32
        // SDL 3.2 prefiere X11 si el compositor no ofrece fifo-v1. La sesión
        // Wayland debe preferir su backend nativo, respetando SDL_VIDEODRIVER.
        const char* session = SDL_getenv("XDG_SESSION_TYPE");
        if (SDL_getenv("WAYLAND_DISPLAY") || (session && SDL_strcmp(session, "wayland") == 0))
            SDL_SetHintWithPriority(SDL_HINT_VIDEO_DRIVER, "wayland,x11", SDL_HINT_DEFAULT);
#endif
        if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) return false;
        if (!SetGLAttributes()) { SDL_QuitSubSystem(SDL_INIT_VIDEO); return false; }
#ifndef _WIN32
        // Si Wayland sólo ofrece GL por software y el usuario no forzó el
        // driver, se reintenta con X11 (XWayland), que llega al driver de la GPU.
        if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "wayland") == 0 && !SDL_getenv("SDL_VIDEODRIVER") &&
            ProbeSoftwareGL()) {
            SDL_QuitSubSystem(SDL_INIT_VIDEO);
            SDL_SetHintWithPriority(SDL_HINT_VIDEO_DRIVER, "x11", SDL_HINT_OVERRIDE);
            if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
                SDL_SetHintWithPriority(SDL_HINT_VIDEO_DRIVER, "wayland", SDL_HINT_OVERRIDE);
                if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) return false;
            }
            if (!SetGLAttributes()) { SDL_QuitSubSystem(SDL_INIT_VIDEO); return false; }
        }
#endif
        SDL_Log("Backend de video: %s", SDL_GetCurrentVideoDriver());
        window = SDL_CreateWindow("Mu Online", settings.width, settings.height,
            SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
        if (!window) { SDL_QuitSubSystem(SDL_INIT_VIDEO); return false; }
        if (SDL_Surface* icon = LoadWindowIcon()) { SDL_SetWindowIcon(window, icon); SDL_DestroySurface(icon); }
        InstallSDLInput();
        ChangeMode(settings);
        SDL_HideCursor();
        SDL_StartTextInput(window);
        return true;
    }
    static bool SetGLAttributes() {
        // DESVIACION: perfil compatible para conservar glBegin, listas y matrices
        // durante la migración por subsistemas; no solicitar core ni forward.
        const bool context=SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,3) &&
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,3) &&
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_COMPATIBILITY) &&
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,0);
        // IDA: OpenGL_Init (0x0041DE30), mínimos de color/profundidad de 16 bits.
        return context && SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5) &&
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6) &&
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5) &&
            SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0) &&
            SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 16) &&
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1) &&
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    }
    void ServiceLoadingEvents(unsigned) override { ServiceEvents(); }
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
    void QuitLoop() override {
#ifdef _WIN32
        PostQuitMessage(0); // Conserva el comportamiento del cliente SDL Windows.
#else
        closing = true; // El cierre de recursos ocurre al salir del bucle.
#endif
    }
    void CloseWindow() override { RequestCloseApplication(); }
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
