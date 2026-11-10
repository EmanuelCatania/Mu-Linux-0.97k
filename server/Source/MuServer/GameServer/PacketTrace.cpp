#include "stdafx.h"
#include "PacketTrace.h"

namespace MuTrace
{
	RingHeader* gRing = nullptr;
}

#ifndef __linux__

void PacketTraceInit(int) {}
void PacketTraceShutdown() {}
void PacketTraceRecvImpl(int, const BYTE*, int, int) {}
void PacketTraceSendImpl(int, const BYTE*, int) {}
void PacketTraceDecisionImpl(int, int, int, const char*, ...) {}

#else

#include "Guild.h"
#include "Protocol.h"
#include "User.h"
#include "Util.h"
#include "Warehouse.h"

#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <mutex>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

using namespace MuTrace;

namespace
{
	std::mutex gProducerMutex;   // serializa a los productores; sólo con captura prendida
	char gShmName[64] = {};
	size_t gMapBytes = 0;

	uint64_t NowUs()
	{
		timespec ts {};
		clock_gettime(CLOCK_REALTIME, &ts);
		return (uint64_t)ts.tv_sec * 1000000ull + (uint64_t)ts.tv_nsec / 1000ull;
	}

	uint64_t RoundPow2(uint64_t v)
	{
		uint64_t p = 1;
		while (p < v) p <<= 1;
		return p;
	}

	const char* AccountOf(int aIndex)
	{
		return OBJECT_RANGE(aIndex) ? gObj[aIndex].Account : "";
	}

	bool Matches(RingHeader* ring, Dir dir, int aIndex, int head, int sub, const char* account)
	{
		if ((ring->dirMask.load(std::memory_order_relaxed) & DirBit(dir)) == 0) return false;

		int fi = ring->filterIndex.load(std::memory_order_relaxed);
		if (fi >= 0 && fi != aIndex) return false;

		int fh = ring->filterHead.load(std::memory_order_relaxed);
		if (fh >= 0 && fh != head) return false;

		int fs = ring->filterSub.load(std::memory_order_relaxed);
		if (fs >= 0 && fs != sub) return false;

		if (ring->filterAccount[0] != 0)
		{
			// El lector cambia la cuenta con filterSeq impar; si lo agarramos a
			// mitad de camino, comparamos igual: es sólo un filtro de diagnóstico.
			char want[kAccountSize];
			memcpy(want, ring->filterAccount, sizeof(want));
			want[kAccountSize - 1] = 0;
			if (want[0] != 0 && strncmp(want, account, kAccountSize - 1) != 0) return false;
		}

		return true;
	}

	void Write(Dir dir, int frame, int head, int sub, int aIndex, int origSize, const void* data, uint32_t dataSize)
	{
		RingHeader* ring = gRing;
		const char* account = AccountOf(aIndex);

		if (!Matches(ring, dir, aIndex, head, sub, account)) return;

		if (dataSize > kMaxPayload) dataSize = kMaxPayload;

		const uint32_t size = Align8((uint32_t)sizeof(RecordHeader) + dataSize);
		const uint64_t cap = ring->capacity;
		uint8_t* base = RingData(ring);

		std::lock_guard<std::mutex> lock(gProducerMutex);

		uint64_t w = ring->writePos.load(std::memory_order_relaxed);
		const uint64_t r = ring->readPos.load(std::memory_order_acquire);
		uint64_t off = w & (cap - 1);
		const uint64_t tail = cap - off;
		const uint64_t need = size + ((tail < size) ? tail : 0);

		if (w + need - r > cap)
		{
			ring->dropped.fetch_add(1, std::memory_order_relaxed);
			return;
		}

		if (tail < size)
		{
			// No entra antes del final: relleno y vuelta al principio.
			RecordHeader* pad = reinterpret_cast<RecordHeader*>(base + off);
			pad->size = (uint32_t)tail;
			pad->dir = DirPad;
			w += tail;
			off = 0;
		}

		RecordHeader* rec = reinterpret_cast<RecordHeader*>(base + off);
		memset(rec, 0, sizeof(RecordHeader));
		rec->size = size;
		rec->dir = (uint8_t)dir;
		rec->frame = (uint8_t)frame;
		rec->head = (uint8_t)head;
		rec->sub = (int16_t)sub;
		rec->dataSize = (uint16_t)dataSize;
		rec->origSize = (uint16_t)((origSize > 0xFFFF) ? 0xFFFF : origSize);
		rec->aIndex = aIndex;
		rec->timeUs = NowUs();
		strncpy(rec->account, account, kAccountSize - 1);

		if (dataSize > 0) memcpy(reinterpret_cast<uint8_t*>(rec) + sizeof(RecordHeader), data, dataSize);

		ring->written.fetch_add(1, std::memory_order_relaxed);
		ring->writePos.store(w + size, std::memory_order_release);
	}

	void Blank(BYTE* buf, int size, size_t offset, size_t length)
	{
		if ((int)offset >= size) return;
		if ((int)(offset + length) > size) length = size - offset;
		memset(buf + offset, '*', length);
	}

	// Tapa contraseñas y códigos personales antes de copiarlos al anillo. Los
	// offsets salen de los structs, así que siguen valiendo si cambia el layout.
	void MaskSecrets(BYTE* buf, int size, int head, int sub)
	{
		if (head == 0xF1 && sub == 0x01)
		{
			Blank(buf, size, offsetof(PMSG_CONNECT_ACCOUNT_RECV, password), sizeof(((PMSG_CONNECT_ACCOUNT_RECV*)0)->password));
		}
		else if (head == 0xF3 && sub == 0x02)
		{
			Blank(buf, size, offsetof(PMSG_CHARACTER_DELETE_RECV, PersonalCode), sizeof(((PMSG_CHARACTER_DELETE_RECV*)0)->PersonalCode));
		}
		else if (head == 0x53)
		{
			Blank(buf, size, offsetof(PMSG_GUILD_DELETE_RECV, PersonalCode), sizeof(((PMSG_GUILD_DELETE_RECV*)0)->PersonalCode));
		}
		else if (head == 0x83)
		{
			Blank(buf, size, offsetof(PMSG_WAREHOUSE_PASSWORD_RECV, password), sizeof(((PMSG_WAREHOUSE_PASSWORD_RECV*)0)->password));
			Blank(buf, size, offsetof(PMSG_WAREHOUSE_PASSWORD_RECV, PersonalCode), sizeof(((PMSG_WAREHOUSE_PASSWORD_RECV*)0)->PersonalCode));
		}
	}

	void Split(const BYTE* lpMsg, int size, int& head, int& sub)
	{
		head = -1;
		sub = -1;
		if (size < 3) return;

		if (lpMsg[0] == 0xC1 || lpMsg[0] == 0xC3)
		{
			head = lpMsg[2];
			if (size > 3) sub = lpMsg[3];
		}
		else if (size >= 4)
		{
			head = lpMsg[3];
			if (size > 4) sub = lpMsg[4];
		}
	}
}

void PacketTraceInit(int port)
{
	const char* enabled = getenv("MU_TRACE_ENABLED");
	if (enabled == nullptr || strcmp(enabled, "1") != 0) return;

	uint64_t mb = kDefaultRingMB;
	if (const char* env = getenv("MU_TRACE_RING_MB"))
	{
		long v = strtol(env, nullptr, 10);
		if (v >= 1 && v <= 1024) mb = (uint64_t)v;
	}

	const uint64_t capacity = RoundPow2(mb * 1024ull * 1024ull);
	const size_t bytes = (size_t)RingBytes(capacity);

	snprintf(gShmName, sizeof(gShmName), "%s%d", kShmPrefix, port);

	int fd = shm_open(gShmName, O_CREAT | O_RDWR, 0600);
	if (fd < 0)
	{
		LogAdd(LOG_RED, "[PacketTrace] shm_open(%s) falló: %s", gShmName, strerror(errno));
		return;
	}

	if (ftruncate(fd, (off_t)bytes) != 0)
	{
		LogAdd(LOG_RED, "[PacketTrace] ftruncate(%s, %zu) falló: %s", gShmName, bytes, strerror(errno));
		close(fd);
		shm_unlink(gShmName);
		return;
	}

	void* map = mmap(nullptr, bytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);

	if (map == MAP_FAILED)
	{
		LogAdd(LOG_RED, "[PacketTrace] mmap(%s) falló: %s", gShmName, strerror(errno));
		shm_unlink(gShmName);
		return;
	}

	RingHeader* ring = static_cast<RingHeader*>(map);

	// Un segmento que sobrevivió a un server anterior se reinicia entero. El
	// magic va último para que un lector no vea una cabecera a medio escribir.
	ring->magic = 0;
	std::atomic_thread_fence(std::memory_order_release);
	ring->version = kVersion;
	ring->capacity = capacity;
	ring->serverPid = (uint32_t)getpid();
	ring->serverPort = (uint32_t)port;
	ring->startTimeUs = NowUs();
	ring->capture.store(0);
	ring->consumerPid.store(0);
	ring->consumerKind.store(ConsumerNone);
	memset(ring->recordDir, 0, sizeof(ring->recordDir));
	ring->dirMask.store(kAllDirs);
	ring->filterIndex.store(-1);
	ring->filterHead.store(-1);
	ring->filterSub.store(-1);
	ring->filterSeq.store(0);
	memset(ring->filterAccount, 0, sizeof(ring->filterAccount));
	ring->writePos.store(0);
	ring->readPos.store(0);
	ring->dropped.store(0);
	ring->written.store(0);
	std::atomic_thread_fence(std::memory_order_release);
	ring->magic = kMagic;

	gMapBytes = bytes;
	MuTrace::gRing = ring;

	LogAdd(LOG_BLUE, "[PacketTrace] Anillo %s listo (%llu MB). Leer con: mu-trace tail", gShmName, (unsigned long long)(capacity >> 20));
}

void PacketTraceShutdown()
{
	RingHeader* ring = MuTrace::gRing;
	if (ring == nullptr) return;

	MuTrace::gRing = nullptr;
	munmap(ring, gMapBytes);
	shm_unlink(gShmName);
}

void PacketTraceRecvImpl(int aIndex, const BYTE* lpMsg, int size, int encrypt)
{
	if (size < 3) return;

	int head, sub;
	Split(lpMsg, size, head, sub);

	const int frame = (lpMsg[0] == 0xC1) ? (encrypt ? 0xC3 : 0xC1) : (encrypt ? 0xC4 : 0xC2);

	BYTE copy[kMaxPayload];
	const int n = (size > (int)kMaxPayload) ? (int)kMaxPayload : size;
	memcpy(copy, lpMsg, n);
	MaskSecrets(copy, n, head, sub);

	Write(DirRecv, frame, head, sub, aIndex, size, copy, (uint32_t)n);
}

void PacketTraceSendImpl(int aIndex, const BYTE* lpMsg, int size)
{
	if (size < 3) return;

	int head, sub;
	Split(lpMsg, size, head, sub);

	Write(DirSend, lpMsg[0], head, sub, aIndex, size, lpMsg, (uint32_t)size);
}

void PacketTraceDecisionImpl(int aIndex, int head, int sub, const char* fmt, ...)
{
	char text[256];
	va_list args;
	va_start(args, fmt);
	int n = vsnprintf(text, sizeof(text), fmt, args);
	va_end(args);

	if (n < 0) return;
	if (n >= (int)sizeof(text)) n = sizeof(text) - 1;

	Write(DirDecision, 0, head & 0xFF, sub, aIndex, n, text, (uint32_t)n);
}

#endif
