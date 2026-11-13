// Ver SecureChannel.h.  ESTE ARCHIVO ES EL MISMO EN EL CLIENTE Y EN EL SERVER.
// Sin stdafx.h ni plataforma: se compila igual en las dos puntas y en las pruebas.

#include "SecureChannel.h"
#include <cstring>

// Monocypher está en net/crypto del cliente y en Common/monocypher del server.
extern "C" {
#if __has_include("net/crypto/monocypher.h")
#include "net/crypto/monocypher.h"
#else
#include "monocypher/monocypher.h"
#endif
}

namespace
{
	const char kServerMagic[4] = { 'M', 'U', 'S', '1' };
	const char kClientMagic[4] = { 'M', 'U', 'C', '1' };
	const char kHandshakeContext[] = "MuHandshake1";
	const char kChannelContext[] = "MuChannel1";
	const std::uint8_t kHasRotation = 0x01;
	const std::size_t kMacSize = 16;

	void MakeNonce(std::uint8_t nonce[24], std::uint64_t counter)
	{
		std::memset(nonce, 0, 24);

		for (int n = 0; n < 8; n++)
		{
			nonce[n] = (std::uint8_t)(counter >> (8 * n));
		}
	}

	bool IsZero(const std::uint8_t* data, std::size_t size)
	{
		std::uint8_t acc = 0;

		for (std::size_t n = 0; n < size; n++)
		{
			acc |= data[n];
		}

		return acc == 0;
	}
}

SecureChannel::SecureChannel()
{
	this->Reset();
}

SecureChannel::~SecureChannel()
{
	this->Reset();
}

bool SecureChannel::Fail(Failure why)
{
	this->m_State = State::Failed;

	this->m_Failure = why;

	return false;
}

void SecureChannel::Reset()
{
	this->m_State = State::Idle;

	this->m_Failure = Failure::None;

	this->m_IsServer = false;

	crypto_wipe(this->m_Secret, sizeof(this->m_Secret));

	crypto_wipe(this->m_SendKey, sizeof(this->m_SendKey));

	crypto_wipe(this->m_ReceiveKey, sizeof(this->m_ReceiveKey));

	std::memset(this->m_Public, 0, sizeof(this->m_Public));

	std::memset(this->m_PeerIdentity, 0, sizeof(this->m_PeerIdentity));

	this->m_SendCounter = 0;

	this->m_ReceiveCounter = 0;

	this->m_Pending.clear();

	this->m_Reply.clear();

	this->m_Trust = nullptr;
}

void SecureChannel::BeginServer(std::uint8_t* out, const std::uint8_t random[KeySize],
                                const std::uint8_t identitySecret[64], const std::uint8_t identityPublic[KeySize],
                                const std::uint8_t* previousKey, const std::uint8_t* rotationSignature)
{
	this->Reset();

	this->m_IsServer = true;

	std::memcpy(this->m_Secret, random, KeySize);

	crypto_x25519_public_key(this->m_Public, this->m_Secret);

	std::uint8_t message[sizeof(kHandshakeContext) - 1 + KeySize];

	std::memcpy(message, kHandshakeContext, sizeof(kHandshakeContext) - 1);

	std::memcpy(message + sizeof(kHandshakeContext) - 1, this->m_Public, KeySize);

	std::uint8_t* p = out;

	std::memcpy(p, kServerMagic, 4); p += 4;

	std::memcpy(p, this->m_Public, KeySize); p += KeySize;

	std::memcpy(p, identityPublic, KeySize); p += KeySize;

	crypto_eddsa_sign(p, identitySecret, message, sizeof(message)); p += SignatureSize;

	const bool rotation = previousKey != nullptr && rotationSignature != nullptr;

	*p++ = rotation ? kHasRotation : 0;

	if (rotation)
	{
		std::memcpy(p, previousKey, KeySize);

		std::memcpy(p + KeySize, rotationSignature, SignatureSize);
	}
	else
	{
		std::memset(p, 0, KeySize + SignatureSize);
	}

	this->m_State = State::AwaitClientHello;
}

void SecureChannel::BeginClient(const std::uint8_t random[KeySize], TrustFn trust)
{
	this->Reset();

	this->m_IsServer = false;

	std::memcpy(this->m_Secret, random, KeySize);

	crypto_x25519_public_key(this->m_Public, this->m_Secret);

	this->m_Trust = trust;

	this->m_State = State::AwaitServerHello;
}

std::size_t SecureChannel::TakeReply(std::uint8_t* out)
{
	const std::size_t size = this->m_Reply.size();

	if (size != 0)
	{
		std::memcpy(out, this->m_Reply.data(), size);

		this->m_Reply.clear();
	}

	return size;
}

void SecureChannel::DeriveKeys(const std::uint8_t serverEphemeral[KeySize], const std::uint8_t clientEphemeral[KeySize], const std::uint8_t peerEphemeral[KeySize])
{
	std::uint8_t shared[KeySize];

	crypto_x25519(shared, this->m_Secret, peerEphemeral);

	// Una clave pública de orden bajo da un secreto todo en cero: se rechaza.
	if (IsZero(shared, sizeof(shared)))
	{
		this->Fail(Failure::BadKey);

		return;
	}

	std::uint8_t message[sizeof(kChannelContext) - 1 + KeySize * 2];

	std::memcpy(message, kChannelContext, sizeof(kChannelContext) - 1);

	std::memcpy(message + sizeof(kChannelContext) - 1, serverEphemeral, KeySize);

	std::memcpy(message + sizeof(kChannelContext) - 1 + KeySize, clientEphemeral, KeySize);

	std::uint8_t keys[KeySize * 2];

	crypto_blake2b_keyed(keys, sizeof(keys), shared, sizeof(shared), message, sizeof(message));

	// Primera mitad: server -> cliente.  Segunda: cliente -> server.
	std::memcpy(this->m_IsServer ? this->m_SendKey : this->m_ReceiveKey, keys, KeySize);

	std::memcpy(this->m_IsServer ? this->m_ReceiveKey : this->m_SendKey, keys + KeySize, KeySize);

	crypto_wipe(shared, sizeof(shared));

	crypto_wipe(keys, sizeof(keys));

	crypto_wipe(this->m_Secret, sizeof(this->m_Secret));

	this->m_SendCounter = 0;

	this->m_ReceiveCounter = 0;

	this->m_State = State::Ready;
}

// Consume el saludo de m_Pending si ya llegó entero.  false = error fatal.
bool SecureChannel::HandleHello()
{
	if (this->m_State == State::AwaitClientHello)
	{
		// La marca se mira apenas llegan sus 4 bytes: una punta que no habla
		// este protocolo no manda nunca el saludo entero.
		if (this->m_Pending.size() >= 4 && std::memcmp(this->m_Pending.data(), kClientMagic, 4) != 0)
		{
			return this->Fail(Failure::NotThisProtocol);
		}

		if (this->m_Pending.size() < ClientHelloSize)
		{
			return true;
		}

		const std::uint8_t* p = this->m_Pending.data();

		std::uint8_t clientEphemeral[KeySize];

		std::memcpy(clientEphemeral, p + 4, KeySize);

		this->m_Pending.erase(this->m_Pending.begin(), this->m_Pending.begin() + ClientHelloSize);

		this->DeriveKeys(this->m_Public, clientEphemeral, clientEphemeral);

		return this->m_State == State::Ready;
	}

	if (this->m_State == State::AwaitServerHello)
	{
		if (this->m_Pending.size() >= 4 && std::memcmp(this->m_Pending.data(), kServerMagic, 4) != 0)
		{
			return this->Fail(Failure::NotThisProtocol);
		}

		if (this->m_Pending.size() < ServerHelloSize)
		{
			return true;
		}

		const std::uint8_t* p = this->m_Pending.data();

		std::uint8_t serverEphemeral[KeySize], identity[KeySize], signature[SignatureSize], previous[KeySize], rotation[SignatureSize];

		std::memcpy(serverEphemeral, p + 4, KeySize);

		std::memcpy(identity, p + 4 + KeySize, KeySize);

		std::memcpy(signature, p + 4 + KeySize * 2, SignatureSize);

		const bool hasRotation = (p[4 + KeySize * 2 + SignatureSize] & kHasRotation) != 0;

		std::memcpy(previous, p + 4 + KeySize * 2 + SignatureSize + 1, KeySize);

		std::memcpy(rotation, p + 4 + KeySize * 3 + SignatureSize + 1, SignatureSize);

		this->m_Pending.erase(this->m_Pending.begin(), this->m_Pending.begin() + ServerHelloSize);

		// La identidad firma la clave efímera: quien no tenga la clave privada
		// del server no puede armar este saludo.
		std::uint8_t message[sizeof(kHandshakeContext) - 1 + KeySize];

		std::memcpy(message, kHandshakeContext, sizeof(kHandshakeContext) - 1);

		std::memcpy(message + sizeof(kHandshakeContext) - 1, serverEphemeral, KeySize);

		if (crypto_eddsa_check(signature, identity, message, sizeof(message)) != 0)
		{
			return this->Fail(Failure::BadSignature);
		}

		if (!this->m_Trust || !this->m_Trust(identity, hasRotation ? previous : nullptr, hasRotation ? rotation : nullptr))
		{
			return this->Fail(Failure::Untrusted);
		}

		std::memcpy(this->m_PeerIdentity, identity, KeySize);

		this->m_Reply.assign(kClientMagic, kClientMagic + 4);

		this->m_Reply.insert(this->m_Reply.end(), this->m_Public, this->m_Public + KeySize);

		this->DeriveKeys(serverEphemeral, this->m_Public, serverEphemeral);

		return this->m_State == State::Ready;
	}

	return this->m_State == State::Ready;
}

bool SecureChannel::Receive(const std::uint8_t* data, std::size_t size, std::vector<std::uint8_t>& plain)
{
	if (this->m_State == State::Idle || this->m_State == State::Failed)
	{
		return false;
	}

	this->m_Pending.insert(this->m_Pending.end(), data, data + size);

	if (this->m_State != State::Ready)
	{
		if (!this->HandleHello())
		{
			return false;
		}

		if (this->m_State != State::Ready)
		{
			return true;   // el saludo todavía no llegó entero
		}
	}

	std::size_t offset = 0;

	while (this->m_Pending.size() - offset >= 2)
	{
		const std::uint8_t* record = this->m_Pending.data() + offset;

		const std::size_t length = record[0] | ((std::size_t)record[1] << 8);

		if (length > MaxPlain)
		{
			return this->Fail(Failure::BadRecord);
		}

		if (this->m_Pending.size() - offset < length + Overhead)
		{
			break;
		}

		std::uint8_t nonce[24];

		MakeNonce(nonce, this->m_ReceiveCounter);

		const std::size_t at = plain.size();

		plain.resize(at + length);

		if (crypto_aead_unlock(length != 0 ? plain.data() + at : nullptr, record + 2 + length, this->m_ReceiveKey, nonce, record, 2, record + 2, length) != 0)
		{
			plain.resize(at);

			return this->Fail(Failure::BadRecord);
		}

		this->m_ReceiveCounter++;

		offset += length + Overhead;
	}

	if (offset != 0)
	{
		this->m_Pending.erase(this->m_Pending.begin(), this->m_Pending.begin() + offset);
	}

	return true;
}

std::size_t SecureChannel::Seal(const std::uint8_t* plain, std::size_t size, std::uint8_t* out)
{
	if (this->m_State != State::Ready || size > MaxPlain)
	{
		return 0;
	}

	out[0] = (std::uint8_t)(size & 0xFF);

	out[1] = (std::uint8_t)(size >> 8);

	std::uint8_t nonce[24];

	MakeNonce(nonce, this->m_SendCounter);

	crypto_aead_lock(out + 2, out + 2 + size, this->m_SendKey, nonce, out, 2, plain, size);

	this->m_SendCounter++;

	return size + Overhead;
}
