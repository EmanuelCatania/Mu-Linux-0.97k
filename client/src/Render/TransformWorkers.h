#pragma once
#include <algorithm>
#include <array>
#include <cfenv>
#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <thread>
#include <xmmintrin.h>

namespace TransformWorkers {
// DESVIACION: equipo acotado y persistente para matemáticas puras. El llamador
// participa y espera antes de reutilizar los datos. No admite llamadas anidadas
// ni concurrentes; pertenece al hilo principal del juego.
class Team {
public:
    using Work = void (*)(void*, std::size_t, std::size_t);
    Team() {
        const unsigned hw = std::thread::hardware_concurrency();
        const unsigned wanted = hw > 1 ? (std::min)(3u, hw - 1) : 0;
        try {
            for (; count_ < wanted; ++count_)
                threads_[count_] = std::thread([this, id = count_] { Worker(id); });
        } catch (...) { /* La ruta restante sigue funcionando sin más hilos. */ }
    }
    ~Team() {
        { std::lock_guard<std::mutex> lock(mutex_); stop_ = true; }
        ready_.notify_all();
        for (unsigned i = 0; i < count_; ++i) threads_[i].join();
    }
    Team(const Team&) = delete;
    Team& operator=(const Team&) = delete;
    unsigned Workers() const { return count_; }
    void Run(std::size_t size, void* context, Work work) {
        if (!count_) { work(context, 0, size); return; }
        {
            std::lock_guard<std::mutex> lock(mutex_);
            size_ = size; context_ = context; work_ = work;
            std::fegetenv(&environment_); mxcsr_ = _mm_getcsr();
            pending_ = count_; ++generation_;
        }
        ready_.notify_all();
        work(context, 0, size / (count_ + 1));
        std::unique_lock<std::mutex> lock(mutex_);
        done_.wait(lock, [this] { return pending_ == 0; });
    }
private:
    void Worker(unsigned id) {
        std::size_t seen = 0;
        std::unique_lock<std::mutex> lock(mutex_);
        for (;;) {
            ready_.wait(lock, [&] { return stop_ || generation_ != seen; });
            if (stop_) return;
            seen = generation_;
            const auto size = size_; const auto context = context_; const auto work = work_;
            const auto environment = environment_; const auto mxcsr = mxcsr_;
            const auto begin = size * (id + 1) / (count_ + 1);
            const auto end = size * (id + 2) / (count_ + 1);
            lock.unlock();
            // Los hilos nuevos no heredan necesariamente el modo FP del juego.
            std::fesetenv(&environment); _mm_setcsr(mxcsr);
            work(context, begin, end);
            lock.lock();
            if (--pending_ == 0) done_.notify_one();
        }
    }
    std::array<std::thread, 3> threads_;
    unsigned count_ = 0, pending_ = 0, mxcsr_ = 0;
    std::size_t generation_ = 0, size_ = 0;
    std::fenv_t environment_{};
    void* context_ = nullptr;
    Work work_ = nullptr;
    std::mutex mutex_;
    std::condition_variable ready_, done_;
    bool stop_ = false;
};
// Los lotes y la ruta de una malla grande se ejecutan en fases sucesivas.
// Compartir equipo evita conservar dos grupos de hilos inactivos.
inline Team& SharedTeam() { static Team team; return team; }

}
