#pragma once

// Largos de los textos que comparten el server y el protocolo, en bytes de
// UTF-8 (0.97.40).
//
// *_SIZE es el campo del paquete, que puede ir lleno sin terminador.
// *_BUFFER es el buffer interno: el mismo largo más el NUL.
//
// El nombre de personaje admite 10 grapheme clusters y el de guild 8; los
// bytes alcanzan para 4 por cluster. Las cuentas y contraseñas quedan fuera:
// siguen siendo ASCII de 10 bytes.
#define CHARACTER_NAME_CLUSTERS 10
#define CHARACTER_NAME_SIZE     40
#define CHARACTER_NAME_BUFFER   (CHARACTER_NAME_SIZE + 1)

#define GUILD_NAME_CLUSTERS 8
#define GUILD_NAME_SIZE     32
#define GUILD_NAME_BUFFER   (GUILD_NAME_SIZE + 1)

// Mensaje de chat, susurro, aviso de guild y /post.
#define CHAT_MESSAGE_SIZE 200
