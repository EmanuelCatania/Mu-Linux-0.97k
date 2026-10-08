#include "stdafx.h"
#include "Net/Ping.h"
#include <iphlpapi.h>
#include <icmpapi.h>
#include <chrono>
#include <system_error>

CPing gPing;

CPing::~CPing()
{
    Stop();
}

void CPing::SetServer(SOCKET socket)
{
    sockaddr_in peer{};
    int size = sizeof(peer);
    const bool connected = getpeername(socket, (sockaddr*)&peer, &size) == 0 &&
                           peer.sin_family == AF_INET;
    std::lock_guard<std::mutex> lock(m_Mutex);
    ++m_Generation;
    m_Address = connected ? peer.sin_addr.s_addr : 0;
    m_HasSample = false;
    if (m_Address && !m_Worker.joinable()) {
        m_Stop = false;
        try {
            m_Worker = std::thread(&CPing::Run, this);
        } catch (const std::system_error&) {
            m_Address = 0;
        }
    }
    m_Wake.notify_all();
}

void CPing::Reset()
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    ++m_Generation;
    m_Address = 0;
    m_HasSample = false;
    m_Wake.notify_all();
}

void CPing::Stop()
{
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_Stop = true;
        m_Address = 0;
        m_HasSample = false;
        m_Wake.notify_all();
    }
    if (m_Worker.joinable()) m_Worker.join();
}

bool CPing::GetMilliseconds(DWORD& value) const
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (!m_HasSample) return false;
    value = m_Milliseconds;
    return true;
}

void CPing::Run()
{
    std::unique_lock<std::mutex> lock(m_Mutex);
    while (!m_Stop) {
        m_Wake.wait(lock, [this] { return m_Stop || m_Address != 0; });
        if (m_Stop) break;
        const DWORD address = m_Address;
        const unsigned int generation = m_Generation;
        lock.unlock();

        // DLL CPingSystem::SetPing: timeout de 1000 ms, fuera del hilo del juego.
        bool valid = false;
        DWORD milliseconds = 0;
        HANDLE handle = IcmpCreateFile();
        if (handle != INVALID_HANDLE_VALUE) {
            char payload[32] = "Data Buffer";
            alignas(ICMP_ECHO_REPLY) BYTE buffer[sizeof(ICMP_ECHO_REPLY) + sizeof(payload) + 8]{};
            const DWORD count = IcmpSendEcho(handle, address, payload, sizeof(payload),
                                             nullptr, buffer, sizeof(buffer), 1000);
            const auto* reply = (const ICMP_ECHO_REPLY*)buffer;
            valid = count != 0 && reply->Status == IP_SUCCESS;
            if (valid) milliseconds = reply->RoundTripTime;
            IcmpCloseHandle(handle);
        }

        lock.lock();
        // Descartar resultados de una conexión anterior, incluso a la misma IP.
        if (!m_Stop && generation == m_Generation) {
            m_HasSample = valid;
            m_Milliseconds = milliseconds;
        }
        m_Wake.wait_for(lock, std::chrono::milliseconds(1000), [this, generation] {
            return m_Stop || generation != m_Generation;
        });
    }
}
