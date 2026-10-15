#pragma once
#include <cstring>
#include <cstdarg>
#include <cstdio>
#include <cstdint>
#include <algorithm>

namespace Platform {
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
