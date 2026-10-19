#pragma once
#include "Platform/ITimer.h"
class CFrameLimiter {
public:
    using StartTime = Platform::TimerStart;
    CFrameLimiter() = default;
    ~CFrameLimiter();
    CFrameLimiter(const CFrameLimiter&) = delete;
    CFrameLimiter& operator=(const CFrameLimiter&) = delete;
    StartTime Start();
    uint32_t Wait(const StartTime& start);
    // DESVIACION: preparar el límite libre sin adelantar el desacople del render.
    void SetFpsLimit(int fps) { m_FpsLimit = fps >= 0 ? fps : 60; }
    void SetRenderDecoupled(bool enabled) { m_RenderDecoupled = enabled; }
    void Shutdown();
private:
    double m_FractionMilliseconds = 0;
    int m_FpsLimit = 60;
    bool m_RenderDecoupled = false;
};
extern CFrameLimiter gFrameLimiter;
