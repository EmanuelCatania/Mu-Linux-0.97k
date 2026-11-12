#pragma once

#include <cstdint>

// Velocidad de ataque de los demás jugadores (Fase 5, A5).
//
// El 0.97k tiene una sola tabla de velocidades para todos los jugadores (las
// acciones de Models[390]), que SetAttackSpeed (0x00443E70) llena con la
// velocidad del héroe: los demás se animan a la velocidad del que mira.  El
// server manda la de cada jugador en el 0x12 y en el F3:FA cuando cambia; al
// animar a ese jugador, CharacterAnimation usa la suya.

// Guarda la velocidad del jugador del slot `slot` (0..399) con esa Key.
void PlayerAttackSpeed_Set(int slot, std::uint16_t key, int physiSpeed, int magicSpeed);

// Velocidad de la acción `action` para el personaje `c` si el server mandó la
// suya y la acción depende de la velocidad de ataque; si no, devuelve false.
bool PlayerAttackSpeed_ForAction(const void* c, int action, float* speed);

// F3:FA.
void PlayerAttackSpeed_Receive(const std::uint8_t* msg, int size);
