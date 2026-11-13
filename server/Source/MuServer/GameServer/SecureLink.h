#pragma once

// Canal cifrado con cada cliente (Fase 5, C4): une Common/SecureChannel con el
// SocketManager.  Reemplaza a EncryptData/DecryptData de HackCheck, el XOR de
// byte que se aplicaba a todo el tráfico.
//
// Al aceptar una conexión el server manda su saludo (ServerHello) en claro y
// espera el del cliente.  Recién cuando el canal queda listo manda el F1:00;
// antes de eso no sale ningún paquete del juego.
//
// Todas las llamadas se hacen con el m_critical del SocketManager tomado.

#include "SecureChannel.h"
#include "SocketManager.h"

class CSecureLink
{
public:

	CSecureLink();

	// En el accept: arma el canal del usuario y manda el ServerHello.
	void Begin(int index);

	// Al desconectar: borra las claves.
	void End(int index);

	// Bytes crudos del socket.  Descifra y le entrega el texto claro a
	// CSocketManager::DataRecv a través de `buffer`.  false = hay que desconectar.
	bool Receive(int index, const BYTE* raw, int size, IO_MAIN_BUFFER* buffer);

	// Cifra un paquete ya armado.  `out` necesita size + SecureChannel::Overhead.
	// Devuelve 0 si el canal todavía no terminó el saludo: el paquete no se manda.
	int Seal(int index, const BYTE* plain, int size, BYTE* out);

private:

	SecureChannel* Channel(int index);

	std::vector<SecureChannel> m_Channels;

	std::vector<BYTE> m_Plain;
};

extern CSecureLink gSecureLink;
