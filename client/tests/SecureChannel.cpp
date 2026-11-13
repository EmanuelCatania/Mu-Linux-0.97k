// Fase 5, C4: canal cifrado entre el cliente y el GameServer.  Las dos puntas
// corren en el mismo proceso y se pasan los bytes a mano, así se puede cortar,
// alterar, repetir y reordenar lo que "viaja".

#include "net/SecureChannel.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

extern "C" {
#include "net/crypto/monocypher.h"
}

namespace {

int s_Failures = 0;

void Require(bool ok, const char* what)
{
    if (!ok) {
        std::printf("FALLA: %s\n", what);
        ++s_Failures;
    }
}

using Bytes = std::vector<std::uint8_t>;

struct Identity {
    std::uint8_t secret[64];
    std::uint8_t pub[32];
};

Identity MakeIdentity(std::uint8_t fill)
{
    Identity id;
    std::uint8_t seed[32];
    std::memset(seed, fill, sizeof(seed));
    crypto_eddsa_key_pair(id.secret, id.pub, seed);
    return id;
}

void Fill(std::uint8_t out[32], std::uint8_t value) { std::memset(out, value, 32); }

Bytes Seal(SecureChannel& from, const Bytes& plain)
{
    Bytes record(plain.size() + SecureChannel::Overhead);
    const std::size_t n = from.Seal(plain.data(), plain.size(), record.data());
    record.resize(n);
    return record;
}

// Saludo completo.  `trustResult` es lo que contesta el cliente sobre la identidad.
bool Handshake(SecureChannel& server, SecureChannel& client, const Identity& id, bool trustResult = true)
{
    std::uint8_t random[32];
    Bytes hello(SecureChannel::ServerHelloSize);
    Fill(random, 0xA1);
    server.BeginServer(hello.data(), random, id.secret, id.pub, nullptr, nullptr);
    Fill(random, 0xB2);
    client.BeginClient(random, [trustResult](const std::uint8_t*, const std::uint8_t*, const std::uint8_t*) { return trustResult; });
    Bytes plain;
    if (!client.Receive(hello.data(), hello.size(), plain)) return false;
    std::uint8_t reply[SecureChannel::ClientHelloSize];
    const std::size_t replySize = client.TakeReply(reply);
    if (replySize != SecureChannel::ClientHelloSize) return false;
    if (!server.Receive(reply, replySize, plain)) return false;
    return server.IsReady() && client.IsReady() && plain.empty();
}

} // namespace

int main()
{
    const Identity id = MakeIdentity(0x11);
    const Identity impostor = MakeIdentity(0x22);

    // ── Saludo y datos en los dos sentidos ──────────────────────────────────
    {
        SecureChannel server, client;
        Require(Handshake(server, client, id), "el saludo completa");
        Require(std::memcmp(client.PeerIdentity(), id.pub, 32) == 0, "el cliente ve la identidad del server");

        const Bytes a = { 0xC1, 0x04, 0xF1, 0x00 };
        const Bytes b(3000, 0x5A);
        Bytes got;
        Bytes wire = Seal(server, a);
        Require(wire.size() == a.size() + SecureChannel::Overhead, "el registro agrega 18 bytes");
        Require(std::memcmp(wire.data() + 2, a.data(), a.size()) != 0, "el contenido no viaja en claro");
        Require(client.Receive(wire.data(), wire.size(), got) && got == a, "server -> cliente");
        got.clear();
        wire = Seal(client, b);
        Require(server.Receive(wire.data(), wire.size(), got) && got == b, "cliente -> server");

        // Varios registros en una sola lectura y uno partido byte a byte.
        got.clear();
        Bytes many = Seal(server, a);
        const Bytes second = Seal(server, b);
        many.insert(many.end(), second.begin(), second.end());
        Require(client.Receive(many.data(), many.size(), got), "dos registros juntos");
        Bytes expected = a;
        expected.insert(expected.end(), b.begin(), b.end());
        Require(got == expected, "dos registros juntos: contenido");

        got.clear();
        wire = Seal(server, b);
        bool ok = true;
        for (std::size_t i = 0; i < wire.size(); ++i) ok = ok && client.Receive(&wire[i], 1, got);
        Require(ok && got == b, "un registro que llega de a un byte");

        // Registro vacío.
        got.clear();
        wire = Seal(server, Bytes());
        Require(wire.size() == SecureChannel::Overhead && client.Receive(wire.data(), wire.size(), got) && got.empty(), "registro vacío");
    }

    // ── El saludo también puede llegar partido ──────────────────────────────
    {
        SecureChannel server, client;
        std::uint8_t random[32];
        Bytes hello(SecureChannel::ServerHelloSize), plain;
        Fill(random, 1);
        server.BeginServer(hello.data(), random, id.secret, id.pub, nullptr, nullptr);
        Fill(random, 2);
        client.BeginClient(random, [](const std::uint8_t*, const std::uint8_t*, const std::uint8_t*) { return true; });
        bool ok = true;
        for (std::size_t i = 0; i < hello.size(); ++i) ok = ok && client.Receive(&hello[i], 1, plain);
        std::uint8_t reply[SecureChannel::ClientHelloSize];
        Require(ok && client.TakeReply(reply) == SecureChannel::ClientHelloSize, "ServerHello de a un byte");
        // El ClientHello y el primer registro del cliente en la misma lectura.
        const Bytes first = { 1, 2, 3 };
        Bytes wire(reply, reply + sizeof(reply));
        const Bytes record = Seal(client, first);
        wire.insert(wire.end(), record.begin(), record.end());
        Require(server.Receive(wire.data(), wire.size(), plain) && plain == first, "ClientHello + registro juntos");
    }

    // ── Alteraciones ────────────────────────────────────────────────────────
    const Bytes message(64, 0x33);
    for (std::size_t flip : { (std::size_t)0, (std::size_t)1, (std::size_t)10, message.size() + 2, message.size() + 17 }) {
        SecureChannel server, client;
        Handshake(server, client, id);
        Bytes wire = Seal(server, message), got;
        wire[flip] ^= 0x01;
        // Con el largo cambiado puede quedar esperando más bytes: se completa.
        wire.resize(wire.size() + 600, 0);
        Require(!client.Receive(wire.data(), wire.size(), got) && got.empty(), "un bit cambiado se rechaza");
        Require(client.GetState() == SecureChannel::State::Failed, "tras el rechazo el canal queda inutilizable");
        Bytes again = Seal(server, message);
        Require(!client.Receive(again.data(), again.size(), got), "un canal fallado no vuelve a aceptar datos");
    }
    {
        // Repetido.
        SecureChannel server, client;
        Handshake(server, client, id);
        Bytes wire = Seal(server, message), got;
        Require(client.Receive(wire.data(), wire.size(), got), "primer envío");
        Require(!client.Receive(wire.data(), wire.size(), got), "el mismo registro repetido se rechaza");
    }
    {
        // Reordenado.
        SecureChannel server, client;
        Handshake(server, client, id);
        const Bytes first = Seal(server, message), second = Seal(server, message);
        Bytes got;
        Require(!client.Receive(second.data(), second.size(), got), "un registro fuera de orden se rechaza");
    }
    {
        // Reflejado: lo que mandó el cliente no vale como si viniera del server.
        SecureChannel server, client;
        Handshake(server, client, id);
        const Bytes wire = Seal(client, message);
        Bytes got;
        Require(!client.Receive(wire.data(), wire.size(), got), "un registro propio devuelto se rechaza");
    }
    {
        // De otra sesión.
        SecureChannel server1, client1, server2, client2;
        Handshake(server1, client1, id);
        std::uint8_t random[32];
        Bytes hello(SecureChannel::ServerHelloSize), plain;
        Fill(random, 0x77);
        server2.BeginServer(hello.data(), random, id.secret, id.pub, nullptr, nullptr);
        Fill(random, 0x78);
        client2.BeginClient(random, [](const std::uint8_t*, const std::uint8_t*, const std::uint8_t*) { return true; });
        client2.Receive(hello.data(), hello.size(), plain);
        std::uint8_t reply[SecureChannel::ClientHelloSize];
        client2.TakeReply(reply);
        server2.Receive(reply, sizeof(reply), plain);
        const Bytes wire = Seal(server1, message);
        Bytes got;
        Require(!client2.Receive(wire.data(), wire.size(), got), "un registro de otra sesión se rechaza");
    }

    // ── Identidad ───────────────────────────────────────────────────────────
    {
        SecureChannel server, client;
        Require(!Handshake(server, client, id, false), "sin confianza en la identidad no hay canal");
        Require(client.GetState() == SecureChannel::State::Failed, "el cliente queda en fallo");
    }
    {
        // El saludo trae la identidad real pero lo firmó otra clave.
        SecureChannel server, client;
        std::uint8_t random[32];
        Bytes hello(SecureChannel::ServerHelloSize), plain;
        Fill(random, 5);
        server.BeginServer(hello.data(), random, impostor.secret, id.pub, nullptr, nullptr);
        Fill(random, 6);
        bool asked = false;
        client.BeginClient(random, [&asked](const std::uint8_t*, const std::uint8_t*, const std::uint8_t*) { asked = true; return true; });
        Require(!client.Receive(hello.data(), hello.size(), plain), "firma de otra clave: se rechaza");
        Require(!asked, "con la firma mal ni se consulta la confianza");
    }
    {
        // Alguien en el medio cambia la clave efímera del server.
        SecureChannel server, client;
        std::uint8_t random[32];
        Bytes hello(SecureChannel::ServerHelloSize), plain;
        Fill(random, 7);
        server.BeginServer(hello.data(), random, id.secret, id.pub, nullptr, nullptr);
        hello[4 + 3] ^= 0x10;
        Fill(random, 8);
        client.BeginClient(random, [](const std::uint8_t*, const std::uint8_t*, const std::uint8_t*) { return true; });
        Require(!client.Receive(hello.data(), hello.size(), plain), "clave efímera cambiada: se rechaza");
    }
    {
        // La rotación viaja en el saludo y llega al que decide la confianza.
        SecureChannel server, client;
        std::uint8_t random[32], rotation[64];
        std::memset(rotation, 0x5C, sizeof(rotation));
        Bytes hello(SecureChannel::ServerHelloSize), plain;
        Fill(random, 9);
        server.BeginServer(hello.data(), random, id.secret, id.pub, impostor.pub, rotation);
        Fill(random, 10);
        bool seen = false;
        client.BeginClient(random, [&](const std::uint8_t* identity, const std::uint8_t* previous, const std::uint8_t* signature) {
            seen = previous && signature && std::memcmp(identity, id.pub, 32) == 0 &&
                   std::memcmp(previous, impostor.pub, 32) == 0 && std::memcmp(signature, rotation, 64) == 0;
            return true;
        });
        Require(client.Receive(hello.data(), hello.size(), plain) && seen, "la rotación llega al control de confianza");
    }
    {
        // Saludos mal formados.
        SecureChannel server, client;
        std::uint8_t random[32];
        Bytes hello(SecureChannel::ServerHelloSize), plain;
        Fill(random, 11);
        server.BeginServer(hello.data(), random, id.secret, id.pub, nullptr, nullptr);
        const Bytes junk(SecureChannel::ClientHelloSize, 0xC1);
        Require(!server.Receive(junk.data(), junk.size(), plain), "el server rechaza un ClientHello sin la marca");

        // Clave efímera de orden bajo (todo ceros): el secreto compartido sería cero.
        SecureChannel server2;
        Fill(random, 12);
        server2.BeginServer(hello.data(), random, id.secret, id.pub, nullptr, nullptr);
        Bytes weak = { 'M', 'U', 'C', '1' };
        weak.resize(SecureChannel::ClientHelloSize, 0);
        Require(!server2.Receive(weak.data(), weak.size(), plain), "el server rechaza una clave efímera nula");

        Fill(random, 13);
        client.BeginClient(random, [](const std::uint8_t*, const std::uint8_t*, const std::uint8_t*) { return true; });
        const Bytes legacy(SecureChannel::ServerHelloSize, 0xC1);
        Require(!client.Receive(legacy.data(), legacy.size(), plain), "el cliente rechaza un server sin saludo");
    }
    {
        // Antes del saludo no se puede cifrar, y un texto demasiado largo tampoco.
        SecureChannel idle;
        std::uint8_t out[64];
        const std::uint8_t byte = 1;
        Require(idle.Seal(&byte, 1, out) == 0, "sin canal no se cifra");
        SecureChannel server, client;
        Handshake(server, client, id);
        const Bytes huge(SecureChannel::MaxPlain + 1, 0);
        Bytes record(huge.size() + SecureChannel::Overhead);
        Require(server.Seal(huge.data(), huge.size(), record.data()) == 0, "más que MaxPlain no se cifra");
    }

    if (s_Failures == 0) std::printf("SecureChannel: OK\n");
    return s_Failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
