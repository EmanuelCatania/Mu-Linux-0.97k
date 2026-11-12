#pragma once

// Captura de la traza de paquetes (lado GameServer). El formato del anillo y la
// forma de leerlo están en Common/TraceRing.h; el lector es mu-trace.
//
// Sólo existe en Linux. En Windows todas las funciones son no-ops y
// PacketTraceActive() devuelve siempre false.

#include "TraceRing.h"   // Common/TraceRing.h

namespace MuTrace
{
	extern RingHeader* gRing;
}

// Crea el segmento si MU_TRACE_ENABLED=1. Llamar una vez, con el puerto ya abierto.
void PacketTraceInit(int port);
void PacketTraceShutdown();

// true si hay un lector pidiendo captura. Es lo único que se evalúa cuando la
// traza está apagada.
inline bool PacketTraceActive()
{
	MuTrace::RingHeader* ring = MuTrace::gRing;
	return ring != nullptr && ring->capture.load(std::memory_order_relaxed) != 0;
}

// Paquete recibido, ya descifrado (C3/C4 llegan como C1/C2). `encrypt` indica si
// viajó cifrado, para reconstruir el frame original.
void PacketTraceRecvImpl(int aIndex, const BYTE* lpMsg, int size, int encrypt);
// Paquete enviado al cliente, antes de cifrar.
void PacketTraceSendImpl(int aIndex, const BYTE* lpMsg, int size);
// Decisión del server: rechazo, descarte o corte, con el motivo.
void PacketTraceDecisionImpl(int aIndex, int head, int sub, const char* fmt, ...);

inline void PacketTraceRecv(int aIndex, const BYTE* lpMsg, int size, int encrypt)
{
	if (PacketTraceActive()) PacketTraceRecvImpl(aIndex, lpMsg, size, encrypt);
}

inline void PacketTraceSend(int aIndex, const BYTE* lpMsg, int size)
{
	if (PacketTraceActive()) PacketTraceSendImpl(aIndex, lpMsg, size);
}

// Para los `return` de los handlers: deja escrito por qué el server cortó.
//   TRACE_REJECT(aIndex, 0x1D, -1, "MultiSkillIndex=%d", lpObj->MultiSkillIndex);
// Los argumentos sólo se evalúan con la captura prendida.
#define TRACE_REJECT(aIndex, head, sub, ...) \
	do { if (PacketTraceActive()) PacketTraceDecisionImpl((aIndex), (head), (sub), __VA_ARGS__); } while (0)

// Mediciones que no son rechazo (el inicio de cada camino, el intervalo entre
// ataques): sirven para elegir las tolerancias de los chequeos en modo observar.
#define TRACE_NOTE(aIndex, head, sub, fmt, ...) \
	TRACE_REJECT((aIndex), (head), (sub), "nota: " fmt, ##__VA_ARGS__)
