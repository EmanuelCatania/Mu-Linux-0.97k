#pragma once
#include <cstring>

namespace Platform {
// Contrato lstrlenA: cero para puntero nulo; longitud de bytes sin el NUL.
// Los textos del cliente caben en int (buffers y longitudes de protocolo x86).
inline int TextLength(const char* text)
{
    return text ? static_cast<int>(std::strlen(text)) : 0;
}
} // namespace Platform
