#pragma once

#include <cstddef>
#include <string>

// Nombres de personaje y de guild en UTF-8 (0.97.40).
//
// Valida lo que manda el cliente y compara nombres igual que la base
// (utf8mb4_0900_as_ci): sin distinguir mayúsculas, distinguiendo acentos.
namespace TextName {

// Nombre de personaje o de guild recibido del cliente. Tiene que ser UTF-8
// válido y estar en NFC, tener entre 1 y maxClusters grapheme clusters y no
// llevar controles, separadores, comillas, barra invertida, símbolos ni
// emojis. Con allowSpecial en false sólo acepta letras, marcas y dígitos.
bool IsValid(const char* text, std::size_t size, int maxClusters, bool allowSpecial);

// Texto que sólo se usa como clave de consulta (borrar o elegir un
// personaje): UTF-8 válido, sin controles, separadores ni los caracteres
// que rompen el SQL armado con sprintf.
bool IsSafeKey(const char* text, std::size_t size);

// Clave de comparación: NFC con case fold. Vacía si el texto no es UTF-8.
std::string Key(const char* text, std::size_t size);

// Compara dos nombres terminados en NUL sin distinguir mayúsculas.
bool Equals(const char* a, const char* b);

// strstr sin distinguir mayúsculas, para la lista de nombres prohibidos.
bool Contains(const char* text, const char* fragment);

}
