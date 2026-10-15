#include "Platform/ITimer.h"
#include "Platform/LocalTime.h"
#include <SDL3/SDL.h>
namespace Platform {
namespace {
class SDLTimer final : public ITimer {
public:
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
