#pragma once

// Bytes aleatorios del sistema operativo, para claves (Fase 5, C3 y C4).
// rand() no sirve para esto: es predecible.

#include <cstddef>
#include <cstdint>
#include <cstdio>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <bcrypt.h>
#ifdef _MSC_VER
#pragma comment(lib, "bcrypt")
#endif
#endif

// false si el sistema no pudo dar los bytes: quien llama no debe seguir.
inline bool SecureRandomBytes(void* out, std::size_t size)
{
#ifdef _WIN32
	return BCryptGenRandom(nullptr, static_cast<PUCHAR>(out), static_cast<ULONG>(size), BCRYPT_USE_SYSTEM_PREFERRED_RNG) >= 0;
#else
	FILE* f = std::fopen("/dev/urandom", "rb");

	if (f == nullptr)
	{
		return false;
	}

	const std::size_t got = std::fread(out, 1, size, f);

	std::fclose(f);

	return got == size;
#endif
}
