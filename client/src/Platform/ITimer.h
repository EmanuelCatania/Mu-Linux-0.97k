#pragma once
#include <cstdint>
namespace Platform {
struct TimerStart { int64_t counter = 0; uint32_t ticks = 0; };
class ITimer {
public:
    virtual ~ITimer() = default;
    virtual uint32_t Ticks() = 0;
    virtual uint32_t MultimediaTicks() = 0;
    virtual void SleepMilliseconds(uint32_t milliseconds) = 0;
    virtual TimerStart Start() = 0;
    virtual double Wait(const TimerStart& start, double minimumMilliseconds) = 0;
    virtual void Shutdown() = 0;
};
ITimer& GetTimer();
} // namespace Platform
