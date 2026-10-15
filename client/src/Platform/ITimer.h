#pragma once
#include <cstdint>
namespace Platform {
struct LocalDateTime { uint16_t wYear, wMonth, wDayOfWeek, wDay, wHour, wMinute, wSecond, wMilliseconds; };
struct TimerStart { int64_t counter = 0; uint32_t ticks = 0; };
class ITimer {
public:
    virtual ~ITimer() = default;
    // Cero indica que el reloj de alta resolución no está disponible.
    virtual int64_t CounterFrequency() = 0;
    virtual bool ReadCounter(int64_t& counter) = 0;
    virtual LocalDateTime LocalTime() = 0;
    virtual uint32_t Ticks() = 0;
    virtual uint32_t MultimediaTicks() = 0;
    virtual void SleepMilliseconds(uint32_t milliseconds) = 0;
    virtual TimerStart Start() = 0;
    virtual double Wait(const TimerStart& start, double minimumMilliseconds) = 0;
    virtual void Shutdown() = 0;
};
ITimer& GetTimer();
} // namespace Platform
