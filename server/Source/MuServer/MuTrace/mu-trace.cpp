// mu-trace: lector de la traza de paquetes del GameServer (Fase 5, sección 5c).
//
//   mu-trace status
//   mu-trace tail [--ring NOMBRE] [--account CUENTA] [--index N] [--op HH[:SS]]
//                 [--dir recv,send,decision] [--bytes N]
//
// El server tiene que haber arrancado con MU_TRACE_ENABLED=1. Mientras
// `tail` corre, el server copia los paquetes que pasan el filtro; al salir
// (Ctrl+C) la captura se apaga y el server vuelve a no copiar nada.
// Hay un solo lector a la vez.

#include "TraceRing.h"

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <dirent.h>
#include <fcntl.h>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

using namespace MuTrace;

namespace
{
	struct PacketName
	{
		int head;
		int sub;
		const char* name;
	};

#include "PacketNames.inc"

	volatile sig_atomic_t gStop = 0;

	void OnSignal(int) { gStop = 1; }

	const char* Lookup(const PacketName* table, size_t count, int head, int sub)
	{
		const char* any = nullptr;
		for (size_t i = 0; i < count; i++)
		{
			if (table[i].head != head) continue;
			if (table[i].sub == sub) return table[i].name;
			if (table[i].sub < 0 && any == nullptr) any = table[i].name;
		}
		return any;
	}

	const char* NameOf(const RecordHeader* rec)
	{
		if (rec->dir == DirRecv) return Lookup(kRecvNames, sizeof(kRecvNames) / sizeof(kRecvNames[0]), rec->head, rec->sub);
		if (rec->dir == DirSend) return Lookup(kSendNames, sizeof(kSendNames) / sizeof(kSendNames[0]), rec->head, rec->sub);
		return nullptr;
	}

	// Un subcódigo sólo tiene sentido en los opcodes que lo usan; para el resto
	// el byte 3 es el primer dato del paquete.
	bool HasSub(const RecordHeader* rec)
	{
		const PacketName* table = (rec->dir == DirSend) ? kSendNames : kRecvNames;
		const size_t count = (rec->dir == DirSend) ? sizeof(kSendNames) / sizeof(kSendNames[0]) : sizeof(kRecvNames) / sizeof(kRecvNames[0]);
		for (size_t i = 0; i < count; i++)
		{
			if (table[i].head == rec->head && table[i].sub != -1) return true;
		}
		return false;
	}

	std::vector<std::string> ListRings()
	{
		std::vector<std::string> out;
		DIR* dir = opendir("/dev/shm");
		if (dir == nullptr) return out;
		const std::string prefix = std::string(kShmPrefix).substr(1);
		while (dirent* e = readdir(dir))
		{
			if (strncmp(e->d_name, prefix.c_str(), prefix.size()) == 0) out.push_back(std::string("/") + e->d_name);
		}
		closedir(dir);
		return out;
	}

	struct Mapping
	{
		RingHeader* ring = nullptr;
		size_t bytes = 0;
	};

	bool Open(const std::string& name, Mapping& m)
	{
		int fd = shm_open(name.c_str(), O_RDWR, 0);
		if (fd < 0)
		{
			fprintf(stderr, "mu-trace: no se pudo abrir %s: %s\n", name.c_str(), strerror(errno));
			return false;
		}

		struct stat st {};
		if (fstat(fd, &st) != 0 || (size_t)st.st_size < sizeof(RingHeader))
		{
			fprintf(stderr, "mu-trace: %s no tiene una cabecera válida\n", name.c_str());
			close(fd);
			return false;
		}

		void* map = mmap(nullptr, (size_t)st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		close(fd);
		if (map == MAP_FAILED)
		{
			fprintf(stderr, "mu-trace: mmap(%s) falló: %s\n", name.c_str(), strerror(errno));
			return false;
		}

		RingHeader* ring = static_cast<RingHeader*>(map);
		if (ring->magic != kMagic || ring->version != kVersion || RingBytes(ring->capacity) > (uint64_t)st.st_size)
		{
			fprintf(stderr, "mu-trace: %s es de otra versión (magic %08X, versión %u); recompilar mu-trace\n", name.c_str(), ring->magic, ring->version);
			munmap(map, (size_t)st.st_size);
			return false;
		}

		m.ring = ring;
		m.bytes = (size_t)st.st_size;
		return true;
	}

	bool PickRing(const char* wanted, std::string& name)
	{
		if (wanted != nullptr)
		{
			name = (wanted[0] == '/') ? wanted : std::string("/") + wanted;
			return true;
		}

		std::vector<std::string> rings = ListRings();
		if (rings.empty())
		{
			fprintf(stderr, "mu-trace: no hay ningún anillo en /dev/shm. ¿El GameServer arrancó con MU_TRACE_ENABLED=1?\n");
			return false;
		}
		if (rings.size() > 1)
		{
			fprintf(stderr, "mu-trace: hay varios GameServer; elegí uno con --ring:\n");
			for (const std::string& r : rings) fprintf(stderr, "  %s\n", r.c_str());
			return false;
		}
		name = rings[0];
		return true;
	}

	bool ProcessAlive(int pid)
	{
		return pid > 0 && (kill(pid, 0) == 0 || errno == EPERM);
	}

	void FormatTime(uint64_t us, char* out, size_t size)
	{
		time_t secs = (time_t)(us / 1000000ull);
		tm t {};
		localtime_r(&secs, &t);
		snprintf(out, size, "%02d:%02d:%02d.%03u", t.tm_hour, t.tm_min, t.tm_sec, (unsigned)((us / 1000ull) % 1000ull));
	}

	void Print(const RecordHeader* rec, int maxBytes)
	{
		char when[16];
		FormatTime(rec->timeUs, when, sizeof(when));

		const uint8_t* data = reinterpret_cast<const uint8_t*>(rec) + sizeof(RecordHeader);
		char account[kAccountSize + 1] = {};
		memcpy(account, rec->account, kAccountSize);

		if (rec->dir == DirDecision)
		{
			char op[8] = "--";
			if (rec->head != 0xFF) snprintf(op, sizeof(op), (rec->sub >= 0) ? "%02X:%02X" : "%02X", rec->head, rec->sub);
			printf("%s  DECIDE  #%-5d %-10s %-5s  %.*s\n", when, rec->aIndex, account, op, (int)rec->dataSize, (const char*)data);
			return;
		}

		char op[8];
		if (rec->sub >= 0 && HasSub(rec)) snprintf(op, sizeof(op), "%02X:%02X", rec->head, rec->sub);
		else snprintf(op, sizeof(op), "%02X", rec->head);

		const char* name = NameOf(rec);
		printf("%s  %s  #%-5d %-10s %02X %-5s  %-26s %4uB",
			when, (rec->dir == DirRecv) ? "RECV  " : "SEND  ", rec->aIndex, account,
			rec->frame, op, (name != nullptr) ? name : "?", (unsigned)rec->origSize);

		int n = (maxBytes < (int)rec->dataSize) ? maxBytes : (int)rec->dataSize;
		if (n > 0)
		{
			printf(" ");
			for (int i = 0; i < n; i++) printf(" %02X", data[i]);
			if (n < (int)rec->origSize) printf(" …");
		}
		printf("\n");
	}

	int ParseByte(const char* s)
	{
		char* end = nullptr;
		long v = strtol(s, &end, 16);
		if (end == s || v < 0 || v > 0xFF) return -2;
		return (int)v;
	}

	void Usage()
	{
		fprintf(stderr,
			"uso:\n"
			"  mu-trace status [--ring NOMBRE]\n"
			"  mu-trace tail [--ring NOMBRE] [--account CUENTA] [--index N] [--op HH[:SS]]\n"
			"                [--dir recv,send,decision] [--bytes N]\n"
			"\n"
			"  --op      opcode en hex (1D, F3:03)\n"
			"  --dir     qué mostrar; por defecto las tres\n"
			"  --bytes   cuántos bytes del paquete mostrar (por defecto 32, 0 = ninguno)\n");
	}

	int CmdStatus(const char* ringName)
	{
		std::vector<std::string> rings;
		if (ringName != nullptr) rings.push_back((ringName[0] == '/') ? ringName : std::string("/") + ringName);
		else rings = ListRings();

		if (rings.empty())
		{
			printf("No hay anillos de traza. El GameServer los crea con MU_TRACE_ENABLED=1.\n");
			return 0;
		}

		for (const std::string& name : rings)
		{
			Mapping m;
			if (!Open(name, m)) continue;
			RingHeader* r = m.ring;
			const int consumer = r->consumerPid.load();
			printf("%s: GameServer pid %u puerto %u, anillo %llu MB\n", name.c_str(), r->serverPid, r->serverPort, (unsigned long long)(r->capacity >> 20));
			printf("  captura %s%s, registros %llu, descartados %llu\n",
				r->capture.load() ? "prendida" : "apagada",
				ProcessAlive(consumer) ? (" (lector pid " + std::to_string(consumer) + ")").c_str() : "",
				(unsigned long long)r->written.load(), (unsigned long long)r->dropped.load());
			munmap(r, m.bytes);
		}
		return 0;
	}

	int CmdTail(int argc, char** argv)
	{
		const char* ringName = nullptr;
		const char* account = nullptr;
		int index = -1, head = -1, sub = -1, maxBytes = 32;
		uint32_t dirs = kAllDirs;

		for (int i = 0; i < argc; i++)
		{
			const std::string a = argv[i];
			const char* v = (i + 1 < argc) ? argv[i + 1] : nullptr;
			if (v == nullptr) { Usage(); return 2; }
			i++;
			if (a == "--ring") ringName = v;
			else if (a == "--account") account = v;
			else if (a == "--index") index = atoi(v);
			else if (a == "--bytes") maxBytes = atoi(v);
			else if (a == "--op")
			{
				std::string s = v;
				size_t colon = s.find(':');
				head = ParseByte(s.substr(0, colon).c_str());
				if (colon != std::string::npos) sub = ParseByte(s.substr(colon + 1).c_str());
				if (head < 0 || sub < -1) { fprintf(stderr, "mu-trace: --op inválido: %s\n", v); return 2; }
			}
			else if (a == "--dir")
			{
				dirs = 0;
				std::string s = v;
				for (size_t pos = 0; pos <= s.size();)
				{
					size_t comma = s.find(',', pos);
					std::string d = s.substr(pos, (comma == std::string::npos) ? std::string::npos : comma - pos);
					if (d == "recv") dirs |= DirBit(DirRecv);
					else if (d == "send") dirs |= DirBit(DirSend);
					else if (d == "decision") dirs |= DirBit(DirDecision);
					else { fprintf(stderr, "mu-trace: --dir inválido: %s\n", d.c_str()); return 2; }
					if (comma == std::string::npos) break;
					pos = comma + 1;
				}
			}
			else { Usage(); return 2; }
		}

		std::string name;
		if (!PickRing(ringName, name)) return 1;

		Mapping m;
		if (!Open(name, m)) return 1;
		RingHeader* ring = m.ring;

		const int other = ring->consumerPid.load();
		if (other != (int)getpid() && ProcessAlive(other))
		{
			fprintf(stderr, "mu-trace: ya hay otro lector en %s (pid %d)\n", name.c_str(), other);
			return 1;
		}

		signal(SIGINT, OnSignal);
		signal(SIGTERM, OnSignal);
		signal(SIGHUP, OnSignal);

		ring->consumerPid.store((int32_t)getpid());
		ring->dirMask.store(dirs);
		ring->filterIndex.store(index);
		ring->filterHead.store(head);
		ring->filterSub.store(sub);
		ring->filterSeq.fetch_add(1);
		memset(ring->filterAccount, 0, kAccountSize);
		if (account != nullptr) strncpy(ring->filterAccount, account, kAccountSize - 1);
		ring->filterSeq.fetch_add(1);

		// Lo que quedó de una captura anterior no se muestra.
		ring->readPos.store(ring->writePos.load(std::memory_order_acquire), std::memory_order_release);
		uint64_t dropped = ring->dropped.load();
		ring->capture.store(1);

		fprintf(stderr, "mu-trace: leyendo %s (GameServer pid %u). Ctrl+C para salir.\n", name.c_str(), ring->serverPid);

		const uint32_t serverPid = ring->serverPid;
		const uint64_t cap = ring->capacity;
		const uint8_t* base = RingData(ring);
		int idle = 0;

		while (!gStop)
		{
			uint64_t r = ring->readPos.load(std::memory_order_relaxed);
			const uint64_t w = ring->writePos.load(std::memory_order_acquire);

			if (r == w)
			{
				fflush(stdout);
				usleep(20000);
				if (++idle % 50 == 0)
				{
					// Cada segundo sin datos: ¿el server sigue vivo y es el mismo?
					if (ring->magic != kMagic || ring->serverPid != serverPid || !ProcessAlive((int)serverPid))
					{
						fprintf(stderr, "mu-trace: el GameServer se reinició o se cerró; volvé a correr mu-trace tail\n");
						break;
					}
				}
				continue;
			}
			idle = 0;

			while (r != w && !gStop)
			{
				const RecordHeader* rec = reinterpret_cast<const RecordHeader*>(base + (r & (cap - 1)));
				if (rec->size < 8 || rec->size > cap)
				{
					fprintf(stderr, "mu-trace: registro corrupto en %llu; me resincronizo\n", (unsigned long long)r);
					r = w;
					break;
				}
				if (rec->dir != DirPad) Print(rec, maxBytes);
				r += rec->size;
			}
			ring->readPos.store(r, std::memory_order_release);

			const uint64_t d = ring->dropped.load(std::memory_order_relaxed);
			if (d != dropped)
			{
				fprintf(stderr, "mu-trace: el anillo se llenó y se descartaron %llu registros\n", (unsigned long long)(d - dropped));
				dropped = d;
			}
		}

		if (ring->consumerPid.load() == (int32_t)getpid())
		{
			ring->capture.store(0);
			ring->consumerPid.store(0);
		}
		fflush(stdout);
		munmap(ring, m.bytes);
		return 0;
	}
}

int main(int argc, char** argv)
{
	setvbuf(stdout, nullptr, _IOFBF, 1 << 16);

	if (argc < 2)
	{
		Usage();
		return 2;
	}

	const std::string cmd = argv[1];
	if (cmd == "tail") return CmdTail(argc - 2, argv + 2);
	if (cmd == "status")
	{
		const char* ring = (argc >= 4 && strcmp(argv[2], "--ring") == 0) ? argv[3] : nullptr;
		return CmdStatus(ring);
	}

	Usage();
	return 2;
}
