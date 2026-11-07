#pragma once

#include <cstddef>
#include <cstdint>

// Nombre para mostrar de las entidades de CharactersClient (0.97.40).
//
// DESVIACION: CHARACTER::ID (+0x1C1) tiene 25 bytes hasta GuildMarkIndex
// (+0x1DA) y los nombres de personaje miden hasta 40 bytes de UTF-8. El nombre
// completo vive en una tabla aparte, una entrada por slot del pool; ID guarda
// una copia cortada en un borde de cluster para el código que todavía lo lee.
// Ahí entran también los nombres de las invocaciones ("dueño's monstruo") y
// los de los monstruos, que antes se cortaban a 25 bytes.
namespace EntityName {

constexpr std::size_t Capacity = 96;   // bytes con el NUL ("dueño's monstruo" entra entero)

// Nombre completo de la entidad. Si el puntero no es un slot del pool,
// devuelve CHARACTER::ID.
const char* Get(const void* character);
inline const char* Get(std::uintptr_t character) { return Get((const void*)character); }

// Guarda `length` bytes de `text` (corta en el primer NUL) como nombre de la
// entidad: completo en la tabla y cortado en CHARACTER::ID.
void Set(void* character, const char* text, std::size_t length);
void Set(void* character, const char* text);

// Borra el nombre (slot liberado o reusado).
void Clear(void* character);

// Personaje elegido en char-select. CharacterMachine guarda sólo 10 bytes de
// nombre (en +11 empieza la clase); el completo queda acá hasta que el F3/03
// crea al héroe.
void SetSelected(const char* name);
const char* Selected();

}
