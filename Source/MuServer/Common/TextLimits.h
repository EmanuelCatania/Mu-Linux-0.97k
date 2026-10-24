#pragma once

// Largos de los nombres que comparten el server y el protocolo.
//
// *_SIZE es el campo del paquete, que puede ir lleno sin terminador.
// *_BUFFER es el buffer interno: el mismo largo más el NUL.
//
// Las cuentas y contraseñas quedan fuera: siguen siendo ASCII de 10 bytes.
// Máximo de grapheme clusters que valida TextName.
#define CHARACTER_NAME_CLUSTERS 10
#define CHARACTER_NAME_SIZE   10
#define CHARACTER_NAME_BUFFER (CHARACTER_NAME_SIZE + 1)

#define GUILD_NAME_CLUSTERS 8
#define GUILD_NAME_SIZE   8
#define GUILD_NAME_BUFFER (GUILD_NAME_SIZE + 1)
