#include "Platform/ITimer.h"
#include "Platform/LocalTime.h"
#include <SDL3/SDL.h>
#ifdef _WIN32
#include <windows.h>
#endif
namespace Platform {
namespace {
class SDLTimer final : public ITimer {
public:
#ifdef _WIN32
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
#else
    int64_t CounterFrequency() override { return static_cast<int64_t>(SDL_GetPerformanceFrequency()); }
    bool ReadCounter(int64_t& counter) override {
        counter = static_cast<int64_t>(SDL_GetPerformanceCounter());
        return CounterFrequency() > 0;
    }
#endif
    LocalDateTime LocalTime() override { return ReadLocalTime(); }
    // Origen monotónico SDL; conserva el wrap de 32 bits usado por el cliente.
    uint32_t Ticks() override { return static_cast<uint32_t>(SDL_GetTicks()); }
    uint32_t MultimediaTicks() override { return Ticks(); }
    void SleepMilliseconds(uint32_t milliseconds) override { SDL_Delay(milliseconds); }
    TimerStart Start() override { return {static_cast<int64_t>(SDL_GetPerformanceCounter()), Ticks()}; }
    double Wait(const TimerStart& start, double minimumMilliseconds) override {
        const auto frequency = SDL_GetPerformanceFrequency();
        for (;;) {
            const double elapsed = (SDL_GetPerformanceCounter() - static_cast<uint64_t>(start.counter)) * 1000.0 / frequency;
            if (elapsed >= minimumMilliseconds) return elapsed;
            SDL_DelayPrecise(static_cast<uint64_t>((minimumMilliseconds - elapsed) * 1000000.0));
        }
    }
    void Shutdown() override {}
};
}
ITimer& GetTimer() { static auto* timer = new SDLTimer; return *timer; }
}
