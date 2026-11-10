#pragma once

// Traza de paquetes del server (Fase 5, sección 5c del plan).
//
// Layout del anillo en memoria compartida que comparten el GameServer (el que
// escribe) y mu-trace (el que lee). Lo incluyen los dos, así que cualquier
// cambio de layout tiene que subir kVersion.
//
// - El server crea el segmento al arrancar si MU_TRACE_ENABLED=1. Sin eso no
//   hay segmento y cada punto de captura cuesta una comparación de puntero.
// - Con el segmento creado, el server sólo copia cuando un lector puso
//   `capture` en 1. Sin lector, cada punto de captura cuesta además una
//   lectura atómica.
// - Los productores del server se serializan entre ellos con un mutex local
//   (sólo se toma con la captura prendida). Hacia el lector el anillo es de un
//   productor y un consumidor: el server publica `writePos` con release y el
//   lector avanza `readPos`. Si no hay lugar, el registro se descarta y se
//   cuenta en `dropped`; el server nunca espera al lector.

#include <atomic>
#include <cstdint>

namespace MuTrace
{
	constexpr uint32_t kMagic = 0x3152544D;   // "MTR1"
	constexpr uint32_t kVersion = 2;
	constexpr const char* kShmPrefix = "/mu-trace-";   // + puerto del GameServer
	constexpr uint32_t kDefaultRingMB = 16;
	constexpr uint32_t kMaxPayload = 1024;   // bytes de paquete que se copian como máximo
	constexpr uint32_t kAccountSize = 16;

	enum Dir : uint8_t
	{
		DirRecv = 0,
		DirSend = 1,
		DirDecision = 2,
		DirPad = 0xFF,   // relleno al final del anillo; el lector lo saltea
	};

	enum ConsumerKind : uint32_t
	{
		ConsumerNone = 0,
		ConsumerTail = 1,
		ConsumerRecord = 2,
	};

	// Archivos que escribe `mu-trace record`: esta cabecera y después los
	// registros tal cual salen del anillo (RecordHeader + datos, alineados a 8).
	struct FileHeader
	{
		char magic[4];        // "MTF1"
		uint32_t version;     // kVersion del anillo que los produjo
		uint32_t serverPort;
		uint32_t reserved;
		uint64_t startTimeUs;
	};

	static_assert(sizeof(FileHeader) == 24, "FileHeader cambió de tamaño");

	constexpr uint32_t DirBit(Dir d) { return 1u << d; }
	constexpr uint32_t kAllDirs = DirBit(DirRecv) | DirBit(DirSend) | DirBit(DirDecision);

	// Cabecera de cada registro. Le siguen `dataSize` bytes: el paquete (recv/send)
	// o el texto del motivo (decision). El tamaño total se redondea a 8.
	struct RecordHeader
	{
		uint32_t size;        // total del registro, cabecera incluida
		uint8_t dir;          // Dir
		uint8_t frame;        // C1..C4 tal como viajó por la red (recv: antes de descifrar)
		uint8_t head;
		uint8_t reserved0;
		int16_t sub;          // subcódigo, o -1
		uint16_t dataSize;    // bytes copiados (recortado a kMaxPayload)
		uint16_t origSize;    // tamaño real del paquete
		uint16_t reserved1;
		int32_t aIndex;
		uint32_t reserved2;
		uint64_t timeUs;      // tiempo real en microsegundos
		char account[kAccountSize];
	};

	static_assert(sizeof(RecordHeader) == 48, "RecordHeader cambió de tamaño");

	struct alignas(64) RingHeader
	{
		uint32_t magic;
		uint32_t version;
		uint64_t capacity;    // bytes del área de datos; potencia de dos
		uint32_t serverPid;
		uint32_t serverPort;
		uint64_t startTimeUs;

		// Escritos por el lector; el server los lee en cada captura.
		std::atomic<uint32_t> capture;      // 0 = nadie leyendo, no se copia nada
		std::atomic<int32_t> consumerPid;
		std::atomic<uint32_t> consumerKind; // ConsumerKind
		char recordDir[128];                // con un grabador: dónde escribe
		std::atomic<uint32_t> dirMask;      // DirBit(...)
		std::atomic<int32_t> filterIndex;   // -1 = todos
		std::atomic<int32_t> filterHead;    // -1 = todos
		std::atomic<int32_t> filterSub;     // -1 = todos
		std::atomic<uint32_t> filterSeq;    // impar mientras el lector cambia filterAccount
		char filterAccount[kAccountSize];   // vacío = todas

		alignas(64) std::atomic<uint64_t> writePos;
		alignas(64) std::atomic<uint64_t> readPos;
		std::atomic<uint64_t> dropped;
		std::atomic<uint64_t> written;
	};

	static_assert(std::atomic<uint64_t>::is_always_lock_free, "el anillo necesita atómicos de 64 bits sin lock");
	static_assert(std::atomic<uint32_t>::is_always_lock_free, "el anillo necesita atómicos de 32 bits sin lock");

	inline uint8_t* RingData(RingHeader* h)
	{
		return reinterpret_cast<uint8_t*>(h) + sizeof(RingHeader);
	}

	inline uint64_t RingBytes(uint64_t capacity)
	{
		return sizeof(RingHeader) + capacity;
	}

	inline uint32_t Align8(uint32_t v)
	{
		return (v + 7u) & ~7u;
	}
}
