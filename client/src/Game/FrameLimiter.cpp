#include "stdafx.h"
#include "Game/FrameLimiter.h"
CFrameLimiter gFrameLimiter;
CFrameLimiter::~CFrameLimiter() { Shutdown(); }
CFrameLimiter::StartTime CFrameLimiter::Start() { return Platform::GetTimer().Start(); }
// DESVIACION: acumular de entrada a entrada incluye espera, trabajo posterior
// al swap y despacho de eventos. Wait sólo limita y no suma tiempo dos veces.
uint32_t CFrameLimiter::Advance(const StartTime& start)
{
    if (!m_HasPreviousStart) { m_PreviousStart=start; m_HasPreviousStart=true; return 0; }
    const auto frequency=Platform::GetTimer().CounterFrequency();
    const double elapsed=start.counter && m_PreviousStart.counter && frequency>0 ?
        (start.counter-m_PreviousStart.counter)*1000.0/frequency :
        (double)(uint32_t)(start.ticks-m_PreviousStart.ticks);
    m_PreviousStart=start;
    const double total=elapsed+m_SimulationFraction;
    const uint32_t milliseconds=(uint32_t)total;
    m_SimulationFraction=total-milliseconds;
    return milliseconds;
}
// IDA: Game_MainLoop (0x00525D40): paso fijo de 40 ms y fracciones acumuladas.
uint32_t CFrameLimiter::Wait(const StartTime& start)
{
    // DESVIACION: desacoplado usa FpsLimit/VSync; el modo ligado conserva 40 ms.
    const double minimum = !m_RenderDecoupled ? 40.0 : (m_FpsLimit ? 1000.0 / m_FpsLimit : 0.0);
    const double elapsed = Platform::GetTimer().Wait(start, minimum) + m_FractionMilliseconds;
    const uint32_t milliseconds = (uint32_t)elapsed;
    m_FractionMilliseconds = elapsed - milliseconds;
    return milliseconds;
}
void CFrameLimiter::Shutdown()
{
    Platform::GetTimer().Shutdown();
    m_FractionMilliseconds = 0;
    m_SimulationFraction = 0; m_HasPreviousStart=false;
}
