#include "stdafx.h"
#include "Game/FrameLimiter.h"
CFrameLimiter gFrameLimiter;
CFrameLimiter::~CFrameLimiter() { Shutdown(); }
CFrameLimiter::StartTime CFrameLimiter::Start() { return Platform::GetTimer().Start(); }
// IDA: Game_MainLoop (0x00525D40): paso fijo de 40 ms y fracciones acumuladas.
uint32_t CFrameLimiter::Wait(const StartTime& start)
{
    // DESVIACION: FpsLimit se aplica al habilitar el desacople; hasta entonces
    // se mantiene la espera original de 40 ms para no acelerar los avances por render.
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
}
