#pragma once

// Tabla de sonidos con id (Content/audio/sound/sounds.toml).
//
// Cada sonido tiene un slot fijo: el número que usan PlayBuffer y la tabla de
// sonidos de cada modelo. El archivo dice, para cada slot, qué .wav cargar,
// cuántos canales reservar y si es 3D.

// Carga la sección [sound]: los sonidos globales que se cargan al arrancar.
void SoundTable_LoadStartup();

// Carga la sección [monster.N] del monstruo de tipo N, si existe.
void SoundTable_LoadMonster(int type);
