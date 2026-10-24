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
    uint32_t Advance(const StartTime& start);
    uint32_t Wait(const StartTime& start);
    // DESVIACION: tras una carga bloqueante (OpenWorld) el tiempo de carga no
    // es tiempo de juego; se descarta para no recuperarlo despues a 5 pasos por frame.
    void DiscardElapsed();
    // DESVIACION: preferencias del dibujo; el paso SIM conserva siempre 40 ms.
    void SetFpsLimit(int fps) { m_FpsLimit = fps >= 0 ? fps : 60; }
    void SetRenderDecoupled(bool enabled) { m_RenderDecoupled = enabled; }
    bool IsRenderDecoupled() const { return m_RenderDecoupled; }
    void Shutdown();
private:
    StartTime m_PreviousStart{};
    bool m_HasPreviousStart = false;
    double m_SimulationFraction = 0;
    double m_FractionMilliseconds = 0;
    int m_FpsLimit = 60;
    bool m_RenderDecoupled = false;
};
extern CFrameLimiter gFrameLimiter;
