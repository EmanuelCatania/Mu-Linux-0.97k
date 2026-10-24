#pragma once

#include <cstddef>
#include <cstring>

#include "TextLimits.h"

// Copia de campos de texto de largo fijo (nombres, cuentas, guilds).
//
// Los structs del server y del protocolo guardan el texto en arrays de largo
// fijo que no siempre terminan en NUL: un nombre de 10 bytes en el paquete
// ocupa el campo entero. El código copiaba con memcpy(dst, src, sizeof(dst)),
// que sólo funciona mientras los dos arrays midan lo mismo. Copy toma el
// tamaño de los dos lados del tipo, así que agrandar un buffer no hace leer
// fuera del otro:
//
//   - copia hasta el primer NUL del origen, sin pasar el tamaño de ninguno;
//   - rellena con ceros el resto del destino.
//
// Si el destino es más largo que el texto, queda terminado en NUL. Si es igual
// de largo (el campo de 10 bytes del paquete), queda lleno sin terminador,
// igual que antes.
namespace TextField {

inline std::size_t Length(const char* text, std::size_t max)
{
	std::size_t n = 0;
	while (n < max && text[n] != 0) ++n;
	return n;
}

inline void CopyBounded(char* dst, std::size_t dstSize, const char* src, std::size_t srcSize)
{
	const std::size_t n = Length(src, (srcSize < dstSize) ? srcSize : dstSize);
	std::memcpy(dst, src, n);
	std::memset(dst + n, 0, dstSize - n);
}

template <std::size_t D, std::size_t S>
inline void Copy(char (&dst)[D], const char (&src)[S])
{
	CopyBounded(dst, D, src, S);
}

// Origen sin tamaño conocido (parámetro char*): tiene que terminar en NUL o
// medir al menos lo que el destino.
template <std::size_t D>
inline void CopyString(char (&dst)[D], const char* src)
{
	CopyBounded(dst, D, src, D);
}

}
