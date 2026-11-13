#include "stdafx.h"
#include "ServerIdentity.h"
#include "SecureRandom.h"
#include "Util.h"
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <string>

// fopen: mismo criterio que el resto del server.
#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif

extern "C"
{
#include "monocypher/monocypher.h"
}

CServerIdentity gServerIdentity;

namespace
{
	std::string ToHex(const BYTE* data, size_t size)
	{
		static const char digits[] = "0123456789abcdef";

		std::string out;

		for (size_t n = 0; n < size; n++)
		{
			out += digits[data[n] >> 4];

			out += digits[data[n] & 15];
		}

		return out;
	}

	bool FromHex(const char* text, BYTE* out, size_t size)
	{
		for (size_t n = 0; n < size; n++)
		{
			int value = 0;

			for (int d = 0; d < 2; d++)
			{
				const char c = text[n * 2 + d];

				int digit;

				if (c >= '0' && c <= '9') digit = c - '0';
				else if (c >= 'a' && c <= 'f') digit = c - 'a' + 10;
				else if (c >= 'A' && c <= 'F') digit = c - 'A' + 10;
				else return false;

				value = value * 16 + digit;
			}

			out[n] = (BYTE)value;
		}

		return true;
	}

	// "clave=valor" de una línea; false si la línea es de otra clave.
	bool ReadField(const char* line, const char* key, BYTE* out, size_t size)
	{
		const size_t length = strlen(key);

		if (strncmp(line, key, length) != 0 || line[length] != '=')
		{
			return false;
		}

		return strlen(line + length + 1) >= size * 2 && FromHex(line + length + 1, out, size);
	}
}

CServerIdentity::CServerIdentity()
{
	this->m_Loaded = false;

	this->m_HasRotation = false;

	memset(this->m_Seed, 0, sizeof(this->m_Seed));

	memset(this->m_Secret, 0, sizeof(this->m_Secret));

	memset(this->m_Public, 0, sizeof(this->m_Public));

	memset(this->m_PreviousPublic, 0, sizeof(this->m_PreviousPublic));

	memset(this->m_RotationSignature, 0, sizeof(this->m_RotationSignature));
}

bool CServerIdentity::Generate()
{
	if (!SecureRandomBytes(this->m_Seed, sizeof(this->m_Seed)))
	{
		return false;
	}

	// crypto_eddsa_key_pair borra la semilla que recibe: se le pasa una copia.
	BYTE seed[SERVER_IDENTITY_KEY_SIZE];

	memcpy(seed, this->m_Seed, sizeof(seed));

	crypto_eddsa_key_pair(this->m_Secret, this->m_Public, seed);

	return true;
}

bool CServerIdentity::Read(const char* path)
{
	FILE* file = fopen(path, "r");

	if (file == nullptr)
	{
		return false;
	}

	bool hasSeed = false, hasPrevious = false, hasRotation = false;

	char line[256];

	while (fgets(line, sizeof(line), file) != nullptr)
	{
		line[strcspn(line, "\r\n")] = 0;

		if (ReadField(line, "seed", this->m_Seed, sizeof(this->m_Seed))) hasSeed = true;
		else if (ReadField(line, "previous", this->m_PreviousPublic, sizeof(this->m_PreviousPublic))) hasPrevious = true;
		else if (ReadField(line, "rotation", this->m_RotationSignature, sizeof(this->m_RotationSignature))) hasRotation = true;
	}

	fclose(file);

	if (!hasSeed)
	{
		return false;
	}

	BYTE seed[SERVER_IDENTITY_KEY_SIZE];

	memcpy(seed, this->m_Seed, sizeof(seed));

	crypto_eddsa_key_pair(this->m_Secret, this->m_Public, seed);

	this->m_HasRotation = hasPrevious && hasRotation;

	return true;
}

bool CServerIdentity::Write(const char* path) const
{
	FILE* file = fopen(path, "w");

	if (file == nullptr)
	{
		return false;
	}

	fprintf(file, "# Clave de identidad de este server (Mu-Linux). No la compartas: quien la\n");
	fprintf(file, "# tenga puede hacerse pasar por el server.  La publica, que si se comparte, es:\n");
	fprintf(file, "# %s\n", ToHex(this->m_Public, sizeof(this->m_Public)).c_str());
	fprintf(file, "seed=%s\n", ToHex(this->m_Seed, sizeof(this->m_Seed)).c_str());

	if (this->m_HasRotation)
	{
		fprintf(file, "previous=%s\n", ToHex(this->m_PreviousPublic, sizeof(this->m_PreviousPublic)).c_str());
		fprintf(file, "rotation=%s\n", ToHex(this->m_RotationSignature, sizeof(this->m_RotationSignature)).c_str());
	}

	const bool ok = ferror(file) == 0;

	return fclose(file) == 0 && ok;
}

bool CServerIdentity::Load(const char* folder)
{
	if (this->m_Loaded)
	{
		return true;
	}

	std::string base(folder);

	for (char& c : base)
	{
		if (c == '\\') c = '/';
	}

	while (!base.empty() && base.back() == '/')
	{
		base.pop_back();
	}

	std::error_code ec;

	std::filesystem::create_directories(base, ec);

	const std::string keyPath = base + "/ServerIdentity.key";

	const std::string markerPath = base + "/ServerIdentity.regenerate";

	const bool exists = this->Read(keyPath.c_str());

	const bool regenerate = std::filesystem::exists(markerPath, ec);

	if (exists && !regenerate)
	{
		this->m_Loaded = true;

		LogAdd(LOG_BLUE, "[ServerIdentity] Clave publica: %s", ToHex(this->m_Public, sizeof(this->m_Public)).c_str());

		return true;
	}

	// La clave que se reemplaza firma a la nueva, para que los clientes que la
	// conocen acepten el cambio.
	BYTE oldSecret[64], oldPublic[SERVER_IDENTITY_KEY_SIZE];

	memcpy(oldSecret, this->m_Secret, sizeof(oldSecret));

	memcpy(oldPublic, this->m_Public, sizeof(oldPublic));

	if (!this->Generate())
	{
		LogAdd(LOG_RED, "[ServerIdentity] El sistema no dio bytes aleatorios: no se pudo generar la clave");

		return false;
	}

	this->m_HasRotation = false;

	if (exists)
	{
		BYTE message[sizeof(SERVER_IDENTITY_ROTATION_CONTEXT) - 1 + SERVER_IDENTITY_KEY_SIZE];

		memcpy(message, SERVER_IDENTITY_ROTATION_CONTEXT, sizeof(SERVER_IDENTITY_ROTATION_CONTEXT) - 1);

		memcpy(message + sizeof(SERVER_IDENTITY_ROTATION_CONTEXT) - 1, this->m_Public, SERVER_IDENTITY_KEY_SIZE);

		crypto_eddsa_sign(this->m_RotationSignature, oldSecret, message, sizeof(message));

		memcpy(this->m_PreviousPublic, oldPublic, sizeof(this->m_PreviousPublic));

		this->m_HasRotation = true;
	}

	crypto_wipe(oldSecret, sizeof(oldSecret));

	if (!this->Write(keyPath.c_str()))
	{
		LogAdd(LOG_RED, "[ServerIdentity] No se pudo escribir %s", keyPath.c_str());

		return false;
	}

	// Sólo el usuario del server la lee (en Windows no cambia nada).
	std::filesystem::permissions(keyPath, std::filesystem::perms::owner_read | std::filesystem::perms::owner_write, ec);

	if (regenerate)
	{
		std::filesystem::remove(markerPath, ec);
	}

	this->m_Loaded = true;

	LogAdd(LOG_BLUE, "[ServerIdentity] Clave %s. Publica: %s", exists ? "regenerada (con rotacion)" : "generada", ToHex(this->m_Public, sizeof(this->m_Public)).c_str());

	return true;
}

void CServerIdentity::Sign(BYTE signature[SERVER_IDENTITY_SIGNATURE_SIZE], const BYTE* message, size_t size) const
{
	crypto_eddsa_sign(signature, this->m_Secret, message, size);
}
