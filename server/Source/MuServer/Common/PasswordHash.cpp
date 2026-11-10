#include "PasswordHash.h"

#include <cstdio>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#endif

extern "C"
{
#include "crypt_blowfish/crypt_blowfish.h"
}

namespace
{
	bool RandomBytes(unsigned char* out, int size)
	{
#ifdef _WIN32
		return BCryptGenRandom(nullptr, out, (ULONG)size, BCRYPT_USE_SYSTEM_PREFERRED_RNG) == 0;
#else
		FILE* file = fopen("/dev/urandom", "rb");

		if (file == nullptr)
		{
			return false;
		}

		const bool ok = fread(out, 1, (size_t)size, file) == (size_t)size;

		fclose(file);

		return ok;
#endif
	}
}

bool PasswordHash::IsHash(const unsigned char* stored, int size)
{
	if (stored == nullptr || size <= HashLength || stored[HashLength] != 0)
	{
		return false;
	}

	if (stored[0] != '$' || stored[1] != '2' || stored[3] != '$' || stored[6] != '$')
	{
		return false;
	}

	if (stored[2] != 'a' && stored[2] != 'b' && stored[2] != 'y')
	{
		return false;
	}

	for (int n = 0; n < HashLength; n++)
	{
		if (stored[n] < 0x21 || stored[n] > 0x7E)
		{
			return false;
		}
	}

	return true;
}

bool PasswordHash::Make(const char* password, int cost, char* out, int outSize)
{
	if (password == nullptr || out == nullptr || outSize <= HashLength)
	{
		return false;
	}

	if (cost < MinCost || cost > MaxCost)
	{
		cost = DefaultCost;
	}

	unsigned char random[16];

	if (RandomBytes(random, sizeof(random)) == false)
	{
		return false;
	}

	char setting[32];

	if (_crypt_gensalt_blowfish_rn("$2b$", (unsigned long)cost, (const char*)random, sizeof(random), setting, sizeof(setting)) == nullptr)
	{
		return false;
	}

	char hash[HashLength + 4];

	if (_crypt_blowfish_rn(password, setting, hash, sizeof(hash)) == nullptr || strlen(hash) != HashLength)
	{
		return false;
	}

	memcpy(out, hash, HashLength + 1);

	return true;
}

bool PasswordHash::Check(const char* password, const char* hash)
{
	if (password == nullptr || hash == nullptr || IsHash((const unsigned char*)hash, HashLength + 1) == false)
	{
		return false;
	}

	char computed[HashLength + 4];

	if (_crypt_blowfish_rn(password, hash, computed, sizeof(computed)) == nullptr || strlen(computed) != HashLength)
	{
		return false;
	}

	unsigned char diff = 0;

	for (int n = 0; n < HashLength; n++)
	{
		diff |= (unsigned char)(computed[n] ^ hash[n]);
	}

	return diff == 0;
}
