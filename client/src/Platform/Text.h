#pragma once
#include <cstring>

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
} // namespace Platform
