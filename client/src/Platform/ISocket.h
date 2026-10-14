#pragma once
#include <cstdint>
namespace Platform {
using SocketHandle = uintptr_t;
constexpr SocketHandle InvalidSocket = static_cast<SocketHandle>(-1);
// Códigos heredados del transporte Win32; no se modifica la política de reintento.
constexpr int SocketWouldBlock = 10035;
class ISocket {
public:
    virtual ~ISocket() = default;
    virtual int Startup(uint16_t requested, uint16_t& actual) = 0;
    virtual int Cleanup() = 0;
    virtual SocketHandle CreateTcp(int protocol) = 0;
    virtual bool ResolveIPv4(const char* name, uint32_t& networkAddress) = 0;
    virtual int ConnectIPv4(SocketHandle socket, uint32_t networkAddress, uint16_t port) = 0;
    // Puente Win32 transitorio: máscara y mensaje originales, sin sondeo por frame.
    virtual int NotifyWindow(SocketHandle socket, void* window, uint32_t message, int32_t mask) = 0;
    virtual int Send(SocketHandle socket, const char* data, int size, int flags) = 0;
    virtual int Receive(SocketHandle socket, char* data, int capacity, int flags) = 0;
    virtual int Close(SocketHandle socket) = 0;
    virtual int LastError() = 0;
};
ISocket& GetSocket();
}
