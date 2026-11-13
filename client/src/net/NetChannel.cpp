#include "stdafx.h"
#include "net/NetChannel.h"
#include "net/Net.h"
#include "net/SecureChannel.h"
#include "net/ServerTrust.h"
#include "net/crypto/SecureRandom.h"
#include "app/core/Window.h"
#include "assets/text/ClientText.h"
#include "platform/ISocket.h"
#include "platform/IWindow.h"
#include "platform/Text.h"
#include <vector>

// Acá se usa el send real del socket, no el macro de stdafx.h (que vuelve a
// entrar por NetChannel::Send).
#ifdef send
#undef send
#endif

extern "C" void DbgLogPublic(const char* msg);

namespace NetChannel {

namespace {

const int kQueueSize = 0x2000;          // tamaño de SocketClientSendBuffer
const std::size_t kEarlyLimit = 0x10000;

bool s_Secure = false;
SecureChannel s_Channel;
std::vector<unsigned char> s_Plain;     // texto claro recibido, sin entregar
std::vector<unsigned char> s_Early;     // lo que se quiso mandar antes de terminar el saludo
std::vector<unsigned char> s_Record;

bool Close()
{
    Net_Disconnect(SocketClient);
    return false;
}

// Bytes al socket tal cual, respetando la cola: si hay algo pendiente, lo nuevo
// va detrás.  false = se cerró la conexión.
bool Write(const unsigned char* data, int len)
{
    if (len <= 0) return true;
    if (!Platform::GetSocket().IsOpen(SocketClientSocket)) return false;
    int sent = 0;
    if (SocketClientSendBufferLength == 0 && Platform::GetSocket().IsConnected(SocketClientSocket)) {
        while (sent < len) {
            const int n = Platform::GetSocket().Send(SocketClientSocket, (const char*)data + sent, len - sent, 0);
            if (n > 0) { sent += n; continue; }
            if (n == -1 && Platform::GetSocket().LastError() == Platform::SocketWouldBlock) break;
            return Close();
        }
    }
    const int rest = len - sent;
    if (rest == 0) return true;
    if (rest > kQueueSize - (int)SocketClientSendBufferLength) return Close();
    memcpy(SocketClientSendBuffer + SocketClientSendBufferLength, data + sent, rest);
    SocketClientSendBufferLength += rest;
    return true;
}

// Cifra y manda.  Parte en registros si no entra en uno.
bool WriteSealed(const unsigned char* data, int len)
{
    while (len > 0) {
        const int part = len > (int)SecureChannel::MaxPlain ? (int)SecureChannel::MaxPlain : len;
        s_Record.resize(part + SecureChannel::Overhead);
        const std::size_t size = s_Channel.Seal(data, part, s_Record.data());
        if (size == 0 || !Write(s_Record.data(), (int)size)) return false;
        data += part;
        len -= part;
    }
    return true;
}

bool TrustServer(const std::uint8_t identity[32], const std::uint8_t* previous, const std::uint8_t* rotation)
{
    const ServerTrust::Result result = ServerTrust::Check(identity, previous, rotation);
    if (result == ServerTrust::Result::FirstUse) DbgLogPublic("NET: canal: primer contacto, se fija la clave del server");
    if (result == ServerTrust::Result::Rotated) DbgLogPublic("NET: canal: el server rotó su clave, se fija la nueva");
    return result != ServerTrust::Result::Mismatch;
}

// El server rechazó la conexión antes del saludo (lleno, límite por IP): manda
// un F1:01 en claro y corta.  No está autenticado, pero sólo sirve para mostrar
// el motivo del rechazo.
bool IsPlainRefusal(const unsigned char* raw, int size)
{
    return size >= 5 && raw[0] == 0xC1 && raw[1] == size && raw[2] == 0xF1 && raw[3] == 0x01;
}

} // namespace

void OnNewConnection(bool secure)
{
    s_Secure = secure;
    s_Channel.Reset();
    s_Plain.clear();
    s_Early.clear();
    // Lo que haya quedado en la cola es de la conexión anterior (el binario nunca
    // la vaciaba).  En el socket nuevo saldría antes que el saludo y el server
    // cerraría la conexión.
    SocketClientSendBufferLength = 0;
    if (!secure) return;
    std::uint8_t random[SecureChannel::KeySize];
    if (!SecureRandomBytes(random, sizeof(random))) {
        // Sin bytes aleatorios no hay canal: el saludo va a fallar y se cierra.
        DbgLogPublic("NET: canal: el sistema no dio bytes aleatorios");
        return;
    }
    s_Channel.BeginClient(random, TrustServer);
}

int Send(const char* data, int len)
{
    if (len <= 0) return len;
    const unsigned char* bytes = (const unsigned char*)data;
    if (!s_Secure) return Write(bytes, len) ? len : -1;
    if (!s_Channel.IsReady()) {
        // Todavía no terminó el saludo: se guarda y sale apenas haya claves.
        if (s_Channel.GetState() != SecureChannel::State::AwaitServerHello || s_Early.size() + len > kEarlyLimit) {
            Close();
            return -1;
        }
        s_Early.insert(s_Early.end(), bytes, bytes + len);
        return len;
    }
    return WriteSealed(bytes, len) ? len : -1;
}

namespace {

// Bytes recién leídos del socket.  El texto claro se agrega a s_Plain.
// false = la conexión no sigue (ya se avisó o se cerró).
bool Receive(const unsigned char* raw, int size)
{
    if (size <= 0) return true;
    if (!s_Secure) {
        s_Plain.insert(s_Plain.end(), raw, raw + size);
        return true;
    }
    const bool wasReady = s_Channel.IsReady();
    if (!s_Channel.Receive(raw, (std::size_t)size, s_Plain)) {
        const SecureChannel::Failure why = s_Channel.GetFailure();
        char line[96];
        Platform::TextFormat(line, "NET: canal rechazado (motivo %d)", (int)why);
        DbgLogPublic(line);
        if (why == SecureChannel::Failure::NotThisProtocol && !wasReady) {
            if (IsPlainRefusal(raw, size)) {
                s_Secure = false;
                s_Plain.assign(raw, raw + size);
                return true;
            }
            // El server no habla este protocolo: mismo cartel que la versión
            // incorrecta, que al confirmarlo cierra el cliente.
            SetErrorMessage(0x1a);
        } else if (why == SecureChannel::Failure::Untrusted || why == SecureChannel::Failure::BadSignature) {
            Platform::GetWindow().ShowError(gClientText.Get(ClientTextId::ServerKeyMismatch), "Mu Online", true, true);
            gWindow.RequestCloseApplication();
        }
        return Close();
    }
    // El saludo del cliente sale en claro, antes que cualquier registro.
    std::uint8_t reply[SecureChannel::ClientHelloSize];
    const std::size_t replySize = s_Channel.TakeReply(reply);
    if (replySize != 0 && !Write(reply, (int)replySize)) return false;
    if (!wasReady && s_Channel.IsReady()) {
        DbgLogPublic("NET: canal cifrado listo");
        if (!s_Early.empty()) {
            const std::vector<unsigned char> early = std::move(s_Early);
            s_Early.clear();
            if (!WriteSealed(early.data(), (int)early.size())) return Close();
        }
    }
    return true;
}

} // namespace

int PendingPlain() { return (int)s_Plain.size(); }

bool IsQueued(const char* data)
{
    return data >= SocketClientSendBuffer && data < SocketClientSendBuffer + kQueueSize;
}

int Read(std::uintptr_t socket, char* out, int capacity, int flags)
{
    if (s_Plain.empty()) {
        static unsigned char raw[0x2000];
        const int got = Platform::GetSocket().Receive(socket, (char*)raw, (int)sizeof(raw), flags);
        if (got <= 0) return got;
        if (!Receive(raw, got)) return -1;
    }
    if (capacity <= 0 || s_Plain.empty()) return -1;
    const int take = (int)s_Plain.size() < capacity ? (int)s_Plain.size() : capacity;
    memcpy(out, s_Plain.data(), take);
    s_Plain.erase(s_Plain.begin(), s_Plain.begin() + take);
    return take;
}

} // namespace NetChannel
