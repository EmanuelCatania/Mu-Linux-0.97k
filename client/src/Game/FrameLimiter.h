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
    void Shutdown();
private:
    double m_FractionMilliseconds = 0;
};
extern CFrameLimiter gFrameLimiter;
