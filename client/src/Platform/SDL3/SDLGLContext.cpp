#include "Platform/SDL3/SDLPlatform.h"
#include "Platform/IGLContext.h"
namespace Platform {
namespace {
class SDLGLContext final : public IGLContext {
    SDL_GLContext context = nullptr;
public:
    void* CurrentIdentity() const override { return SDL_GL_GetCurrentContext(); }
    bool Initialize() override {
        if (context) return true;
        context = SDL_GL_CreateContext(SDLWindow());
        if (!context) return false;
        if (!SDL_GL_MakeCurrent(SDLWindow(), context)) { SDL_GL_DestroyContext(context); context = nullptr; return false; }
        // El paso de 40 ms pertenece a CFrameLimiter, no al swap del monitor.
        SDL_GL_SetSwapInterval(0);
        return true;
    }
    void Release() override {
        if (context) { SDL_GL_DestroyContext(context); context = nullptr; }
    }
    void Present() override { if (context) SDL_GL_SwapWindow(SDLWindow()); }
};
}
IGLContext& GetGLContext() { static auto* instance = new SDLGLContext; return *instance; }
}
