#pragma once

// Enlace del socket del juego con el canal cifrado (Fase 5, C4).
//
// Todo lo que el cliente manda y recibe por el socket pasa por acá:
//   - Con el GameServer va por un SecureChannel: saludo al conectar y después
//     registros cifrados y autenticados (ver net/SecureChannel.h).
//   - Con el ConnectServer va en claro: sólo lleva la lista pública de servers.
//
// Reemplaza al XOR de byte de MuEmu (MuEmu::EncryptSend / DecryptRecv).  Ese XOR
// no tenía estado, así que cualquier sitio podía llamar a send() por su cuenta.
// Los registros llevan un contador: tienen que salir en orden y enteros.  Por eso
// el envío tiene un solo camino (Send) y una sola cola (la de SocketClient).

#include <cstddef>
#include <cstdint>

namespace NetChannel {

// Al crear el socket: se olvida el canal de la conexión anterior.
// secure = true para el GameServer, false para el ConnectServer.
void OnNewConnection(bool secure);

// Manda `len` bytes (uno o más paquetes ya armados).  Siempre los acepta
// enteros: lo que el socket no toma ahora queda en la cola de SocketClient, que
// vacía CWsctlc_FDWriteSend en el próximo FD_WRITE.
// Devuelve len, o -1 si hubo que cerrar la conexión.
int Send(const char* data, int len);

// Reemplaza al recv() de CWsctlc_nRecv: lee del socket, descifra y deja en `out`
// hasta `capacity` bytes de texto claro.  Devuelve lo mismo que recv: cuántos
// bytes dejó, 0 si el server cerró, -1 si no hay nada para el parser todavía
// (sin datos, sólo el saludo, o un registro incompleto) o si hubo un error.
// Si quedó texto claro sin entregar, la próxima llamada lo entrega sin leer del
// socket.
int Read(std::uintptr_t socket, char* out, int capacity, int flags);

// Texto claro que no entró en la última lectura.  Mientras sea mayor que cero
// hay que volver a llamar a CWsctlc_nRecv.
int PendingPlain();

// true si `data` apunta dentro de la cola de envío del socket: esos bytes ya
// pasaron por Send (están cifrados) y van al socket tal cual.
bool IsQueued(const char* data);

} // namespace NetChannel
