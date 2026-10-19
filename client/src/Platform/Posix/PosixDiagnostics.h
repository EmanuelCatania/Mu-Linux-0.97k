#pragma once
#include "Platform/IDiagnostics.h"
#include <csignal>
#include <unistd.h>
#include <fcntl.h>
#include <execinfo.h>
#include <chrono>
#include <thread>
#include <cpuid.h>
#include <x86intrin.h>
namespace Platform {
class PosixDiagnostics final : public IDiagnostics {
    inline static int descriptor = -1;
    inline static volatile sig_atomic_t handling = 0;
    inline static unsigned char alternateStack[65536];
    static void Handle(int signal) {
        if (handling) _exit(128 + signal);
        handling = 1;
        const char message[] = "\nCRASH: señal fatal; backtrace (direcciones/símbolos):\n";
        (void)::write(descriptor, message, sizeof(message)-1);
        void* frames[64];
        const int count = ::backtrace(frames, 64);
        ::backtrace_symbols_fd(frames, count, descriptor);
        _exit(128 + signal);
    }
public:
    std::int64_t MeasureClockRate(std::uint32_t milliseconds) override {
        // El port Linux no modifica prioridad ni afinidad. Esto mide TSC,
        // no la frecuencia dinamica del nucleo en CPUs con TSC invariante.
        unsigned int a, b, c, d;
        if (!__get_cpuid(1, &a, &b, &c, &d) || !(d & 0x10)) return 0;
        __cpuid(0, a, b, c, d);
        const auto start = std::chrono::steady_clock::now();
        const auto ticks = __rdtsc();
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
        const auto end = std::chrono::steady_clock::now();
        const auto elapsedTicks = __rdtsc() - ticks;
        const double seconds = std::chrono::duration<double>(end - start).count();
        return seconds > 0 ? static_cast<std::int64_t>(elapsedTicks / seconds) : 0;
    }
    bool Install(DiagnosticLog log) override {
        if (descriptor >= 0) return true;
        const int fd = ::open("debug.log", O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC, 0600);
        if (fd < 0) { if (log) log("No se pudo abrir debug.log para señales"); return false; }
        // Precargar el unwinder antes del crash. El backtrace es de mejor esfuerzo:
        // no se garantiza recuperar una pila corrupta ni locks internos del unwinder.
        void* warm[1]; (void)::backtrace(warm, 1);
        stack_t stack{}; stack.ss_sp = alternateStack; stack.ss_size = sizeof(alternateStack);
        if (::sigaltstack(&stack, nullptr) != 0) { ::close(fd); return false; }
        struct sigaction action{}; action.sa_handler = Handle;
        ::sigemptyset(&action.sa_mask); action.sa_flags = SA_ONSTACK | SA_RESETHAND;
        constexpr int signals[] = {SIGSEGV, SIGBUS, SIGILL, SIGFPE, SIGABRT};
        struct sigaction previous[5]{};
        descriptor = fd;
        for (int i = 0; i < 5; ++i) {
            if (::sigaction(signals[i], &action, &previous[i]) != 0) {
                for (int j = 0; j < i; ++j) ::sigaction(signals[j], &previous[j], nullptr);
                ::close(fd); descriptor = -1;
                if (log) log("No se pudieron instalar los manejadores de señales");
                return false;
            }
        }
        return true;
    }
};
}
