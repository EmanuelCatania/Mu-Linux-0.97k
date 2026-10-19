#pragma once
#include <cstdint>
namespace Platform {
using SocketHandle = uintptr_t;
constexpr SocketHandle InvalidSocket = static_cast<SocketHandle>(-1);
// Códigos heredados del transporte Win32; no se modifica la política de reintento.
constexpr int SocketWouldBlock = 10035;
constexpr int SocketError = -1;
constexpr int TcpProtocol = 6;
constexpr int SocketReadEvent = 1, SocketWriteEvent = 2, SocketCloseEvent = 32;
using SocketEventSink = void (*)(uint16_t event, uint16_t error);
class ISocket {
public:
    virtual ~ISocket() = default;
    virtual void BindEvents(SocketEventSink) {}
    virtual void Poll() {}
    virtual bool IsConnected(SocketHandle) const { return true; } // Win32 conserva WSAAsyncSelect.
    // Consulta de handle; no implica conexión TCP completada.
    virtual bool IsOpen(SocketHandle socket) const = 0;
    virtual int Startup(uint16_t requested, uint16_t& actual) = 0;
    virtual int Cleanup() = 0;
    virtual SocketHandle CreateTcp(int protocol) = 0;
    virtual bool ResolveIPv4(const char* name, uint32_t& networkAddress) = 0;
    virtual int ConnectIPv4(SocketHandle socket, uint32_t networkAddress, uint16_t port) = 0;
    // Win32 registra notificaciones; poll conserva sólo la máscara del socket activo.
    virtual int NotifyWindow(SocketHandle socket, void* window, uint32_t message, int32_t mask) = 0;
    virtual int Send(SocketHandle socket, const char* data, int size, int flags) = 0;
    virtual int Receive(SocketHandle socket, char* data, int capacity, int flags) = 0;
    virtual int Close(SocketHandle socket) = 0;
    virtual int LastError() = 0;
};
ISocket& GetSocket();
}
