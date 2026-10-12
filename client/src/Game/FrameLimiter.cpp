#include "stdafx.h"
#include "Game/FrameLimiter.h"
#include <mmsystem.h>
#include <cmath>

CFrameLimiter gFrameLimiter;

CFrameLimiter::~CFrameLimiter()
{
    Shutdown();
}

void CFrameLimiter::UseSleep()
{
    if (m_Timer) {
        CloseHandle(m_Timer);
        m_Timer = nullptr;
    }
    if (!m_PeriodActive) m_PeriodActive = timeBeginPeriod(1) == TIMERR_NOERROR;
}

CFrameLimiter::StartTime CFrameLimiter::Start()
{
    if (!m_Initialized) {
        m_Initialized = true;
        QueryPerformanceFrequency(&m_Frequency);
        // Resolución dinámica: versiones antiguas pueden carecer de API o del flag.
        using CreateTimer = HANDLE (WINAPI*)(LPSECURITY_ATTRIBUTES, LPCWSTR, DWORD, DWORD);
        const auto createTimer = reinterpret_cast<CreateTimer>(
            GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "CreateWaitableTimerExW"));
        constexpr DWORD HighResolution = 0x00000002;
        if (createTimer && m_Frequency.QuadPart > 0)
            m_Timer = createTimer(nullptr, nullptr, HighResolution, TIMER_MODIFY_STATE | SYNCHRONIZE);
        if (!m_Timer) UseSleep();
    }
    StartTime start;
    start.ticks = GetTickCount();
    if (m_Frequency.QuadPart > 0 && !QueryPerformanceCounter(&start.counter))
        start.counter.QuadPart = 0;
    return start;
}

// IDA: Game_MainLoop (0x00525D40), espera posterior al render.
// DESVIACION: QPC y espera suspendida de alta resolución; fallback Sleep con período balanceado.
DWORD CFrameLimiter::Wait(const StartTime& start)
{
    constexpr double FrameMilliseconds = 40.0;
    double elapsed;
    for (;;) {
        LARGE_INTEGER now;
        if (start.counter.QuadPart && QueryPerformanceCounter(&now))
            elapsed = (double)(now.QuadPart - start.counter.QuadPart) * 1000.0 / m_Frequency.QuadPart;
        else
            elapsed = (DWORD)(GetTickCount() - start.ticks);
        if (elapsed >= FrameMilliseconds) break;
        const double remaining = FrameMilliseconds - elapsed;
        if (m_Timer) {
            LARGE_INTEGER due;
            due.QuadPart = -(LONGLONG)std::ceil(remaining * 10000.0);
            if (SetWaitableTimer(m_Timer, &due, 0, nullptr, nullptr, FALSE) &&
                WaitForSingleObject(m_Timer, INFINITE) == WAIT_OBJECT_0) continue;
            UseSleep();
        }
        Sleep((DWORD)std::ceil(remaining));
    }
    // Conservar demoras y fracciones para el acumulador entero y el catch-up original.
    elapsed += m_FractionMilliseconds;
    const DWORD milliseconds = (DWORD)elapsed;
    m_FractionMilliseconds = elapsed - milliseconds;
    return milliseconds;
}

void CFrameLimiter::Shutdown()
{
    if (m_Timer) {
        CloseHandle(m_Timer);
        m_Timer = nullptr;
    }
    if (m_PeriodActive) {
        timeEndPeriod(1);
        m_PeriodActive = false;
    }
    m_Initialized = false;
    m_FractionMilliseconds = 0;
}
