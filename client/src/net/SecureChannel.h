#pragma once

// Canal cifrado y autenticado entre el cliente y el GameServer (Fase 5, C4).
// Reemplaza al XOR de byte que se aplicaba a todo el tráfico: ese XOR no daba
// integridad (cualquiera en el medio podía cambiar bytes) y su clave salía de
// dos textos fijos del cliente.
//
// ESTE ARCHIVO ES EL MISMO EN EL CLIENTE Y EN EL SERVER
// (client/src/net/SecureChannel.* y server/Source/MuServer/Common/SecureChannel.*).
// Un cambio en uno va también al otro, y sube MU_PROTOCOL_VERSION.
//
// Saludo, en claro, al abrir la conexión:
//   server -> cliente  ServerHello (229 bytes):
//       "MUS1" | clave efímera X25519 (32) | clave de identidad (32)
//       | firma de la identidad sobre "MuHandshake1" + clave efímera (64)
//       | flags (1) | clave de identidad anterior (32) | firma de rotación (64)
//   cliente -> server  ClientHello (36 bytes):
//       "MUC1" | clave efímera X25519 (32)
// El cliente verifica la firma y decide si confía en la identidad (ServerTrust).
// Las dos puntas derivan una clave por sentido:
//   BLAKE2b(clave = X25519(efímeras), mensaje = "MuChannel1" + efímera del server
//           + efímera del cliente) -> 32 bytes server->cliente, 32 cliente->server
//
// Después todo viaja en registros:
//   largo del texto claro (2 bytes, little-endian) | cifrado | MAC (16 bytes)
// con XChaCha20-Poly1305.  El nonce es un contador por sentido, así un registro
// repetido, reordenado o de la otra dirección no pasa la verificación.  El largo
// entra como dato autenticado.
//
// La clase no usa sockets ni el sistema: recibe y entrega bytes.  Los 32 bytes
// aleatorios de la clave efímera los pasa quien llama.

#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

class SecureChannel
{
public:

	static constexpr std::size_t KeySize = 32;
	static constexpr std::size_t SignatureSize = 64;
	static constexpr std::size_t ServerHelloSize = 4 + 32 + 32 + 64 + 1 + 32 + 64;
	static constexpr std::size_t ClientHelloSize = 4 + 32;
	// Bytes que agrega un registro al texto claro: largo (2) + MAC (16).
	static constexpr std::size_t Overhead = 18;
	static constexpr std::size_t MaxPlain = 16384;

	enum class State { Idle, AwaitClientHello, AwaitServerHello, Ready, Failed };

	// Por qué quedó en Failed.
	enum class Failure
	{
		None,
		NotThisProtocol,  // la otra punta no mandó el saludo: es de otra versión
		BadSignature,     // el saludo no está firmado por la identidad que trae
		Untrusted,        // el cliente no confía en esa identidad
		BadKey,           // clave efímera inválida
		BadRecord         // registro alterado, repetido o fuera de orden
	};

	// El cliente decide si confía en la identidad que presenta el server.
	// previousKey/rotationSignature son nullptr si no vino rotación.
	using TrustFn = std::function<bool(const std::uint8_t identity[KeySize],
	                                   const std::uint8_t* previousKey,
	                                   const std::uint8_t* rotationSignature)>;

	SecureChannel();

	~SecureChannel();

	void Reset();

	State GetState() const { return this->m_State; }

	bool IsReady() const { return this->m_State == State::Ready; }

	Failure GetFailure() const { return this->m_Failure; }

	// ── Server ──────────────────────────────────────────────────────────────
	// Escribe el ServerHello en `out` (ServerHelloSize bytes) y queda esperando
	// el ClientHello.  previousKey/rotationSignature: nullptr si no hay rotación.
	void BeginServer(std::uint8_t* out, const std::uint8_t random[KeySize],
	                 const std::uint8_t identitySecret[64], const std::uint8_t identityPublic[KeySize],
	                 const std::uint8_t* previousKey, const std::uint8_t* rotationSignature);

	// ── Cliente ─────────────────────────────────────────────────────────────
	// Queda esperando el ServerHello.
	void BeginClient(const std::uint8_t random[KeySize], TrustFn trust);

	// Bytes en claro que hay que mandar tal cual (el ClientHello), si los hay.
	// Devuelve cuántos copió a `out` (hasta ClientHelloSize) y los descarta.
	std::size_t TakeReply(std::uint8_t* out);

	// Identidad que presentó el server (válida desde que el canal está listo).
	const std::uint8_t* PeerIdentity() const { return this->m_PeerIdentity; }

	// ── Las dos puntas ──────────────────────────────────────────────────────
	// Bytes que llegaron del socket.  Consume el saludo si falta y después los
	// registros completos; el texto claro se agrega a `plain`.  false = error
	// fatal (firma o MAC inválidos, saludo mal formado, identidad sin confianza):
	// hay que cerrar la conexión.
	bool Receive(const std::uint8_t* data, std::size_t size, std::vector<std::uint8_t>& plain);

	// Cifra `size` bytes en un registro.  `out` necesita size + Overhead bytes.
	// Devuelve el tamaño del registro, o 0 si el canal no está listo o el texto
	// es más largo que MaxPlain.
	std::size_t Seal(const std::uint8_t* plain, std::size_t size, std::uint8_t* out);

private:

	bool HandleHello();

	bool Fail(Failure why);

	void DeriveKeys(const std::uint8_t serverEphemeral[KeySize], const std::uint8_t clientEphemeral[KeySize], const std::uint8_t peerEphemeral[KeySize]);

	State m_State;

	Failure m_Failure;

	bool m_IsServer;

	std::uint8_t m_Secret[KeySize];      // efímera propia

	std::uint8_t m_Public[KeySize];

	std::uint8_t m_SendKey[KeySize];

	std::uint8_t m_ReceiveKey[KeySize];

	std::uint64_t m_SendCounter;

	std::uint64_t m_ReceiveCounter;

	std::uint8_t m_PeerIdentity[KeySize];

	std::vector<std::uint8_t> m_Pending;  // bytes recibidos que todavía no forman un saludo o un registro

	std::vector<std::uint8_t> m_Reply;

	TrustFn m_Trust;
};
