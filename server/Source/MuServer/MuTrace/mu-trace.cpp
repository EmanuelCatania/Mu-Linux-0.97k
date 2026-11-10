// mu-trace: lector de la traza de paquetes del GameServer (Fase 5, sección 5c).
//
//   mu-trace status
//   mu-trace tail   [filtros] [--bytes N]
//   mu-trace record --out DIR [filtros] [--rotate-mb N] [--rotate-min N] [--keep-mb N]
//   mu-trace read   ARCHIVO|DIR... [filtros] [--from T] [--to T] [--follow] [--bytes N]
//
//   filtros: --ring NOMBRE --account CUENTA --index N --op HH[:SS]
//            --dir recv,send,decision
//
// El server tiene que haber arrancado con MU_TRACE_ENABLED=1. Mientras hay un
// lector (tail o record) el server copia los paquetes que pasan el filtro; sin
// lector no copia nada. Hay un solo lector a la vez: con el grabador corriendo,
// lo que está pasando se mira con `read --follow` sobre su carpeta.

#include "TraceRing.h"

#include <algorithm>
#include <cerrno>
#include <climits>
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

	void InstallSignals()
	{
		signal(SIGINT, OnSignal);
		signal(SIGTERM, OnSignal);
		signal(SIGHUP, OnSignal);
		signal(SIGPIPE, OnSignal);
	}

	// ── Opciones ────────────────────────────────────────────────────────────

	struct Options
	{
		const char* ring = nullptr;
		std::string account;
		int index = -1;
		int head = -1;
		int sub = -1;
		uint32_t dirs = kAllDirs;
		int maxBytes = 32;

		// record
		std::string out;
		uint64_t rotateMB = 64;
		uint64_t rotateMin = 60;
		uint64_t keepMB = 1024;

		// read
		std::vector<std::string> paths;
		uint64_t fromUs = 0;
		uint64_t toUs = UINT64_MAX;
		bool follow = false;
	};

	void Usage()
	{
		fprintf(stderr,
			"uso:\n"
			"  mu-trace status [--ring NOMBRE]\n"
			"  mu-trace tail   [filtros] [--bytes N]\n"
			"  mu-trace record --out DIR [filtros] [--rotate-mb N] [--rotate-min N] [--keep-mb N]\n"
			"  mu-trace read   ARCHIVO|DIR... [filtros] [--from T] [--to T] [--follow] [--bytes N]\n"
			"\n"
			"filtros:\n"
			"  --ring NOMBRE     anillo a leer (hace falta si hay varios GameServer)\n"
			"  --account CUENTA  sólo esa cuenta\n"
			"  --index N         sólo ese índice de objeto\n"
			"  --op HH[:SS]      sólo ese opcode, en hex (1D, F3:03)\n"
			"  --dir LISTA       recv,send,decision (por defecto las tres)\n"
			"\n"
			"  --bytes N         bytes del paquete a mostrar (32 por defecto, 0 = ninguno)\n"
			"  --rotate-mb/-min  archivo nuevo cada N MB o N minutos (64 / 60)\n"
			"  --keep-mb N       borra los archivos más viejos pasado ese total (1024)\n"
			"  --from/--to T     'AAAA-MM-DD HH:MM:SS' o 'HH:MM:SS' (hoy)\n"
			"  --follow          sigue el archivo más nuevo de la carpeta\n");
	}

	int ParseByte(const std::string& s)
	{
		char* end = nullptr;
		long v = strtol(s.c_str(), &end, 16);
		if (s.empty() || *end != 0 || v < 0 || v > 0xFF) return -2;
		return (int)v;
	}

	bool ParseTime(const char* s, uint64_t& out)
	{
		tm t {};
		const char* end = strptime(s, "%Y-%m-%d %H:%M:%S", &t);
		if (end == nullptr || *end != 0)
		{
			time_t now = time(nullptr);
			localtime_r(&now, &t);
			end = strptime(s, "%H:%M:%S", &t);
			if (end == nullptr || *end != 0) return false;
		}
		t.tm_isdst = -1;
		time_t secs = mktime(&t);
		if (secs < 0) return false;
		out = (uint64_t)secs * 1000000ull;
		return true;
	}

	bool ParseOptions(int argc, char** argv, Options& o, bool allowPaths)
	{
		for (int i = 0; i < argc; i++)
		{
			const std::string a = argv[i];

			if (a.size() < 2 || a.compare(0, 2, "--") != 0)
			{
				if (!allowPaths) { Usage(); return false; }
				o.paths.push_back(a);
				continue;
			}

			if (a == "--follow") { o.follow = true; continue; }

			const char* v = (i + 1 < argc) ? argv[++i] : nullptr;
			if (v == nullptr) { fprintf(stderr, "mu-trace: falta el valor de %s\n", a.c_str()); return false; }

			if (a == "--ring") o.ring = v;
			else if (a == "--account") o.account = v;
			else if (a == "--index") o.index = atoi(v);
			else if (a == "--bytes") o.maxBytes = atoi(v);
			else if (a == "--out") o.out = v;
			else if (a == "--rotate-mb") o.rotateMB = strtoull(v, nullptr, 10);
			else if (a == "--rotate-min") o.rotateMin = strtoull(v, nullptr, 10);
			else if (a == "--keep-mb") o.keepMB = strtoull(v, nullptr, 10);
			else if (a == "--from" || a == "--to")
			{
				if (!ParseTime(v, (a == "--from") ? o.fromUs : o.toUs))
				{
					fprintf(stderr, "mu-trace: hora inválida: %s\n", v);
					return false;
				}
			}
			else if (a == "--op")
			{
				const std::string s = v;
				const size_t colon = s.find(':');
				o.head = ParseByte(s.substr(0, colon));
				o.sub = (colon == std::string::npos) ? -1 : ParseByte(s.substr(colon + 1));
				if (o.head < 0 || o.sub < -1) { fprintf(stderr, "mu-trace: --op inválido: %s\n", v); return false; }
			}
			else if (a == "--dir")
			{
				o.dirs = 0;
				std::string s = v;
				size_t pos = 0;
				while (true)
				{
					const size_t comma = s.find(',', pos);
					const std::string d = s.substr(pos, (comma == std::string::npos) ? std::string::npos : comma - pos);
					if (d == "recv") o.dirs |= DirBit(DirRecv);
					else if (d == "send") o.dirs |= DirBit(DirSend);
					else if (d == "decision") o.dirs |= DirBit(DirDecision);
					else { fprintf(stderr, "mu-trace: --dir inválido: %s\n", d.c_str()); return false; }
					if (comma == std::string::npos) break;
					pos = comma + 1;
				}
			}
			else { fprintf(stderr, "mu-trace: opción desconocida: %s\n", a.c_str()); return false; }
		}

		if (o.rotateMB == 0) o.rotateMB = 1;
		if (o.rotateMin == 0) o.rotateMin = 1;
		return true;
	}

	// Mismo criterio que aplica el server; `read` lo usa sobre lo grabado.
	bool Passes(const RecordHeader* rec, const Options& o)
	{
		if (rec->dir == DirPad) return false;
		if ((o.dirs & DirBit((Dir)rec->dir)) == 0) return false;
		if (o.index >= 0 && rec->aIndex != o.index) return false;
		if (o.head >= 0 && rec->head != o.head) return false;
		if (o.sub >= 0 && rec->sub != o.sub) return false;
		if (rec->timeUs < o.fromUs || rec->timeUs > o.toUs) return false;
		if (!o.account.empty() && strncmp(o.account.c_str(), rec->account, kAccountSize) != 0) return false;
		return true;
	}

	// ── Impresión ───────────────────────────────────────────────────────────

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

	template <size_t N>
	const char* Lookup(const PacketName (&table)[N], int head, int sub) { return Lookup(table, N, head, sub); }

	// Un subcódigo sólo tiene sentido en los opcodes que lo usan; para el resto
	// el byte 3 es el primer dato del paquete.
	template <size_t N>
	bool HasSub(const PacketName (&table)[N], int head)
	{
		for (size_t i = 0; i < N; i++)
		{
			if (table[i].head == head && table[i].sub != -1) return true;
		}
		return false;
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

		const bool recv = (rec->dir == DirRecv);
		const bool sub = rec->sub >= 0 && (recv ? HasSub(kRecvNames, rec->head) : HasSub(kSendNames, rec->head));
		const char* name = recv ? Lookup(kRecvNames, rec->head, rec->sub) : Lookup(kSendNames, rec->head, rec->sub);

		char op[8];
		if (sub) snprintf(op, sizeof(op), "%02X:%02X", rec->head, rec->sub);
		else snprintf(op, sizeof(op), "%02X", rec->head);

		printf("%s  %s  #%-5d %-10s %02X %-5s  %-26s %4uB",
			when, recv ? "RECV  " : "SEND  ", rec->aIndex, account,
			rec->frame, op, (name != nullptr) ? name : "?", (unsigned)rec->origSize);

		const int n = (maxBytes < (int)rec->dataSize) ? maxBytes : (int)rec->dataSize;
		if (n > 0)
		{
			printf(" ");
			for (int i = 0; i < n; i++) printf(" %02X", data[i]);
			if (n < (int)rec->origSize) printf(" …");
		}
		printf("\n");
	}

	// ── Anillo ──────────────────────────────────────────────────────────────

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
		std::sort(out.begin(), out.end());
		return out;
	}

	struct Mapping
	{
		RingHeader* ring = nullptr;
		size_t bytes = 0;

		~Mapping()
		{
			if (ring != nullptr) munmap(ring, bytes);
		}
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
			fprintf(stderr, "mu-trace: %s es de otra versión (magic %08X, versión %u, este mu-trace lee la %u)\n", name.c_str(), ring->magic, ring->version, kVersion);
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

	// Toma el anillo como único lector, le pasa los filtros al server y entrega
	// cada registro a `sink`. `idle` se llama cuando no hay datos (unas 50
	// veces por segundo). Devuelve al terminar (señal o server caído).
	template <typename Sink, typename Idle>
	int Consume(const Options& o, ConsumerKind kind, const std::string& recordDir, Sink sink, Idle idle)
	{
		std::string name;
		if (!PickRing(o.ring, name)) return 1;

		Mapping m;
		if (!Open(name, m)) return 1;
		RingHeader* ring = m.ring;

		const int other = ring->consumerPid.load();
		if (other != (int)getpid() && ProcessAlive(other))
		{
			if (ring->consumerKind.load() == ConsumerRecord && ring->recordDir[0] != 0)
			{
				char dir[sizeof(ring->recordDir) + 1] = {};
				memcpy(dir, ring->recordDir, sizeof(ring->recordDir));
				fprintf(stderr, "mu-trace: el grabador (pid %d) ya está leyendo %s. Para verlo en vivo:\n  mu-trace read %s --follow\n", other, name.c_str(), dir);
			}
			else
			{
				fprintf(stderr, "mu-trace: ya hay otro lector en %s (pid %d)\n", name.c_str(), other);
			}
			return 1;
		}

		InstallSignals();

		ring->consumerPid.store((int32_t)getpid());
		ring->consumerKind.store(kind);
		memset(ring->recordDir, 0, sizeof(ring->recordDir));
		strncpy(ring->recordDir, recordDir.c_str(), sizeof(ring->recordDir) - 1);
		ring->dirMask.store(o.dirs);
		ring->filterIndex.store(o.index);
		ring->filterHead.store(o.head);
		ring->filterSub.store(o.sub);
		ring->filterSeq.fetch_add(1);
		memset(ring->filterAccount, 0, kAccountSize);
		strncpy(ring->filterAccount, o.account.c_str(), kAccountSize - 1);
		ring->filterSeq.fetch_add(1);

		// Lo que quedó de una captura anterior no se muestra.
		ring->readPos.store(ring->writePos.load(std::memory_order_acquire), std::memory_order_release);
		uint64_t dropped = ring->dropped.load();
		ring->capture.store(1);

		fprintf(stderr, "mu-trace: leyendo %s (GameServer pid %u). Ctrl+C para salir.\n", name.c_str(), ring->serverPid);

		const uint32_t serverPid = ring->serverPid;
		const uint64_t cap = ring->capacity;
		const uint8_t* base = RingData(ring);
		int idleTicks = 0;
		int rc = 0;

		while (!gStop)
		{
			uint64_t r = ring->readPos.load(std::memory_order_relaxed);
			const uint64_t w = ring->writePos.load(std::memory_order_acquire);

			if (r == w)
			{
				idle();
				usleep(20000);
				if (++idleTicks % 50 == 0)
				{
					// Cada segundo sin datos: ¿el server sigue vivo y es el mismo?
					if (ring->magic != kMagic || ring->serverPid != serverPid || !ProcessAlive((int)serverPid))
					{
						fprintf(stderr, "mu-trace: el GameServer se reinició o se cerró\n");
						rc = 3;
						break;
					}
				}
				continue;
			}
			idleTicks = 0;

			while (r != w && !gStop)
			{
				const RecordHeader* rec = reinterpret_cast<const RecordHeader*>(base + (r & (cap - 1)));
				if (rec->size < 8 || rec->size > cap || (rec->size & 7) != 0)
				{
					fprintf(stderr, "mu-trace: registro corrupto en %llu; me resincronizo\n", (unsigned long long)r);
					r = w;
					break;
				}
				if (rec->dir != DirPad) sink(rec);
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
			ring->consumerKind.store(ConsumerNone);
			ring->consumerPid.store(0);
		}
		return rc;
	}

	// ── Comandos ────────────────────────────────────────────────────────────

	int CmdStatus(const Options& o)
	{
		std::vector<std::string> rings;
		if (o.ring != nullptr) rings.push_back((o.ring[0] == '/') ? o.ring : std::string("/") + o.ring);
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
			std::string reader = "nadie";
			if (ProcessAlive(consumer))
			{
				reader = ((r->consumerKind.load() == ConsumerRecord) ? "grabador" : "tail") + std::string(" pid ") + std::to_string(consumer);
				if (r->consumerKind.load() == ConsumerRecord) reader += std::string(" en ") + std::string(r->recordDir, strnlen(r->recordDir, sizeof(r->recordDir)));
			}
			printf("%s: GameServer pid %u puerto %u, anillo %llu MB\n", name.c_str(), r->serverPid, r->serverPort, (unsigned long long)(r->capacity >> 20));
			printf("  captura %s, lector: %s\n", r->capture.load() ? "prendida" : "apagada", reader.c_str());
			printf("  registros %llu, descartados %llu\n", (unsigned long long)r->written.load(), (unsigned long long)r->dropped.load());
		}
		return 0;
	}

	int CmdTail(const Options& o)
	{
		setvbuf(stdout, nullptr, _IOFBF, 1 << 16);
		return Consume(o, ConsumerTail, std::string(),
			[&](const RecordHeader* rec) { Print(rec, o.maxBytes); },
			[]() { fflush(stdout); });
	}

	// Archivos de la carpeta del grabador, del más viejo al más nuevo (el
	// nombre lleva la fecha, así que el orden alfabético es el cronológico).
	std::vector<std::string> TraceFiles(const std::string& dirPath)
	{
		std::vector<std::string> out;
		DIR* dir = opendir(dirPath.c_str());
		if (dir == nullptr) return out;
		while (dirent* e = readdir(dir))
		{
			const size_t len = strlen(e->d_name);
			if (strncmp(e->d_name, "trace-", 6) == 0 && len > 4 && strcmp(e->d_name + len - 4, ".bin") == 0)
			{
				out.push_back(dirPath + "/" + e->d_name);
			}
		}
		closedir(dir);
		std::sort(out.begin(), out.end());
		return out;
	}

	bool IsDirectory(const std::string& path)
	{
		struct stat st {};
		return stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
	}

	uint64_t FileSize(const std::string& path)
	{
		struct stat st {};
		return (stat(path.c_str(), &st) == 0) ? (uint64_t)st.st_size : 0;
	}

	class Recorder
	{
	public:
		explicit Recorder(const Options& o) : m_o(o) {}
		~Recorder() { this->Close(); }

		bool Write(const RecordHeader* rec)
		{
			const uint64_t now = rec->timeUs;
			if (m_file == nullptr || m_bytes >= m_o.rotateMB * 1024ull * 1024ull || now - m_openedUs >= m_o.rotateMin * 60ull * 1000000ull)
			{
				if (!this->Rotate(now)) return false;
			}

			if (fwrite(rec, rec->size, 1, m_file) != 1)
			{
				fprintf(stderr, "mu-trace: no se pudo escribir %s: %s\n", m_path.c_str(), strerror(errno));
				return false;
			}
			m_bytes += rec->size;
			return true;
		}

		void Flush()
		{
			if (m_file != nullptr) fflush(m_file);
		}

		void Close()
		{
			if (m_file != nullptr) fclose(m_file);
			m_file = nullptr;
		}

		uint32_t port = 0;

	private:
		bool Rotate(uint64_t nowUs)
		{
			this->Close();

			time_t secs = (time_t)(nowUs / 1000000ull);
			tm t {};
			localtime_r(&secs, &t);
			char name[64];
			strftime(name, sizeof(name), "%Y%m%d-%H%M%S", &t);
			const std::string stem = m_o.out + "/trace-" + std::to_string(port) + "-" + name;
			m_path = stem + ".bin";
			// Dos rotaciones en el mismo segundo: "_1", "_2"... ordenan después del
			// archivo sin sufijo, así que el orden alfabético sigue siendo el real.
			for (int n = 1; access(m_path.c_str(), F_OK) == 0; n++) m_path = stem + "_" + std::to_string(n) + ".bin";

			m_file = fopen(m_path.c_str(), "wb");
			if (m_file == nullptr)
			{
				fprintf(stderr, "mu-trace: no se pudo crear %s: %s\n", m_path.c_str(), strerror(errno));
				return false;
			}
			setvbuf(m_file, nullptr, _IOFBF, 1 << 20);

			FileHeader h {};
			memcpy(h.magic, "MTF1", 4);
			h.version = kVersion;
			h.serverPort = port;
			h.startTimeUs = nowUs;
			fwrite(&h, sizeof(h), 1, m_file);

			m_bytes = sizeof(h);
			m_openedUs = nowUs;
			this->Prune();
			return true;
		}

		// Borra los más viejos hasta quedar debajo de --keep-mb; el actual nunca.
		void Prune()
		{
			std::vector<std::string> files = TraceFiles(m_o.out);
			uint64_t total = 0;
			for (const std::string& f : files) total += FileSize(f);

			const uint64_t limit = m_o.keepMB * 1024ull * 1024ull;
			for (const std::string& f : files)
			{
				if (total <= limit || f == m_path) break;
				const uint64_t size = FileSize(f);
				if (unlink(f.c_str()) == 0) total -= size;
			}
		}

		const Options& m_o;
		FILE* m_file = nullptr;
		std::string m_path;
		uint64_t m_bytes = 0;
		uint64_t m_openedUs = 0;
	};

	int CmdRecord(const Options& o)
	{
		if (o.out.empty())
		{
			fprintf(stderr, "mu-trace: record necesita --out DIR\n");
			return 2;
		}
		if (!IsDirectory(o.out) && mkdir(o.out.c_str(), 0750) != 0)
		{
			fprintf(stderr, "mu-trace: no se pudo crear %s: %s\n", o.out.c_str(), strerror(errno));
			return 1;
		}

		char real[PATH_MAX];
		const std::string dir = (realpath(o.out.c_str(), real) != nullptr) ? real : o.out;

		std::string name;
		if (!PickRing(o.ring, name)) return 1;
		Options copy = o;
		copy.out = dir;
		copy.ring = name.c_str();

		Recorder recorder(copy);
		recorder.port = (uint32_t)atoi(name.c_str() + strlen(kShmPrefix));
		bool ok = true;
		int idleTicks = 0;

		fprintf(stderr, "mu-trace: grabando en %s (archivo nuevo cada %llu MB o %llu min, máximo %llu MB)\n", dir.c_str(),
			(unsigned long long)copy.rotateMB, (unsigned long long)copy.rotateMin, (unsigned long long)copy.keepMB);

		int rc = Consume(copy, ConsumerRecord, dir,
			[&](const RecordHeader* rec) { if (ok && !recorder.Write(rec)) { ok = false; gStop = 1; } },
			[&]() { if (++idleTicks % 50 == 0) recorder.Flush(); });

		recorder.Close();
		return ok ? rc : 1;
	}

	// Lee un archivo grabado desde `offset`. Devuelve el offset hasta donde
	// llegó; un registro a medio escribir queda para la próxima pasada.
	uint64_t ReadFile(const std::string& path, uint64_t offset, const Options& o, bool& bad)
	{
		FILE* f = fopen(path.c_str(), "rb");
		if (f == nullptr)
		{
			fprintf(stderr, "mu-trace: no se pudo abrir %s: %s\n", path.c_str(), strerror(errno));
			bad = true;
			return offset;
		}

		if (offset == 0)
		{
			FileHeader h {};
			if (fread(&h, sizeof(h), 1, f) != 1)
			{
				fclose(f);
				return 0;   // todavía vacío
			}
			if (memcmp(h.magic, "MTF1", 4) != 0 || h.version != kVersion)
			{
				fprintf(stderr, "mu-trace: %s no es una traza de esta versión\n", path.c_str());
				fclose(f);
				bad = true;
				return 0;
			}
			offset = sizeof(h);
		}

		fseek(f, (long)offset, SEEK_SET);
		std::vector<uint8_t> buf(sizeof(RecordHeader) + kMaxPayload + 8);

		while (!gStop)
		{
			RecordHeader* rec = reinterpret_cast<RecordHeader*>(buf.data());
			if (fread(rec, sizeof(RecordHeader), 1, f) != 1) break;
			if (rec->size < sizeof(RecordHeader) || rec->size > buf.size() || (rec->size & 7) != 0)
			{
				fprintf(stderr, "mu-trace: %s está dañado en el byte %llu\n", path.c_str(), (unsigned long long)offset);
				bad = true;
				break;
			}
			const size_t rest = rec->size - sizeof(RecordHeader);
			if (rest > 0 && fread(buf.data() + sizeof(RecordHeader), rest, 1, f) != 1) break;

			offset += rec->size;
			if (Passes(rec, o)) Print(rec, o.maxBytes);
		}

		fclose(f);
		return offset;
	}

	int CmdRead(const Options& o)
	{
		if (o.paths.empty())
		{
			fprintf(stderr, "mu-trace: read necesita un archivo o una carpeta\n");
			return 2;
		}

		std::vector<std::string> files;
		for (const std::string& p : o.paths)
		{
			if (IsDirectory(p))
			{
				std::vector<std::string> in = TraceFiles(p);
				files.insert(files.end(), in.begin(), in.end());
			}
			else
			{
				files.push_back(p);
			}
		}

		bool bad = false;
		if (!o.follow)
		{
			for (const std::string& f : files)
			{
				ReadFile(f, 0, o, bad);
				if (gStop) break;
			}
			return bad ? 1 : 0;
		}

		// --follow: sólo la última carpeta (o el último archivo) y desde el final.
		const std::string target = o.paths.back();
		const bool isDir = IsDirectory(target);
		InstallSignals();
		setvbuf(stdout, nullptr, _IOFBF, 1 << 16);

		// El grabador escribe en bloques, así que el final del archivo puede caer
		// en medio de un registro. Se lee desde el principio y se muestra sólo lo
		// que pasa de acá en adelante.
		Options live = o;
		timespec ts {};
		clock_gettime(CLOCK_REALTIME, &ts);
		live.fromUs = std::max<uint64_t>(o.fromUs, (uint64_t)ts.tv_sec * 1000000ull);

		std::vector<std::string> initial = isDir ? TraceFiles(target) : std::vector<std::string>{ target };
		std::string current = initial.empty() ? std::string() : initial.back();
		uint64_t offset = 0;

		fprintf(stderr, "mu-trace: siguiendo %s. Ctrl+C para salir.\n", isDir ? target.c_str() : current.c_str());

		while (!gStop)
		{
			if (!current.empty())
			{
				offset = ReadFile(current, offset, live, bad);
				fflush(stdout);
				if (bad) return 1;
			}

			if (isDir)
			{
				std::vector<std::string> now = TraceFiles(target);
				if (!now.empty() && now.back() != current)
				{
					// Terminar el archivo anterior antes de pasar al nuevo.
					if (!current.empty()) offset = ReadFile(current, offset, live, bad);
					current = now.back();
					offset = 0;
					continue;
				}
			}

			usleep(200000);
		}
		return 0;
	}
}

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		Usage();
		return 2;
	}

	const std::string cmd = argv[1];
	Options o;

	if (cmd == "status")
	{
		if (!ParseOptions(argc - 2, argv + 2, o, false)) return 2;
		return CmdStatus(o);
	}
	if (cmd == "tail")
	{
		if (!ParseOptions(argc - 2, argv + 2, o, false)) return 2;
		return CmdTail(o);
	}
	if (cmd == "record")
	{
		if (!ParseOptions(argc - 2, argv + 2, o, false)) return 2;
		return CmdRecord(o);
	}
	if (cmd == "read")
	{
		if (!ParseOptions(argc - 2, argv + 2, o, true)) return 2;
		return CmdRead(o);
	}

	Usage();
	return 2;
}
