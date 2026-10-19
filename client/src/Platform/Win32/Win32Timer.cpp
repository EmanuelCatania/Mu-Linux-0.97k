#include "stdafx.h"
#include "Platform/ITimer.h"
#include "Platform/LocalTime.h"
#include <cmath>
namespace Platform {
class Win32Timer final : public ITimer {
public:
    int64_t CounterFrequency() override {
        LARGE_INTEGER value{};
        return QueryPerformanceFrequency(&value) ? value.QuadPart : 0;
    }
    bool ReadCounter(int64_t& counter) override {
        LARGE_INTEGER value{};
        if (!QueryPerformanceCounter(&value)) return false;
        counter = value.QuadPart;
        return true;
    }
    LocalDateTime LocalTime() override { return ReadLocalTime(); }
    uint32_t Ticks() override { return GetTickCount(); }
    uint32_t MultimediaTicks() override { return timeGetTime(); }
    void SleepMilliseconds(uint32_t milliseconds) override { Sleep(milliseconds); }
    TimerStart Start() override;
    double Wait(const TimerStart& start, double minimumMilliseconds) override;
    void Shutdown() override;
private:
    void UseSleep();
    HANDLE m_Timer = nullptr;
    LARGE_INTEGER m_Frequency{};
    bool m_Initialized = false;
    bool m_PeriodActive = false;
};
// Vida de proceso: evita depender del orden de destrucción entre unidades.
// Shutdown libera timer/período; el objeto sigue disponible para CFrameLimiter.
ITimer& GetTimer() { static auto* timer = new Win32Timer; return *timer; }

void Win32Timer::UseSleep()
{
    if (m_Timer) {
        CloseHandle(m_Timer);
        m_Timer = nullptr;
    }
    if (!m_PeriodActive) m_PeriodActive = timeBeginPeriod(1) == TIMERR_NOERROR;
}

TimerStart Win32Timer::Start()
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
    TimerStart start;
    LARGE_INTEGER counter{};
    start.ticks = GetTickCount();
    if (m_Frequency.QuadPart > 0 && !QueryPerformanceCounter(&counter))
        counter.QuadPart = 0;
    start.counter = counter.QuadPart;
    return start;
}

// IDA: Game_MainLoop (0x00525D40), espera posterior al render.
// DESVIACION: QPC y espera suspendida de alta resolución; fallback Sleep con período balanceado.
double Win32Timer::Wait(const TimerStart& start, double FrameMilliseconds)
{
    double elapsed;
    for (;;) {
        LARGE_INTEGER now;
        if (start.counter && QueryPerformanceCounter(&now))
            elapsed = (double)(now.QuadPart - start.counter) * 1000.0 / m_Frequency.QuadPart;
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
    return elapsed;
}

void Win32Timer::Shutdown()
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
}

} // namespace Platform
