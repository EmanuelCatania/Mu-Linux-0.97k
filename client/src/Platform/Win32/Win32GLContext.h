#pragma once
#include "Platform/IGLContext.h"
#include <windows.h>

namespace Platform {
class Win32GLContext final : public IGLContext {
public:
    void* CurrentIdentity() const override;
    bool Initialize() override;
    void Release() override;
    void Present() override;
    // Puente nativo para fuentes GDI y consumidores todavía no extraídos.
    HDC GetHdc() const { return m_hDC; }
    HGLRC GetHandle() const { return m_hRC; }
private:
    // IDA: g_hDC (0x055CA004), g_hRC (0x055CA008).
    HDC m_hDC = nullptr;
    HGLRC m_hRC = nullptr;
};
Win32GLContext& GetWin32GLContext();
} // namespace Platform
