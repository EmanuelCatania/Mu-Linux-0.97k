#pragma once

// Identidad del server (Fase 5, C3): un par de claves de firma (EdDSA de
// Monocypher) propio de cada instalación.  Con la clave privada se firma el
// manifiesto del catálogo; el cliente guarda la pública del primer contacto (o
// la trae compilada en ServerConfig.h) y desconfía si después cambia.
//
// La clave vive en Data/Keys/ServerIdentity.key y se genera sola en el primer
// arranque.  Data/ es un volumen, así que sobrevive a las actualizaciones de la
// imagen.
//
// Para regenerarla el admin crea el archivo vacío Data/Keys/ServerIdentity.regenerate
// y reinicia el GameServer: se genera una clave nueva, la anterior firma el
// reemplazo (rotación) y el archivo marcador se borra.  Los clientes que tenían
// la clave anterior aceptan la nueva sin intervención.  Borrar ServerIdentity.key
// también genera una nueva, pero sin rotación: esos clientes la rechazan hasta
// borrar su copia guardada.

#define SERVER_IDENTITY_KEY_SIZE 32
#define SERVER_IDENTITY_SIGNATURE_SIZE 64

class CServerIdentity
{
public:

	CServerIdentity();

	// Carga la clave de `folder` o la genera.  Sólo la primera llamada trabaja.
	bool Load(const char* folder);

	bool IsLoaded() const { return this->m_Loaded; }

	const BYTE* PublicKey() const { return this->m_Public; }

	void Sign(BYTE signature[SERVER_IDENTITY_SIGNATURE_SIZE], const BYTE* message, size_t size) const;

	// Rotación: clave anterior y su firma sobre la clave actual.
	bool HasRotation() const { return this->m_HasRotation; }

	const BYTE* PreviousPublicKey() const { return this->m_PreviousPublic; }

	const BYTE* RotationSignature() const { return this->m_RotationSignature; }

private:

	bool Read(const char* path);

	bool Write(const char* path) const;

	bool Generate();

	bool m_Loaded;

	bool m_HasRotation;

	BYTE m_Seed[SERVER_IDENTITY_KEY_SIZE];

	BYTE m_Secret[64];

	BYTE m_Public[SERVER_IDENTITY_KEY_SIZE];

	BYTE m_PreviousPublic[SERVER_IDENTITY_KEY_SIZE];

	BYTE m_RotationSignature[SERVER_IDENTITY_SIGNATURE_SIZE];
};

// Mensaje que firma la clave anterior al rotar: este prefijo + la clave nueva.
#define SERVER_IDENTITY_ROTATION_CONTEXT "MuIdentityRotation1"

extern CServerIdentity gServerIdentity;
