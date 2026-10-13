#pragma once

namespace Platform {
// Contexto OpenGL compatibility y presentación; no administra estado de render.
class IGLContext {
public:
    virtual ~IGLContext() = default;
    virtual bool Initialize() = 0;
    virtual void Release() = 0;
    virtual void Present() = 0;
};
IGLContext& GetGLContext();
} // namespace Platform
