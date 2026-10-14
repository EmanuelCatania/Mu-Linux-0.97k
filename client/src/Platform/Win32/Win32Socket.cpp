#include "stdafx.h"
#include "Platform/ISocket.h"
// El cifrado pertenece a MuEmu_send_hook, no al transporte.
#undef send
namespace Platform {
namespace {
class Win32Socket final : public ISocket {
public:
    int Startup(uint16_t requested, uint16_t& actual) override {
        WSADATA data;
        int result = WSAStartup(requested, &data);
        if (result == 0) actual = data.wVersion;
        return result;
    }
    int Cleanup() override { return WSACleanup(); }
    SocketHandle CreateTcp(int protocol) override { return socket(AF_INET, SOCK_STREAM, protocol); }
    bool ResolveIPv4(const char* name, uint32_t& address) override {
        address = inet_addr(name);
        if (address != INADDR_NONE) return true;
        HOSTENT* host = gethostbyname(name);
        if (!host || !host->h_addr_list || !host->h_addr_list[0]) return false;
        memcpy(&address, host->h_addr_list[0], sizeof(address));
        return true;
    }
    int ConnectIPv4(SocketHandle socket, uint32_t address, uint16_t port) override {
        sockaddr_in target = {};
        target.sin_family = AF_INET;
        target.sin_port = htons(port);
        target.sin_addr.s_addr = address;
        return connect(socket, reinterpret_cast<const sockaddr*>(&target), sizeof(target));
    }
    int NotifyWindow(SocketHandle socket, void* window, uint32_t message, int32_t mask) override {
        return WSAAsyncSelect(socket, static_cast<HWND>(window), message, mask);
    }
    int Send(SocketHandle socket, const char* data, int size, int flags) override {
        return ::send(socket, data, size, flags);
    }
    int Receive(SocketHandle socket, char* data, int capacity, int flags) override {
        return recv(socket, data, capacity, flags);
    }
    int Close(SocketHandle socket) override { return closesocket(socket); }
    int LastError() override { return WSAGetLastError(); }
};
}
ISocket& GetSocket() { static Win32Socket transport; return transport; }
}
