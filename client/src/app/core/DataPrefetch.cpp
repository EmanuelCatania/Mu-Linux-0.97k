#include "stdafx.h"
#include "app/core/DataPrefetch.h"
#include "platform/IFileSystem.h"
#include <atomic>
#include <cstdio>
#include <string>
#include <thread>
#ifdef _WIN32
#include <windows.h>
#elif defined(__linux__)
#include <sys/resource.h>
#include <sys/syscall.h>
#include <unistd.h>
#endif

// DESVIACION: el cliente lee texturas y modelos recién cuando aparece algo
// nuevo en pantalla. En disco mecánico una lectura en frío tarda 10-50 ms por
// archivo y el juego se traba. Este hilo lee Data/ una vez al iniciar, con
// prioridad baja de CPU y de disco, para que el sistema lo deje en caché.
namespace {
class CDataPrefetch {
public:
    ~CDataPrefetch()
    {
        m_Stop = true;
        if (m_Thread.joinable()) m_Thread.join();
    }
    void Start(const char* root)
    {
        if (m_Thread.joinable()) return;
        // Fase 5 (E2): el recorrido de Data incluye las capas (packs y
        // Content/, adonde se va mudando Data/).
        m_Thread = std::thread([this, path = std::string(root)] { Run(path); });
    }

private:
    void Run(const std::string& root)
    {
#ifdef _WIN32
        // Modo de fondo: baja también la prioridad de E/S, así no le compite
        // al juego por el disco.
        SetThreadPriority(GetCurrentThread(), THREAD_MODE_BACKGROUND_BEGIN);
#elif defined(__linux__)
        // Equivalente en Linux: clase de E/S idle (el disco sólo atiende este
        // hilo cuando nadie más pide) y nice 19. Las dos se aplican por hilo.
        const long tid = syscall(SYS_gettid);
        constexpr int kIoprioWhoProcess = 1, kIoprioClassIdle = 3;
        syscall(SYS_ioprio_set, kIoprioWhoProcess, 0, kIoprioClassIdle << 13);
        setpriority(PRIO_PROCESS, static_cast<id_t>(tid), 19);
#endif
        static char buffer[64 * 1024];
        Platform::GetFileSystem().WalkFiles(root.c_str(), [this](const std::string& path) {
            // La ruta ya es la real: se abre directo, sin volver a pasar por las capas.
            if (FILE* file = std::fopen(path.c_str(), "rb")) {
                while (!m_Stop && std::fread(buffer, 1, sizeof(buffer), file) == sizeof(buffer)) {}
                std::fclose(file);
            }
            return !m_Stop;
        });
#ifdef _WIN32
        SetThreadPriority(GetCurrentThread(), THREAD_MODE_BACKGROUND_END);
#endif
    }
    std::thread m_Thread;
    std::atomic<bool> m_Stop{false};
};
CDataPrefetch s_Prefetch;
}

void StartDataPrefetch(const char* root)
{
    s_Prefetch.Start(root);
}
