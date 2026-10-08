#pragma once
#include <winsock2.h>
#include <condition_variable>
#include <mutex>
#include <thread>

// PingSystem.cpp del DLL: eco ICMP al GameServer, independiente del protocolo MU.
class CPing
{
public:
    ~CPing();
    void SetServer(SOCKET socket);
    void Reset();
    void Stop();
    bool GetMilliseconds(DWORD& value) const;

private:
    void Run();
    mutable std::mutex m_Mutex;
    std::condition_variable m_Wake;
    std::thread m_Worker;
    DWORD m_Address = 0;
    unsigned int m_Generation = 0;
    DWORD m_Milliseconds = 0;
    bool m_HasSample = false;
    bool m_Stop = false;
};

extern CPing gPing;
