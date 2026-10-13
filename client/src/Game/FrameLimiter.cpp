#include "stdafx.h"
#include "Game/FrameLimiter.h"
CFrameLimiter gFrameLimiter;
CFrameLimiter::~CFrameLimiter() { Shutdown(); }
CFrameLimiter::StartTime CFrameLimiter::Start() { return Platform::GetTimer().Start(); }
// IDA: Game_MainLoop (0x00525D40): paso fijo de 40 ms y fracciones acumuladas.
uint32_t CFrameLimiter::Wait(const StartTime& start)
{
    const double elapsed = Platform::GetTimer().Wait(start, 40.0) + m_FractionMilliseconds;
    const uint32_t milliseconds = (uint32_t)elapsed;
    m_FractionMilliseconds = elapsed - milliseconds;
    return milliseconds;
}
void CFrameLimiter::Shutdown()
{
    Platform::GetTimer().Shutdown();
    m_FractionMilliseconds = 0;
}
