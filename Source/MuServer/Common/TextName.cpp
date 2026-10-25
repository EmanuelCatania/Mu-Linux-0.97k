#include "TextName.h"

#include <cstdlib>
#include <cstring>

#ifndef UTF8PROC_STATIC
#define UTF8PROC_STATIC
#endif
#include "utf8proc/utf8proc.h"

namespace TextName {

namespace {

std::size_t BoundedLength(const char* text, std::size_t size)
{
	std::size_t n = 0;
	while (n < size && text[n] != 0) ++n;
	return n;
}

// Recorre el texto como codepoints; false si no es UTF-8 válido.
template <typename Fn>
bool ForEachCodepoint(const char* text, std::size_t length, Fn fn)
{
	const utf8proc_uint8_t* p = (const utf8proc_uint8_t*)text;
	utf8proc_ssize_t left = (utf8proc_ssize_t)length;
	while (left > 0)
	{
		utf8proc_int32_t c = 0;
		const utf8proc_ssize_t n = utf8proc_iterate(p, left, &c);
		if (n <= 0 || c < 0) return false;
		if (!fn(c)) return false;
		p += n;
		left -= n;
	}
	return true;
}

bool IsSeparatorOrControl(utf8proc_int32_t c)
{
	switch (utf8proc_category(c))
	{
	case UTF8PROC_CATEGORY_CC:
	case UTF8PROC_CATEGORY_CF:
	case UTF8PROC_CATEGORY_CS:
	case UTF8PROC_CATEGORY_CO:
	case UTF8PROC_CATEGORY_CN:
	case UTF8PROC_CATEGORY_ZS:
	case UTF8PROC_CATEGORY_ZL:
	case UTF8PROC_CATEGORY_ZP:
		return true;
	default:
		return false;
	}
}

// Comillas, barra invertida y % rompen el SQL que se arma con sprintf.
bool IsSqlBreaker(utf8proc_int32_t c)
{
	return c == '"' || c == '\'' || c == '\\' || c == '%' || c == '`';
}

std::string Map(const char* text, std::size_t length, utf8proc_option_t options)
{
	utf8proc_uint8_t* out = nullptr;
	const utf8proc_ssize_t n = utf8proc_map((const utf8proc_uint8_t*)text, (utf8proc_ssize_t)length, &out, options);
	std::string result;
	if (n >= 0 && out != nullptr) result.assign((const char*)out, (std::size_t)n);
	std::free(out);
	return result;
}

}

bool IsValid(const char* text, std::size_t size, int maxClusters, bool allowSpecial)
{
	const std::size_t length = BoundedLength(text, size);
	if (length == 0) return false;

	int clusters = 0;
	utf8proc_int32_t previous = -1;
	utf8proc_int32_t state = 0;
	const bool ok = ForEachCodepoint(text, length, [&](utf8proc_int32_t c) {
		if (IsSeparatorOrControl(c) || IsSqlBreaker(c)) return false;
		const utf8proc_category_t category = utf8proc_category(c);
		if (category == UTF8PROC_CATEGORY_SO || category == UTF8PROC_CATEGORY_SK) return false;
		if (!allowSpecial)
		{
			const bool letter = category >= UTF8PROC_CATEGORY_LU && category <= UTF8PROC_CATEGORY_LO;
			const bool mark = category >= UTF8PROC_CATEGORY_MN && category <= UTF8PROC_CATEGORY_ME;
			const bool digit = category >= UTF8PROC_CATEGORY_ND && category <= UTF8PROC_CATEGORY_NO;
			if (!letter && !mark && !digit) return false;
		}
		if (previous < 0 || utf8proc_grapheme_break_stateful(previous, c, &state)) ++clusters;
		previous = c;
		return true;
	});
	if (!ok || clusters < 1 || clusters > maxClusters) return false;

	// Una marca combinante no puede abrir el nombre.
	utf8proc_int32_t first = 0;
	utf8proc_iterate((const utf8proc_uint8_t*)text, (utf8proc_ssize_t)length, &first);
	const utf8proc_category_t firstCategory = utf8proc_category(first);
	if (firstCategory >= UTF8PROC_CATEGORY_MN && firstCategory <= UTF8PROC_CATEGORY_ME) return false;

	// El cliente manda NFC; otra forma sería otro nombre con el mismo aspecto.
	return Map(text, length, (utf8proc_option_t)(UTF8PROC_STABLE | UTF8PROC_COMPOSE)) == std::string(text, length);
}

bool IsSafeKey(const char* text, std::size_t size)
{
	const std::size_t length = BoundedLength(text, size);
	return ForEachCodepoint(text, length, [](utf8proc_int32_t c) {
		return !IsSeparatorOrControl(c) && !IsSqlBreaker(c);
	});
}

std::string ToDisplay(const char* text, std::size_t size)
{
	const std::size_t length = BoundedLength(text, size);
	const bool ok = length > 0 && ForEachCodepoint(text, length, [](utf8proc_int32_t c) {
		switch (utf8proc_category(c))
		{
		case UTF8PROC_CATEGORY_CC:
		case UTF8PROC_CATEGORY_CS:
		case UTF8PROC_CATEGORY_CO:
		case UTF8PROC_CATEGORY_CN:
		case UTF8PROC_CATEGORY_ZL:
		case UTF8PROC_CATEGORY_ZP:
			return false;
		default:
			return true;
		}
	});
	return ok ? Map(text, length, (utf8proc_option_t)(UTF8PROC_STABLE | UTF8PROC_COMPOSE)) : std::string();
}

std::string Key(const char* text, std::size_t size)
{
	return Map(text, BoundedLength(text, size), (utf8proc_option_t)(UTF8PROC_STABLE | UTF8PROC_COMPOSE | UTF8PROC_CASEFOLD));
}

bool Equals(const char* a, const char* b)
{
	if (std::strcmp(a, b) == 0) return true;
	const std::string ka = Key(a, std::strlen(a));
	return !ka.empty() && ka == Key(b, std::strlen(b));
}

bool Contains(const char* text, const char* fragment)
{
	const std::string key = Key(fragment, std::strlen(fragment));
	return !key.empty() && Key(text, std::strlen(text)).find(key) != std::string::npos;
}

}
