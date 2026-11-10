#pragma once
// Sonidos de los emotes propios (Data/Sound/Dances/<nombre>.mp3). Los archivos no
// se distribuyen (copyright): si falta el archivo, el emote se ve y no suena.
// Ver Data/Sound/Dances/README.md.

// Arranca el sonido del emote `emote` para la entidad `character` (puntero al
// slot del personaje). Reemplaza el que esa entidad tuviera.
void EmoteSound_Start(unsigned char* character, int emote);
// Una vez por tick: corta los sonidos cuyo emote ya termino o se cancelo y ajusta
// el volumen segun la distancia al heroe.
void EmoteSound_Update();
