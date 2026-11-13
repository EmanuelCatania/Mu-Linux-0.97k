// MuEmu.h
// Enganche global de send() (Fase 5, C4).
//
// Varios sitios del cliente arman su paquete y llaman a send() directo sobre el
// socket del juego.  stdafx.h redefine `send` para que todos pasen por acá, y de
// acá al canal cifrado (net/NetChannel.h).  Así el envío tiene un solo camino,
// que es lo que necesitan los registros del canal: salir en orden y enteros.
//
// Antes este archivo tenía el XOR de byte de MuEmu (EncryptSend / DecryptRecv,
// con la clave derivada de CustomerName y ServerSerial).  El canal lo reemplaza.

#pragma once
#include "platform/PlatformHeaders.h"

// Devuelve len (el canal acepta siempre el paquete entero y encola lo que el
// socket no toma) o -1 si hubo que cerrar la conexión.
extern "C" int __stdcall MuEmu_send_hook(SOCKET s, const char* buf, int len, int flags);
