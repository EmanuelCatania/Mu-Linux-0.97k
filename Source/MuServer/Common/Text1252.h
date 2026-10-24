#pragma once

#include <cstdint>
#include <string>

// Borde de texto con la base de datos (Fase 4, texto Unicode).
//
// La base y la conexión están en utf8mb4, pero el server todavía guarda y
// manda el texto en Windows-1252 (el protocolo con el cliente es 1252 hasta
// 0.97.40). El QueryManager convierte en el borde: lo que va a la base pasa a
// UTF-8 y lo que vuelve, a 1252. Cuando el server pase a UTF-8 estas
// funciones dejan de usarse.
namespace Text1252 {

inline std::uint32_t Decode(unsigned char byte)
{
	static const std::uint16_t high[32] = {
		0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
		0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008D, 0x017D, 0x008F,
		0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
		0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x009D, 0x017E, 0x0178
	};
	return (byte >= 0x80 && byte < 0xA0) ? high[byte - 0x80] : byte;
}

inline std::string ToUtf8(const char* text)
{
	std::string out;
	if (text == nullptr) return out;
	for (const unsigned char* p = (const unsigned char*)text; *p; ++p)
	{
		const std::uint32_t c = Decode(*p);
		if (c < 0x80)
		{
			out.push_back((char)c);
		}
		else if (c < 0x800)
		{
			out.push_back((char)(0xC0 | (c >> 6)));
			out.push_back((char)(0x80 | (c & 0x3F)));
		}
		else
		{
			out.push_back((char)(0xE0 | (c >> 12)));
			out.push_back((char)(0x80 | ((c >> 6) & 0x3F)));
			out.push_back((char)(0x80 | (c & 0x3F)));
		}
	}
	return out;
}

// Lo que no existe en 1252 sale como '?'; una secuencia inválida también.
inline std::string FromUtf8(const std::string& text)
{
	std::string out;
	const unsigned char* p = (const unsigned char*)text.data();
	const unsigned char* end = p + text.size();
	while (p < end)
	{
		std::uint32_t c = *p++;
		int rest = 0;
		if (c >= 0xF0 && c < 0xF8) { c &= 0x07; rest = 3; }
		else if (c >= 0xE0) { c &= 0x0F; rest = 2; }
		else if (c >= 0xC0) { c &= 0x1F; rest = 1; }
		else if (c >= 0x80) { out.push_back('?'); continue; }
		bool valid = true;
		for (; rest > 0; --rest)
		{
			if (p >= end || (*p & 0xC0) != 0x80) { valid = false; break; }
			c = (c << 6) | (*p++ & 0x3F);
		}
		if (!valid) { out.push_back('?'); continue; }
		if (c < 0x80 || (c >= 0xA0 && c <= 0xFF)) { out.push_back((char)c); continue; }
		char mapped = '?';
		for (int b = 0x80; b < 0xA0; ++b)
		{
			if (Decode((unsigned char)b) == c) { mapped = (char)b; break; }
		}
		out.push_back(mapped);
	}
	return out;
}

} // namespace Text1252
