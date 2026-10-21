#include "Platform/SDL3/SDLPlatform.h"
#include "Platform/IGLContext.h"
#include "Platform/GLFunctions.h"
#include <SDL3/SDL_opengl.h>
namespace Platform {
namespace {
GLProc ResolveGL(const char* name) { return reinterpret_cast<GLProc>(SDL_GL_GetProcAddress(name)); }
class SDLGLContext final : public IGLContext {
    SDL_GLContext context = nullptr;
public:
    void* CurrentIdentity() const override { return SDL_GL_GetCurrentContext(); }
    bool Initialize() override {
        if (context) return SDL_GL_MakeCurrent(SDLWindow(),context);
        context = SDL_GL_CreateContext(SDLWindow());
        if (!context) return false;
        if (!SDL_GL_MakeCurrent(SDLWindow(), context)) { SDL_GL_DestroyContext(context); context = nullptr; return false; }
        // DESVIACION: validar perfil y cargar funciones antes del primer draw.
        if (!LoadGL33(ResolveGL)) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"%s",GL33Error());
            Release(); return false;
        }
        // El paso de 40 ms pertenece a CFrameLimiter, no al swap del monitor.
        SDL_GL_SetSwapInterval(0);
        SDL_Log("OpenGL: %s | %s | %s",
            (const char*)glGetString(GL_VENDOR), (const char*)glGetString(GL_RENDERER),
            (const char*)glGetString(GL_VERSION));
        return true;
    }
    void Release() override {
        if (context) { SDL_GL_DestroyContext(context); context = nullptr; ResetGL33(); }
    }
    void Present() override { if (context) SDL_GL_SwapWindow(SDLWindow()); }
    // DESVIACION: el mismo control de vsync que el backend WGL.
    bool SetSwapInterval(int interval) override {
        return context && (interval == 0 || interval == 1) && SDL_GL_SetSwapInterval(interval);
    }
};
}
IGLContext& GetGLContext() { static auto* instance = new SDLGLContext; return *instance; }
}
