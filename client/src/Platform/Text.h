#pragma once
#include <cstring>
#include <cstdarg>
#include <cstdio>
#include <cstdint>
#include <algorithm>
#include <cerrno>
#include <utility>
#ifndef _WIN32
#include <strings.h>
#endif

namespace Platform {

// Adaptadores CRT acotados. Windows conserva la CRT nativa; POSIX mantiene
// NUL y retornos (-1 para formato truncado, ERANGE para copia sin espacio).
constexpr std::size_t TruncateText = static_cast<std::size_t>(-1);
inline int TextCopySecure(char* out, std::size_t capacity, const char* input, std::size_t count)
{
#ifdef _WIN32
    return strncpy_s(out, capacity, input, count);
#else
    if (!out || !capacity) return EINVAL;
    if (!input && count) { out[0] = 0; return EINVAL; }
    std::size_t length = 0;
    while (length < count && input && input[length]) ++length;
    if (length >= capacity) {
        if (count != TruncateText) { out[0] = 0; return ERANGE; }
        std::memcpy(out, input, capacity - 1); out[capacity - 1] = 0;
        return 80; // STRUNCATE de la CRT de Windows.
    }
    if (length) std::memcpy(out, input, length);
    out[length] = 0; return 0;
#endif
}
template<std::size_t N>
inline int TextCopySecure(char (&out)[N], const char* input, std::size_t count)
{ return TextCopySecure(out, N, input, count); }
inline int TextCopySecure(char* out, std::size_t capacity, const char* input)
{
#ifdef _WIN32
    return strcpy_s(out, capacity, input);
#else
    if (!input) { if (out && capacity) out[0] = 0; return EINVAL; }
    return TextCopySecure(out, capacity, input, std::strlen(input));
#endif
}
template<std::size_t N>
inline int TextCopySecure(char (&out)[N], const char* input)
{ return TextCopySecure(out, N, input); }
inline int TextConcatSecure(char* out, std::size_t capacity, const char* input)
{
#ifdef _WIN32
    return strcat_s(out, capacity, input);
#else
    if (!out || !capacity) return EINVAL;
    if (!input) { out[0] = 0; return EINVAL; }
    std::size_t used = 0;
    while (used < capacity && out[used]) ++used;
    if (used == capacity) { out[0] = 0; return EINVAL; }
    const int result = TextCopySecure(out + used, capacity - used, input);
    if (result) out[0] = 0;
    return result;
#endif
}
inline int TextVFormatTruncated(char* out, std::size_t capacity, std::size_t count,
                               const char* format, va_list args)
{
#ifdef _WIN32
    return _vsnprintf_s(out, capacity, count, format, args);
#else
    if (!out || !capacity || !format) { if (out && capacity) out[0] = 0; return -1; }
    const auto limit = count == TruncateText ? capacity : (std::min)(capacity, count + 1);
    const int written = std::vsnprintf(out, limit, format, args);
    if (written < 0) { out[0] = 0; return -1; }
    if (static_cast<std::size_t>(written) >= limit) {
        if (count != TruncateText && count >= capacity) out[0] = 0;
        return -1;
    }
    return written;
#endif
}
inline int TextFormatTruncated(char* out, std::size_t capacity, std::size_t count,
                              const char* format, ...)
{
    va_list args; va_start(args, format);
    const int result = TextVFormatTruncated(out, capacity, count, format, args);
    va_end(args); return result;
}
template<std::size_t N, class... Args>
inline int TextFormatTruncated(char (&out)[N], std::size_t count, const char* format, Args... args)
{ return TextFormatTruncated(out, N, count, format, args...); }
template<class... Args>
inline int TextFormatSecure(char* out, std::size_t capacity, const char* format, Args... args)
{
#ifdef _WIN32
    return sprintf_s(out, capacity, format, args...);
#else
    if (!out || !capacity || !format) { if (out && capacity) out[0] = 0; return -1; }
    const int result = std::snprintf(out, capacity, format, args...);
    if (result < 0 || static_cast<std::size_t>(result) >= capacity) {
        out[0] = 0; return -1;
    }
    return result;
#endif
}
template<std::size_t N, class... Args>
inline int TextFormatSecure(char (&out)[N], const char* format, Args... args)
{ return TextFormatSecure(out, N, format, args...); }

inline int TextCompareIgnoreCase(const char* left, const char* right)
{
#ifdef _WIN32
    return _stricmp(left, right);
#else
    return strcasecmp(left, right);
#endif
}
inline int TextCompareIgnoreCaseN(const char* left, const char* right, std::size_t count)
{
#ifdef _WIN32
    return _strnicmp(left, right, count);
#else
    return strncasecmp(left, right, count);
#endif
}

// Contrato lstrlenA: cero para puntero nulo; longitud de bytes sin el NUL.
// Los textos del cliente caben en int (buffers y longitudes de protocolo x86).
inline int TextLength(const char* text)
{
    return text ? static_cast<int>(std::strlen(text)) : 0;
}
// Capacidad incluye NUL. Como lstrcpynA, no rellena el resto del buffer.
// Origen/destino deben apuntar a memoria válida y no solaparse.
inline char* TextCopy(char* destination, const char* source, int capacity)
{
    if (!destination || !source) return nullptr;
    if (capacity <= 0) return destination;
    int count = 0;
    while (count < capacity - 1 && source[count]) {
        destination[count] = source[count];
        ++count;
    }
    destination[count] = '\0';
    return destination;
}
// Capacidad explícita: conserva el retorno destino para concatenaciones válidas.
// En desborde no escribe y devuelve nullptr; los callers actuales caben por diseño.
inline char* TextConcat(char* destination, std::size_t capacity, const char* source)
{
    if (!destination || !source) return nullptr;
    std::size_t used = 0;
    while (used < capacity && destination[used]) ++used;
    if (used == capacity) return nullptr;
    const std::size_t extra = std::strlen(source);
    if (extra >= capacity - used) return nullptr;
    std::memcpy(destination + used, source, extra + 1);
    return destination;
}
// Contrato wsprintfA del cliente: máximo 1024 bytes escritos más NUL.
// El destino debe tener espacio para el resultado (como en la API original).
// No admite floats: una conversión desconocida no consume argumentos.
inline int TextFormat(char* destination, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    int written = 0;
    auto emit = [&](char c) { if (written < 1024) destination[written++] = c; };
    while (*format && written < 1024) {
        if (*format != '%') { emit(*format++); continue; }
        ++format;
        bool left = false, zero = false, alternate = false;
        if (*format == '-') { left = true; ++format; }
        if (*format == '#') { alternate = true; ++format; }
        if (*format == '0') { zero = true; ++format; }
        int width = 0;
        while (*format >= '0' && *format <= '9') {
            width = (std::min)(1000000, width * 10 + (*format++ - '0'));
        }
        int precision = -1;
        if (*format == '.') {
            ++format; precision = 0;
            while (*format >= '0' && *format <= '9')
                precision = (std::min)(1000000, precision * 10 + (*format++ - '0'));
        }
        bool wideInteger = false;
        if (format[0] == 'I' && format[1] == '6' && format[2] == '4') {
            wideInteger = true; format += 3;
        } else if (*format == 'l' || *format == 'h') { ++format; }
        const char conversion = *format;
        if (!conversion) break;
        ++format;
        if (conversion == 's' || conversion == 'c') {
            char character[2] = {};
            const char* text;
            if (conversion == 'c') {
                character[0] = static_cast<char>(va_arg(args, int)); text = character;
            } else {
                text = va_arg(args, const char*);
                if (!text) text = "(null)";
            }
            int length = conversion == 'c' ? 1 : static_cast<int>(std::strlen(text));
            if (precision >= 0 && conversion == 's') length = (std::min)(length, precision);
            if (!left) for (int i = length; i < width && written < 1024; ++i) emit(' ');
            for (int i = 0; i < length && written < 1024; ++i) emit(text[i]);
            if (left) for (int i = length; i < width && written < 1024; ++i) emit(' ');
        } else if (conversion == 'd' || conversion == 'i' || conversion == 'u' ||
                   conversion == 'x' || conversion == 'X' || conversion == 'p') {
            char spec[48], value[2048];
            char* out = spec; *out++ = '%';
            if (left) *out++ = '-';
            if (alternate) *out++ = '#';
            if (zero || conversion == 'p') *out++ = '0';
            out += std::snprintf(out, 32, "%d", conversion == 'p' ? 8 : width);
            if (precision >= 0) out += std::snprintf(out, 20, ".%d", (std::max)(1, precision));
            *out++ = 'l'; *out++ = 'l';
            *out++ = conversion == 'p' ? 'X' : conversion; *out = 0;
            if (conversion == 'd' || conversion == 'i') {
                const auto number = wideInteger ? va_arg(args, long long) : va_arg(args, int);
                std::snprintf(value, sizeof(value), spec, number);
            } else {
                const auto number = wideInteger ? va_arg(args, unsigned long long) :
                    static_cast<unsigned long long>(va_arg(args, unsigned int));
                std::snprintf(value, sizeof(value), spec, number);
                // wsprintfA conserva el prefijo hexadecimal también para cero.
                if (alternate && number == 0 && (conversion == 'x' || conversion == 'X')) {
                    const int digits = (std::max)(1, precision);
                    const int total = (std::max)(width, digits + 2);
                    int index = 0;
                    auto append = [&](char c) { if (index < 2047) value[index++] = c; };
                    if (!left && !zero)
                        for (int i = digits + 2; i < total && index < 2047; ++i) append(' ');
                    append('0'); append(conversion);
                    const int zeros = zero && !left ? total - 2 : digits;
                    for (int i = 0; i < zeros && index < 2047; ++i) append('0');
                    if (left)
                        for (int i = digits + 2; i < total && index < 2047; ++i) append(' ');
                    value[index] = 0;
                }
            }
            for (const char* c = value; *c && written < 1024; ++c) emit(*c);
        } else {
            emit(conversion);
        }
    }
    va_end(args);
    destination[written] = 0;
    return written;
}
} // namespace Platform
