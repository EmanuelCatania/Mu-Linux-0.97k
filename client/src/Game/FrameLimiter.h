#pragma once
#include <windows.h>

class CFrameLimiter
{
public:
    struct StartTime {
        LARGE_INTEGER counter{};
        DWORD ticks = 0;
    };
    CFrameLimiter() = default;
    ~CFrameLimiter();
    CFrameLimiter(const CFrameLimiter&) = delete;
    CFrameLimiter& operator=(const CFrameLimiter&) = delete;
    StartTime Start();
    DWORD Wait(const StartTime& start);
    void Shutdown();
private:
    void UseSleep();
    HANDLE m_Timer = nullptr;
    LARGE_INTEGER m_Frequency{};
    bool m_Initialized = false;
    bool m_PeriodActive = false;
    double m_FractionMilliseconds = 0;
};

extern CFrameLimiter gFrameLimiter;
