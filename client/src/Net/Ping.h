#pragma once
#include <windows.h>

// RTT de la conexión MU: eco del TickCount de 0x0E, sin tráfico adicional.
// Todos los accesos ocurren en el hilo de ventana/red del cliente.
class CPing {
public:
    void Reset();
    void RecordSend(DWORD tick);
    void Receive(DWORD tick);
    bool GetMilliseconds(DWORD& value) const;
private:
    static constexpr unsigned int Capacity = 8;
    static constexpr double TimeoutSeconds = 5.0;
    struct Pending {
        DWORD tick = 0;
        LARGE_INTEGER sent{};
        bool active = false;
    };
    Pending m_Pending[Capacity]{};
    unsigned int m_Next = 0;
    LARGE_INTEGER m_Frequency{};
    LARGE_INTEGER m_LastReply{};
    DWORD m_Milliseconds = 0;
    bool m_HasSample = false;
};
extern CPing gPing;
