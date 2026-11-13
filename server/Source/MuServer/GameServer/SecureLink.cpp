#include "stdafx.h"
#include "SecureLink.h"
#include "Log.h"
#include "Protocol.h"
#include "SecureRandom.h"
#include "ServerIdentity.h"
#include "User.h"

CSecureLink gSecureLink;

CSecureLink::CSecureLink()
{
	this->m_Channels.resize(MAX_OBJECT_USER);
}

SecureChannel* CSecureLink::Channel(int index)
{
	if (OBJECT_USER_RANGE(index) == 0)
	{
		return nullptr;
	}

	return &this->m_Channels[index - OBJECT_START_USER];
}

void CSecureLink::Begin(int index)
{
	SecureChannel* channel = this->Channel(index);

	if (channel == nullptr)
	{
		return;
	}

	BYTE random[SecureChannel::KeySize];

	// Sin clave de identidad o sin bytes aleatorios no hay forma de armar un
	// canal seguro: la conexión no sigue.
	if (!gServerIdentity.IsLoaded() || !SecureRandomBytes(random, sizeof(random)))
	{
		gLog.Output(LOG_CONNECT, "[SecureLink][%d] No se pudo iniciar el canal (identidad cargada: %d)", index, gServerIdentity.IsLoaded() ? 1 : 0);

		gSocketManager.Disconnect(index);

		return;
	}

	BYTE hello[SecureChannel::ServerHelloSize];

	const bool rotation = gServerIdentity.HasRotation();

	channel->BeginServer(hello, random, gServerIdentity.SecretKey(), gServerIdentity.PublicKey(),
		rotation ? gServerIdentity.PreviousPublicKey() : nullptr,
		rotation ? gServerIdentity.RotationSignature() : nullptr);

	gSocketManager.DataSendRaw(index, hello, sizeof(hello));
}

void CSecureLink::End(int index)
{
	SecureChannel* channel = this->Channel(index);

	if (channel != nullptr)
	{
		channel->Reset();
	}
}

bool CSecureLink::Receive(int index, const BYTE* raw, int size, IO_MAIN_BUFFER* buffer)
{
	SecureChannel* channel = this->Channel(index);

	if (channel == nullptr)
	{
		return false;
	}

	const bool wasReady = channel->IsReady();

	this->m_Plain.clear();

	if (!channel->Receive(raw, (size_t)size, this->m_Plain))
	{
		gLog.Output(LOG_CONNECT, "[SecureLink][%d] Canal rechazado (motivo %d)", index, (int)channel->GetFailure());

		return false;
	}

	// El saludo terminó en esta lectura: recién ahora sale el primer paquete.
	if (!wasReady && channel->IsReady())
	{
		GCConnectClientSend(index, 1);
	}

	// El texto claro pasa a DataRecv por el buffer de recepción, de a lo que entre.
	size_t offset = 0;

	while (offset < this->m_Plain.size())
	{
		const int capacity = MAX_MAIN_PACKET_SIZE - buffer->size;

		if (capacity <= 0)
		{
			return false;
		}

		const size_t remaining = this->m_Plain.size() - offset;

		const int take = (remaining < (size_t)capacity) ? (int)remaining : capacity;

		memcpy(&buffer->buff[buffer->size], &this->m_Plain[offset], take);

		buffer->size += take;

		offset += take;

		if (gSocketManager.DataRecv(index, buffer) == 0)
		{
			return false;
		}

		// Un paquete pudo cerrar la conexión: lo que quede ya no es de nadie.
		if (gObj[index].Connected == OBJECT_OFFLINE)
		{
			break;
		}
	}

	return true;
}

int CSecureLink::Seal(int index, const BYTE* plain, int size, BYTE* out)
{
	SecureChannel* channel = this->Channel(index);

	if (channel == nullptr || size < 0)
	{
		return 0;
	}

	return (int)channel->Seal(plain, (size_t)size, out);
}
